#include "../include/mem.hxx"
#include <psapi.h>

#pragma comment( lib, "psapi.lib" )

namespace mem {

    //  RawCopy — SEH isolated here. No C++ objects with destructors in scope.
    //  MSVC requires /EHa or that no unwindable objects exist inside __try.
    //  This function has no std::string, no std::vector, no optional — clean.
    bool RawCopy( uintptr_t src, void * dst, size_t size ) {
        __try {
            // memcpy is an intrinsic — safe inside __try
            memcpy( dst, reinterpret_cast<const void *>( src ), size );
        }
        __except ( EXCEPTION_EXECUTE_HANDLER ) {
            return false;
        }
        return true;
    }


    //  SafeReadString
    //  Reads into a plain char buffer (no destructor) inside a helper,
    //  then builds std::string outside any SEH boundary.
    //  I'm so sigma

    // Internal helper — SEH only, no C++ objects in scope
    static bool RawReadStringBuf( uintptr_t addr, char * buf, size_t maxLen ) {
        __try {
            const char * p = reinterpret_cast<const char *>( addr );
            for ( size_t i = 0; i < maxLen; ++i ) {
                char c = p[ i ];
                buf[ i ] = c;
                if ( c == '\0' ) return true;
            }
            buf[ maxLen - 1 ] = '\0';
        }
        __except ( EXCEPTION_EXECUTE_HANDLER ) {
            return false;
        }
        return true;
    }

    std::string SafeReadString( uintptr_t addr, size_t maxLen ) {
        if ( addr == 0 || maxLen == 0 ) return {};

        // Stack buffer for short strings. Heap for longer.
        // Cap at 512 to avoid large stack allocations.
        const size_t cap = ( maxLen < 512 ) ? maxLen + 1 : 513;
        char * buf = new char[ cap ]();

        bool ok = RawReadStringBuf( addr, buf, cap - 1 );

        std::string result;
        if ( ok && buf[ 0 ] != '\0' ) {
            // Validate: reject strings with non-printable bytes (likely bad pointer)
            bool valid = true;
            for ( size_t i = 0; buf[ i ] != '\0' && i < cap; ++i ) {
                unsigned char c = static_cast<unsigned char>( buf[ i ] );
                if ( c < 0x09 || ( c > 0x0D && c < 0x20 ) ) {
                    valid = false;
                    break;
                }
            }
            if ( valid ) result = buf;
        }

        delete[] buf;
        return result;
    }

    std::string SafeReadString( const char * ptr, size_t maxLen ) {
        return SafeReadString( reinterpret_cast<uintptr_t>( ptr ), maxLen );
    }


    //  IsReadable — no SEH needed, VirtualQuery does not fault
    bool IsReadable( uintptr_t addr, size_t size ) {
        if ( addr == 0 ) return false;

        MEMORY_BASIC_INFORMATION mbi{};
        if ( VirtualQuery( reinterpret_cast<LPCVOID>( addr ), &mbi, sizeof( mbi ) ) == 0 )
            return false;

        if ( mbi.State != MEM_COMMIT ) return false;

        constexpr DWORD readableFlags =
            PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ |
            PAGE_EXECUTE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_WRITECOPY;

        if ( !( mbi.Protect & readableFlags ) ) return false;
        if (    mbi.Protect & PAGE_GUARD      ) return false;
        if (    mbi.Protect & PAGE_NOACCESS   ) return false;

        uintptr_t regionEnd =
            reinterpret_cast<uintptr_t>( mbi.BaseAddress ) + mbi.RegionSize;

        return ( addr + size ) <= regionEnd;
    }


    //  ModuleBase / ModuleSize
    uintptr_t ModuleBase( const char * moduleName ) {
        HMODULE hMod = GetModuleHandleA( moduleName );
        return hMod ? reinterpret_cast<uintptr_t>( hMod ) : 0;
    }

    size_t ModuleSize( const char * moduleName ) {
        HMODULE hMod = GetModuleHandleA( moduleName );
        if ( !hMod ) return 0;

        MODULEINFO info{};
        if ( !GetModuleInformation( GetCurrentProcess( ), hMod, &info, sizeof( info ) ) )
            return 0;

        return static_cast<size_t>( info.SizeOfImage );
    }

} // namespace mem
