#ifndef SCANNER_H
#define SCANNER_H

#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>
#include <optional>


//  scanner — byte-pattern scanning for GameAssembly.dll
//
//  Patterns use IDA-style notation: "48 8B 05 ? ? ? ? C3"
//  '?' or '??' are wildcards that match any byte.
//
//  Usage:
//    auto result = scanner::FindPattern( "GameAssembly.dll", "48 89 5C 24 ? 57" );
//    if ( result ) { uintptr_t addr = *result; ... }

namespace scanner {

    //  ParsedPattern
    //  Internal representation of a compiled pattern.
    //  bytes[i] is the expected byte; mask[i] = false means wildcard.
    struct ParsedPattern {
        std::vector<uint8_t> bytes;
        std::vector<bool>    mask;   // true = match this byte, false = wildcard

        size_t size( ) const { return bytes.size( ); }
        bool   empty( ) const { return bytes.empty( ); }
    };

    //  Compile
    //  Parses an IDA-style pattern string into a ParsedPattern.
    //  Returns empty ParsedPattern on malformed input.
    ParsedPattern Compile( const std::string & pattern );

    //  FindPatternInRange
    //  Scans [start, start+size) for the compiled pattern.
    //  Returns address of the first match, or std::nullopt.
    std::optional<uintptr_t> FindPatternInRange(
        uintptr_t          start,
        size_t             size,
        const ParsedPattern & pat
    );

    //  FindPattern (module-level)
    //  Convenience: scans the entire .text section of `moduleName`.
    //  Returns absolute address of first match, or std::nullopt.
    std::optional<uintptr_t> FindPattern(
        const char *       moduleName,
        const std::string & pattern
    );

    //  FindAllPatterns
    //  Like FindPattern but returns every match in the module's .text section.
    std::vector<uintptr_t> FindAllPatterns(
        const char *       moduleName,
        const std::string & pattern
    );

    //  ResolveRipRelative
    //  Resolves a RIP-relative instruction operand to its target address.
    //
    //  Given the address of the first byte of a RIP-relative instruction and
    //  the byte offset within that instruction where the 32-bit displacement
    //  lives, returns the resolved absolute address.
    //
    //  Example: "48 8B 05 [XX XX XX XX]"  instrAddr = addr, dispOffset = 3
    uintptr_t ResolveRipRelative( uintptr_t instrAddr, int dispOffset, int instrLen );

} // namespace scanner

#endif // SCANNER_H
// Please give me money
