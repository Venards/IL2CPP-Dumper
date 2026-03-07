#ifndef MEM_H
#define MEM_H

#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <string>
#include <optional>
#include <cstring>

//  mem — safe memory read / write utilities
//
//  SEH (__try/__except) cannot be mixed with C++ object unwinding.
//  All SEH is isolated in plain C-style helpers (no destructors in scope).
//  C++ wrappers then operate on the raw results outside the SEH boundary.
namespace mem {

    //  Internal — raw SEH byte copy, no C++ objects in scope.
    //  Copies `size` bytes from `src` into `dst`.
    //  Returns true on success, false on access violation.
    //  NOT for direct use — call SafeRead<T> instead.
    bool RawCopy( uintptr_t src, void * dst, size_t size );

    //  SafeRead<T>
    //  Reads a plain-old-data value at `addr`.
    //  T must be trivially copyable (int, uintptr_t, pointers, etc.).
    //  Returns std::nullopt on access violation or null address.
    template<typename T>
    std::optional<T> SafeRead( uintptr_t addr ) {
        static_assert( std::is_trivially_copyable<T>::value,
            "SafeRead<T>: T must be trivially copyable" );

        if ( addr == 0 ) return std::nullopt;

        // Read raw bytes outside any C++ object scope via RawCopy (SEH inside)
        T value{};
        if ( !RawCopy( addr, &value, sizeof( T ) ) )
            return std::nullopt;

        return value;
    }

    // Pointer-overload convenience
    template<typename T>
    std::optional<T> SafeRead( const void * ptr ) {
        return SafeRead<T>( reinterpret_cast<uintptr_t>( ptr ) );
    }

    //  SafeReadString
    //  Reads a null-terminated C string with a maximum length guard.
    //  Returns empty string on any fault or non-printable content.
    std::string SafeReadString( uintptr_t addr, size_t maxLen = 512 );
    std::string SafeReadString( const char * ptr, size_t maxLen = 512 );

    //  IsReadable
    //  Checks via VirtualQuery that [addr, addr+size) is committed & readable.
    //  Use for upfront validation. Prefer SafeRead on hot paths.
    bool IsReadable( uintptr_t addr, size_t size = 1 );

    //  ModuleBase / ModuleSize
    uintptr_t ModuleBase( const char * moduleName );
    size_t    ModuleSize( const char * moduleName );

    //  Align — round up to nearest multiple of alignment
    inline uintptr_t Align( uintptr_t value, uintptr_t alignment ) {
        return ( value + alignment - 1 ) & ~( alignment - 1 );
    }

} // namespace mem

#endif // MEM_H
// Please give me money
