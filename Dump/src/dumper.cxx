#include "../include/dumper.hxx"
#include "../include/il2cpp_api.hxx"
#include "../include/utils.hxx"
#include <fstream>
#include <unordered_map>
#include <map>
#include <iomanip>
#include <sstream>
#include <algorithm>


// ---- Constructor --------------------------------------------------------

Dumper::Dumper( ) {
    api::init( );

    if ( !api::initialized ) {
        Log( "[ERROR] IL2CPP API failed to initialize" );
        return;
    }

    Il2CppDomainOpaque * domain = api::get_domain( );
    if ( !domain ) {
        Log( "[ERROR] il2cpp_domain_get returned null" );
        return;
    }

    size_t count = 0;
    Il2CppAssemblyOpaque ** assemblies = api::get_assemblies( domain, &count );
    if ( !assemblies || count == 0 ) {
        Log( "[ERROR] No assemblies found in domain" );
        return;
    }

    images.reserve( count );

    for ( size_t i = 0; i < count; ++i ) {
        Il2CppAssemblyOpaque * ass = assemblies[ i ];
        if ( !ass ) continue;

        Il2CppImageOpaque * img = api::assembly_get_image( ass );
        if ( !img ) continue;

        const char * name = api::image_get_name( img );
        if ( !name || !*name ) continue;

        images.emplace_back( img );
    }
}


// ---- Parsing (done once per assembly) -----------------------------------

std::vector<Dumper::ParsedClass> Dumper::ParseImage( const Il2CppImage & img ) const {
    std::vector<ParsedClass> result;
    size_t classCount = img.GetClassCount( );
    result.reserve( classCount );

    for ( size_t i = 0; i < classCount; ++i ) {
        Il2CppClass cls = img.GetClassByIndex( i );
        if ( !cls.klass ) continue;

        ParsedClass pc;
        pc.name          = cls.GetName( );
        pc.namespaceName = cls.GetNamespace( );
        pc.token         = cls.GetTypeToken( );
        pc.isGeneric     = ( pc.name.find( '<' ) != std::string::npos );

        bool isInterface = cls.IsInterface( );
        bool isEnum      = cls.IsEnum( );
        bool isStruct    = cls.IsValueType( ) && !isInterface && !isEnum;

        if ( isInterface )     pc.type = "interface";
        else if ( isEnum )     pc.type = "enum";
        else if ( isStruct )   pc.type = "struct";
        else                   pc.type = "class";

        // Parent class (skip trivial base types)
        Il2CppClass parent = cls.GetParent( );
        if ( parent.klass ) {
            std::string pn = parent.GetName( );
            if ( pn != "Object" && pn != "ValueType" && pn != "Enum" ) {
                pc.parentName = pn;
            }
        }

        // Interfaces
        for ( const Il2CppClass & iface : cls.GetInterfaces( ) ) {
            if ( iface.klass ) {
                pc.interfaceNames.emplace_back( iface.GetName( ) );
            }
        }

        // Nested types (names only — they are also emitted as top-level entries)
        for ( const Il2CppClass & nested : cls.GetNestedTypes( ) ) {
            if ( nested.klass ) {
                pc.nestedTypeNames.emplace_back( nested.GetName( ) );
            }
        }

        pc.fields  = cls.GetFields( );
        pc.methods = cls.GetMethods( );

        result.push_back( std::move( pc ) );
    }

    return result;
}


// ---- Normal (C#) writer -------------------------------------------------

void Dumper::WriteNormal( const std::string & asmName, const std::vector<ParsedClass> & classes ) const {
    std::string filename = std::string( DUMP_DIR_NORMAL ) + asmName + ".cs";
    std::ofstream out( filename );
    if ( !out.is_open( ) ) {
        Log( "[ERROR] Cannot open for writing: " + filename );
        return;
    }

    out << "// ========================================================\n";
    out << "// Dumped by @wg5h\n";
    out << "// Assembly: " << asmName << "\n";
    out << "// Classes:  " << classes.size( ) << "\n";
    out << "// Date:     " << GetCurrentTimestamp( ) << "\n";
    out << "// ========================================================\n\n";
    out << "using System;\n";
    out << "using System.Collections.Generic;\n\n";

    // Group by namespace using an ordered map so output is deterministic
    std::map<std::string, std::vector<const ParsedClass *>> ns_map;
    for ( const auto & pc : classes ) {
        ns_map[ pc.namespaceName ].push_back( &pc );
    }

    for ( const auto & [ns, classList] : ns_map ) {
        const bool hasNs = !ns.empty( );
        const std::string indent = hasNs ? "    " : "";

        if ( hasNs ) {
            out << "namespace " << ns << "\n{\n\n";
        }

        for ( const ParsedClass * pc : classList ) {
            // Comment-out compiler-generated / generic classes rather than silently dropping them
            const bool commentOut = pc->isGeneric;
            const std::string prefix = commentOut ? ( indent + "// [GENERIC] " ) : indent;

            out << prefix << "// TypeToken: 0x"
                << std::hex << std::uppercase << pc->token << std::dec << "\n";

            out << indent << "public " << pc->type << " " << pc->name;

            // Inheritance
            bool wroteBase = false;
            if ( !pc->parentName.empty( ) ) {
                out << " : " << pc->parentName;
                wroteBase = true;
            }
            if ( !pc->interfaceNames.empty( ) ) {
                out << ( wroteBase ? ", " : " : " );
                for ( size_t j = 0; j < pc->interfaceNames.size( ); ++j ) {
                    if ( j > 0 ) out << ", ";
                    out << pc->interfaceNames[ j ];
                }
            }
            out << "\n" << indent << "{\n";

            // Fields
            if ( !pc->fields.empty( ) ) {
                out << indent << "    // Fields\n";
                for ( const auto & f : pc->fields ) {
                    std::string acc  = GetAccessModifier( f.flags );
                    std::string mods;
                    if ( f.flags & 0x0010 ) mods += "static ";
                    if ( f.flags & 0x0020 ) mods += "readonly ";

                    out << indent << "    " << acc << " " << mods
                        << f.typeName << " " << f.name << ";";
                    out << " // 0x" << std::hex << f.offset << std::dec << "\n";
                }
                out << "\n";
            }

            // Methods
            if ( !pc->methods.empty( ) ) {
                out << indent << "    // Methods\n";
                for ( const auto & m : pc->methods ) {
                    std::string acc  = GetAccessModifier( m.flags );
                    std::string mods;
                    if ( m.flags & 0x0010 ) mods += "static ";
                    if ( m.flags & 0x0400 ) mods += "abstract ";
                    else if ( m.flags & 0x0040 ) mods += "virtual ";

                    out << indent << "    " << acc << " " << mods
                        << m.returnType << " " << m.name << "(";

                    for ( size_t j = 0; j < m.params.size( ); ++j ) {
                        if ( j > 0 ) out << ", ";
                        out << m.params[ j ].typeName << " " << m.params[ j ].name;
                    }

                    out << ") { }";

                    // Emit RVA if available
                    if ( m.rva != 0 ) {
                        out << " // RVA: 0x" << std::hex << std::uppercase << m.rva << std::dec;
                    }
                    out << "\n";
                }
                out << "\n";
            }

            out << indent << "}\n\n";
        }

        if ( hasNs ) {
            out << "}\n\n";
        }
    }

    Log( "[C#]  Saved -> " + filename );
}


// ---- AI writer ----------------------------------------------------------

void Dumper::WriteAI( const std::string & asmName, const std::vector<ParsedClass> & classes ) const {
    std::string filename = std::string( DUMP_DIR_AI ) + asmName + ".txt";
    std::ofstream out( filename );
    if ( !out.is_open( ) ) {
        Log( "[ERROR] Cannot open for writing: " + filename );
        return;
    }

    out << "# ========================================================\n";
    out << "# AI-FRIENDLY STRUCTURED DUMP\n";
    out << "# Optimized for LLM parsing / code generation\n";
    out << "# Assembly: " << asmName << "\n";
    out << "# Classes:  " << classes.size( ) << "\n";
    out << "# Date:     " << GetCurrentTimestamp( ) << "\n";
    out << "# ========================================================\n\n";

    for ( const auto & pc : classes ) {
        const std::string fullName = pc.namespaceName.empty( )
            ? pc.name
            : pc.namespaceName + "." + pc.name;

        out << "CLASS:   " << fullName << "\n";
        out << "TYPE:    " << pc.type << "\n";
        out << "TOKEN:   0x" << std::hex << std::uppercase << pc.token << std::dec << "\n";

        if ( pc.isGeneric ) {
            out << "GENERIC: true\n";
        }

        if ( !pc.parentName.empty( ) ) {
            out << "EXTENDS: " << pc.parentName << "\n";
        }

        if ( !pc.interfaceNames.empty( ) ) {
            out << "IMPLEMENTS:";
            for ( const auto & in : pc.interfaceNames ) out << " " << in;
            out << "\n";
        }

        if ( !pc.nestedTypeNames.empty( ) ) {
            out << "NESTED:";
            for ( const auto & nt : pc.nestedTypeNames ) out << " " << nt;
            out << "\n";
        }

        // Fields
        if ( !pc.fields.empty( ) ) {
        out << "FIELDS:\n";
        for ( const auto & f : pc.fields ) {
            std::string acc  = GetAccessModifier( f.flags );
            std::string mods;
            if ( f.flags & 0x0010 ) mods += "static ";
            if ( f.flags & 0x0020 ) mods += "readonly ";

            out << "  " << std::left
                << std::setw( 20 ) << acc + mods
                << std::setw( 36 ) << f.typeName
                << f.name
                << "  // offset: 0x" << std::hex << f.offset << std::dec << "\n";
        }
        }

        // Methods
        if ( !pc.methods.empty( ) ) {
        out << "METHODS:\n";
        for ( const auto & m : pc.methods ) {
            out << "  " << m.returnType << " " << m.name << "(";
            for ( size_t j = 0; j < m.params.size( ); ++j ) {
                if ( j > 0 ) out << ", ";
                out << m.params[ j ].typeName << " " << m.params[ j ].name;
            }
            out << ")";
            if ( m.rva != 0 ) {
                out << "  // RVA: 0x" << std::hex << std::uppercase << m.rva << std::dec;
            }
            out << "\n";
        }
        }

        out << "END_CLASS\n\n";
    }

    Log( "[AI]  Saved -> " + filename );
}


// ---- Public entry point -------------------------------------------------

void Dumper::DumpAllToFiles( ) {
    EnsureDirectory( DUMP_DIR_NORMAL );
    EnsureDirectory( DUMP_DIR_AI );

    for ( const auto & img : images ) {
        std::string name = img.GetName( );
        Log( "Dumping: " + name + " ..." );

        // Parse metadata once, write to both outputs
        std::vector<ParsedClass> classes = ParseImage( img );

        WriteNormal( name, classes );
        WriteAI( name, classes );
    }

    Log( "\n[DONE]" );
    Log( "  C# output:       " + std::string( DUMP_DIR_NORMAL ) );
    Log( "  AI output:       " + std::string( DUMP_DIR_AI ) );
}
