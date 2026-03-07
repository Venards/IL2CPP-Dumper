#ifndef RVA_H
#define RVA_H

#include <cstdint>
#include <string>
#include <optional>

//  rva — Relative Virtual Address utilities
//
//  Converts between absolute runtime addresses and:
//    - Module-relative offsets  (addr - module_base)
//    - File offsets             (via section table lookup)
//
//  All functions default to GameAssembly.dll. Pass a different name if needed.

namespace rva {

    //  ToRVA
    //  Converts an absolute address to a module-relative offset.
    //  Returns 0 if the address is outside the module's mapped range.
    uintptr_t ToRVA( uintptr_t absAddr, const char * moduleName = "GameAssembly.dll" );

    //  ToAbsolute
    //  Converts a module-relative offset to an absolute runtime address.
    uintptr_t ToAbsolute( uintptr_t rvaOffset, const char * moduleName = "GameAssembly.dll" );

    //  ToFileOffset
    //  Converts an absolute address to the corresponding raw file offset
    //  using the module's section table.
    //  Returns std::nullopt if the address isn't in any mapped section.
    std::optional<uintptr_t> ToFileOffset( uintptr_t absAddr, const char * moduleName = "GameAssembly.dll" );

    //  FormatRVA
    //  Returns a formatted string like "GameAssembly.dll+0x1A2B3C".
    //  Useful for logging and output files.
    std::string FormatRVA( uintptr_t absAddr, const char * moduleName = "GameAssembly.dll" );

    //  GetModuleBase (cached)
    //  Returns the load base of the given module, caching the result.
    //  Returns 0 on failure.
    uintptr_t GetModuleBase( const char * moduleName = "GameAssembly.dll" );

} // namespace rva

#endif // RVA_H
