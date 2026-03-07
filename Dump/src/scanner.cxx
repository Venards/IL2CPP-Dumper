#include "../include/scanner.hxx"
#include "../include/mem.hxx"
#include "../include/utils.hxx"
#include <windows.h>
#include <psapi.h>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace scanner {

    //  Compile
    ParsedPattern Compile( const std::string & pattern ) {
        ParsedPattern result;
        std::istringstream ss( pattern );
        std::string token;

        while ( ss >> token ) {
            // Wildcard: '?' or '??'
            if ( token == "?" || token == "??" ) {
                result.bytes.push_back( 0x00 );
                result.mask.push_back( false );
                continue;
            }

            // Must be exactly 1 or 2 hex digits
            if ( token.size( ) > 2 ) {
                Log( "[SCANNER] Malformed pattern token: " + token );
                return {};
            }

            // Validate hex characters
            bool valid = true;
            for ( char c : token ) {
                if ( !std::isxdigit( static_cast<unsigned char>( c ) ) ) {
                    valid = false;
                    break;
                }
            }
            if ( !valid ) {
                Log( "[SCANNER] Non-hex pattern token: " + token );
                return {};
            }

            uint8_t byte = static_cast<uint8_t>( std::stoul( token, nullptr, 16 ) );
            result.bytes.push_back( byte );
            result.mask.push_back( true );
        }

        return result;
    }


    //  FindPatternInRange
    //  Boyer-Moore-Horspool style skip table for performance on large ranges.
    std::optional<uintptr_t> FindPatternInRange(
        uintptr_t             start,
        size_t                size,
        const ParsedPattern & pat
    ) {
        if ( pat.empty( ) || size < pat.size( ) ) return std::nullopt;
        if ( !mem::IsReadable( start, size ) )     return std::nullopt;

        const size_t patLen = pat.size( );

        // Build bad-character skip table using only non-wildcard bytes
        // Default skip is patLen; last wildcard byte positions reset it to 1
        size_t skipTable[ 256 ];
        for ( size_t i = 0; i < 256; ++i ) skipTable[ i ] = patLen;
        for ( size_t i = 0; i < patLen - 1; ++i ) {
            if ( pat.mask[ i ] ) {
                skipTable[ pat.bytes[ i ] ] = patLen - 1 - i;
            }
        }

        const uint8_t * data  = reinterpret_cast<const uint8_t *>( start );
        const size_t    limit = size - patLen;

        size_t pos = 0;
        while ( pos <= limit ) {
            // Match backwards from last byte
            size_t j = patLen - 1;

            // Walk backwards checking each byte
            while ( true ) {
                if ( pat.mask[ j ] && data[ pos + j ] != pat.bytes[ j ] ) {
                    pos += skipTable[ data[ pos + patLen - 1 ] ];
                    break;
                }
                if ( j == 0 ) {
                    return start + pos;  // Full match
                }
                --j;
            }
        }

        return std::nullopt;
    }


    //  GetTextSection
    //  Locates the .text section of a loaded PE module.
    static bool GetTextSection( const char * moduleName, uintptr_t & outBase, size_t & outSize ) {
        HMODULE hMod = GetModuleHandleA( moduleName );
        if ( !hMod ) return false;

        uintptr_t base = reinterpret_cast<uintptr_t>( hMod );
        auto dosHdr = reinterpret_cast<const IMAGE_DOS_HEADER *>( base );
        if ( dosHdr->e_magic != IMAGE_DOS_SIGNATURE ) return false;

        auto ntHdr = reinterpret_cast<const IMAGE_NT_HEADERS *>( base + dosHdr->e_lfanew );
        if ( ntHdr->Signature != IMAGE_NT_SIGNATURE ) return false;

        const IMAGE_SECTION_HEADER * section = IMAGE_FIRST_SECTION( ntHdr );
        uint16_t numSections = ntHdr->FileHeader.NumberOfSections;

        for ( uint16_t i = 0; i < numSections; ++i ) {
            // .text section name check (null-padded 8 bytes)
            if ( std::strncmp( reinterpret_cast<const char *>( section[ i ].Name ), ".text", 5 ) == 0 ) {
                outBase = base + section[ i ].VirtualAddress;
                outSize = section[ i ].Misc.VirtualSize;
                return true;
            }
        }

        // Fallback: use the full image if no .text section found
        MODULEINFO info{};
        if ( GetModuleInformation( GetCurrentProcess( ), hMod, &info, sizeof( info ) ) ) {
            outBase = base;
            outSize = info.SizeOfImage;
            return true;
        }

        return false;
    }


    //  FindPattern
    std::optional<uintptr_t> FindPattern(
        const char *        moduleName,
        const std::string & pattern
    ) {
        ParsedPattern pat = Compile( pattern );
        if ( pat.empty( ) ) return std::nullopt;

        uintptr_t base = 0;
        size_t    size = 0;
        if ( !GetTextSection( moduleName, base, size ) ) {
            Log( std::string( "[SCANNER] Module not found: " ) + moduleName );
            return std::nullopt;
        }

        return FindPatternInRange( base, size, pat );
    }


    //  FindAllPatterns
    std::vector<uintptr_t> FindAllPatterns(
        const char *        moduleName,
        const std::string & pattern
    ) {
        std::vector<uintptr_t> results;

        ParsedPattern pat = Compile( pattern );
        if ( pat.empty( ) ) return results;

        uintptr_t base = 0;
        size_t    size = 0;
        if ( !GetTextSection( moduleName, base, size ) ) return results;

        uintptr_t searchBase = base;
        size_t    remaining  = size;

        while ( remaining >= pat.size( ) ) {
            auto match = FindPatternInRange( searchBase, remaining, pat );
            if ( !match ) break;

            results.push_back( *match );

            // Advance past this match
            size_t advance = ( *match - searchBase ) + 1;
            if ( advance >= remaining ) break;
            searchBase += advance;
            remaining  -= advance;
        }

        return results;
    }


    // -----------------------------------------------------------------------
    //  ResolveRipRelative
    // -----------------------------------------------------------------------
    uintptr_t ResolveRipRelative( uintptr_t instrAddr, int dispOffset, int instrLen ) {
        auto disp = mem::SafeRead<int32_t>( instrAddr + static_cast<uintptr_t>( dispOffset ) );
        if ( !disp ) return 0;
        // RIP points to the next instruction (instrAddr + instrLen)
        return instrAddr + static_cast<uintptr_t>( instrLen ) + static_cast<uintptr_t>( *disp );
    }

} // namespace scanner
