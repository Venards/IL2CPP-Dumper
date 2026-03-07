#ifndef DUMPER_H
#define DUMPER_H

#include <vector>
#include <string>
#include "il2cpp_image.hxx"
#include "il2cpp_class.hxx"

// Output directory configuration — change these to redirect dump output
static constexpr const char * DUMP_DIR_NORMAL = "C:\\IL2CPP_Dump_Normal\\";
static constexpr const char * DUMP_DIR_AI     = "C:\\IL2CPP_Dump_AI\\";

class Dumper {

public:
    std::vector<Il2CppImage> images;

    Dumper( );

    // Dump all loaded assemblies to both Normal (C#) and AI-friendly formats
    void DumpAllToFiles( );

private:
    // Parsed representation of a single class, built once and written to both outputs
    struct ParsedClass {
        std::string                         namespaceName;
        std::string                         name;
        std::string                         type;          // "class", "struct", "interface", "enum"
        uint32_t                            token;
        std::string                         parentName;
        std::vector<std::string>            interfaceNames;
        std::vector<Il2CppClass::FieldInfo> fields;
        std::vector<Il2CppClass::MethodInfo> methods;
        std::vector<std::string>            nestedTypeNames;
        bool                                isGeneric;     // name contains '<'
    };

    // Parse all classes from an image into ParsedClass structs (done once per assembly)
    std::vector<ParsedClass> ParseImage( const Il2CppImage & img ) const;

    // Write the Normal (C#-style) output file
    void WriteNormal( const std::string & asmName, const std::vector<ParsedClass> & classes ) const;

    // Write the AI-friendly structured output file
    void WriteAI( const std::string & asmName, const std::vector<ParsedClass> & classes ) const;

};

#endif // DUMPER_H
