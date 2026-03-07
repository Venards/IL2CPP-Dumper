#include "../include/rva.hxx"
#include "../include/utils.hxx"
#include <windows.h>
#include <psapi.h>
#pragma comment( lib, "psapi.lib" )
#include <unordered_map>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <cstring>

namespace rva {

    // Simple cache: moduleName -> base address
    static std::unordered_map<std::string, uintptr_t> s_baseCache;
    static std::mutex                                  s_cacheMutex;


    uintptr_t GetModuleBase( const char * moduleName ) {
        std::string key( moduleName );
        {
            std::lock_guard<std::mutex> lock( s_cacheMutex );
            auto it = s_baseCache.find( key );
            if ( it != s_baseCache.end( ) ) return it->second;
        }

        HMODULE hMod = GetModuleHandleA( moduleName );
        uintptr_t base = hMod ? reinterpret_cast<uintptr_t>( hMod ) : 0;

        if ( base ) {
            std::lock_guard<std::mutex> lock( s_cacheMutex );
            s_baseCache[ key ] = base;
        }

        return base;
    }


    uintptr_t ToRVA( uintptr_t absAddr, const char * moduleName ) {
        uintptr_t base = GetModuleBase( moduleName );
        if ( base == 0 || absAddr < base ) return 0;

        // Verify the address is within the module's mapped image
        HMODULE hMod = reinterpret_cast<HMODULE>( base );
        MODULEINFO info{};
        if ( GetModuleInformation( GetCurrentProcess( ), hMod, &info, sizeof( info ) ) ) {
            if ( absAddr >= base + info.SizeOfImage ) return 0;
        }

        return absAddr - base;
    }


    uintptr_t ToAbsolute( uintptr_t rvaOffset, const char * moduleName ) {
        uintptr_t base = GetModuleBase( moduleName );
        if ( base == 0 ) return 0;
        return base + rvaOffset;
    }


    std::optional<uintptr_t> ToFileOffset( uintptr_t absAddr, const char * moduleName ) {
        uintptr_t base = GetModuleBase( moduleName );
        if ( base == 0 ) return std::nullopt;

        auto dosHdr = reinterpret_cast<const IMAGE_DOS_HEADER *>( base );
        if ( dosHdr->e_magic != IMAGE_DOS_SIGNATURE ) return std::nullopt;

        auto ntHdr = reinterpret_cast<const IMAGE_NT_HEADERS *>( base + dosHdr->e_lfanew );
        if ( ntHdr->Signature != IMAGE_NT_SIGNATURE ) return std::nullopt;

        uintptr_t rvaOff = absAddr - base;

        const IMAGE_SECTION_HEADER * section = IMAGE_FIRST_SECTION( ntHdr );
        uint16_t numSections = ntHdr->FileHeader.NumberOfSections;

        for ( uint16_t i = 0; i < numSections; ++i ) {
            uintptr_t secVA    = section[ i ].VirtualAddress;
            uintptr_t secSize  = section[ i ].Misc.VirtualSize;
            uintptr_t secRaw   = section[ i ].PointerToRawData;

            if ( rvaOff >= secVA && rvaOff < secVA + secSize ) {
                return secRaw + ( rvaOff - secVA );
            }
        }

        return std::nullopt;
    }


    std::string FormatRVA( uintptr_t absAddr, const char * moduleName ) {
        uintptr_t offset = ToRVA( absAddr, moduleName );

        std::ostringstream ss;
        ss << moduleName << "+0x"
           << std::hex << std::uppercase << offset;
        return ss.str( );
    }

} // namespace rva
