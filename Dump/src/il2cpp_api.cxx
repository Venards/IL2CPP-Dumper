#include "../include/il2cpp_api.hxx"
#include "../include/utils.hxx"
#include "../include/scanner.hxx"
#include "../include/rva.hxx"
#include <windows.h>
#include <iomanip>
#include <sstream>
#include <vector>

namespace api {

    bool initialized = false;

    // ---- Function pointer instances ----
    get_domain_t                    get_domain               = nullptr;
    get_assemblies_t                get_assemblies           = nullptr;
    assembly_get_image_t            assembly_get_image       = nullptr;
    image_get_name_t                image_get_name           = nullptr;
    image_get_class_count_t         image_get_class_count    = nullptr;
    image_get_class_t               image_get_class          = nullptr;
    class_get_name_t                class_get_name           = nullptr;
    class_get_namespace_t           class_get_namespace      = nullptr;
    class_get_flags_t               class_get_flags          = nullptr;
    class_get_parent_t              class_get_parent         = nullptr;
    class_is_valuetype_t            class_is_valuetype       = nullptr;
    class_is_interface_t            class_is_interface       = nullptr;
    class_is_enum_t                 class_is_enum            = nullptr;
    class_get_interfaces_t          class_get_interfaces     = nullptr;
    class_get_nested_types_t        class_get_nested_types   = nullptr;
    class_num_fields_t              class_num_fields         = nullptr;
    class_get_fields_t              class_get_fields         = nullptr;
    field_get_name_t                field_get_name           = nullptr;
    field_get_type_t                field_get_type           = nullptr;
    field_get_flags_t               field_get_flags          = nullptr;
    field_get_offset_t              field_get_offset         = nullptr;
    class_get_methods_t             class_get_methods        = nullptr;
    method_get_name_t               method_get_name          = nullptr;
    method_get_flags_t              method_get_flags         = nullptr;
    method_get_param_count_t        method_get_param_count   = nullptr;
    method_get_param_t              method_get_param         = nullptr;
    method_get_param_name_t         method_get_param_name    = nullptr;
    method_get_return_type_t        method_get_return_type   = nullptr;
    method_get_function_pointer_t   method_get_function_pointer = nullptr;
    type_get_name_t                 type_get_name            = nullptr;
    class_get_type_token_t          class_get_type_token     = nullptr;


    const char * ResolveName( ResolveMethod m ) {
        switch ( m ) {
            case ResolveMethod::Export:  return "Export";
            case ResolveMethod::Pattern: return "Pattern";
            default:                     return "None";
        }
    }


    //  Internal resolve tracking for the report
    struct FuncEntry {
        const char *  exportName;
        void **       fnPtr;
        const char *  pattern;
        ResolveMethod how;
    };

    static std::vector<FuncEntry> s_registry;



    //  Resolve — tries Export then Pattern scan
    static void Resolve(
        HMODULE      hMod,
        const char * exportName,
        void **      fnPtr,
        const char * pattern,
        bool         required
    ) {
        FuncEntry entry{ exportName, fnPtr, pattern, ResolveMethod::None };

        // Export table
        void * addr = reinterpret_cast<void *>( GetProcAddress( hMod, exportName ) );
        if ( addr ) {
            *fnPtr    = addr;
            entry.how = ResolveMethod::Export;
            s_registry.push_back( entry );
            return;
        }

        // Pattern scan
        if ( pattern ) {
            auto match = scanner::FindPattern( "GameAssembly.dll", pattern );
            if ( match ) {
                *fnPtr    = reinterpret_cast<void *>( *match );
                entry.how = ResolveMethod::Pattern;
                s_registry.push_back( entry );
                return;
            }
        }

        // Not resolved
        s_registry.push_back( entry );

        if ( required ) {
            Log( std::string( "[ERROR] Required function not resolved: " ) + exportName );
        } else {
            Log( std::string( "[WARN]  Optional function not resolved: " ) + exportName );
        }
    }


    //  init
    void init( ) {
        if ( initialized ) return;

        // Poll for GameAssembly.dll — no arbitrary Sleep()
        HMODULE hMod = nullptr;
        for ( int i = 0; i < 200 && !hMod; ++i ) {
            hMod = GetModuleHandleA( "GameAssembly.dll" );
            if ( !hMod ) Sleep( 50 );
        }

        if ( !hMod ) {
            Log( "[ERROR] GameAssembly.dll not found after 10s" );
            return;
        }

        {
            std::ostringstream ss;
            ss << "[OK] GameAssembly.dll @ 0x"
               << std::hex << std::uppercase << reinterpret_cast<uintptr_t>( hMod );
            Log( ss.str( ) );
        }

        s_registry.clear( );
        s_registry.reserve( 32 );


        Resolve( hMod, "il2cpp_domain_get",
            reinterpret_cast<void**>( &get_domain ),
            "48 83 EC 28 48 8B 05 ? ? ? ? 48 85 C0",
            true );

        Resolve( hMod, "il2cpp_domain_get_assemblies",
            reinterpret_cast<void**>( &get_assemblies ),
            "48 89 5C 24 ? 57 48 83 EC 20 48 8B D9 48 8B FA",
            true );

        Resolve( hMod, "il2cpp_assembly_get_image",
            reinterpret_cast<void**>( &assembly_get_image ),
            "48 8B 41 ? C3",
            true );

        Resolve( hMod, "il2cpp_image_get_name",
            reinterpret_cast<void**>( &image_get_name ),
            nullptr, true );

        Resolve( hMod, "il2cpp_image_get_class_count",
            reinterpret_cast<void**>( &image_get_class_count ),
            nullptr, true );

        Resolve( hMod, "il2cpp_image_get_class",
            reinterpret_cast<void**>( &image_get_class ),
            nullptr, true );

        // ---- Class reflection ----
        Resolve( hMod, "il2cpp_class_get_name",         reinterpret_cast<void**>( &class_get_name ),        nullptr, false );
        Resolve( hMod, "il2cpp_class_get_namespace",    reinterpret_cast<void**>( &class_get_namespace ),   nullptr, false );
        Resolve( hMod, "il2cpp_class_get_flags",        reinterpret_cast<void**>( &class_get_flags ),       nullptr, false );
        Resolve( hMod, "il2cpp_class_get_parent",       reinterpret_cast<void**>( &class_get_parent ),      nullptr, false );
        Resolve( hMod, "il2cpp_class_is_valuetype",     reinterpret_cast<void**>( &class_is_valuetype ),    nullptr, false );
        Resolve( hMod, "il2cpp_class_is_interface",     reinterpret_cast<void**>( &class_is_interface ),    nullptr, false );
        Resolve( hMod, "il2cpp_class_is_enum",          reinterpret_cast<void**>( &class_is_enum ),         nullptr, false );
        Resolve( hMod, "il2cpp_class_get_interfaces",   reinterpret_cast<void**>( &class_get_interfaces ),  nullptr, false );
        Resolve( hMod, "il2cpp_class_get_nested_types", reinterpret_cast<void**>( &class_get_nested_types ),nullptr, false );

        // ---- Fields ----
        Resolve( hMod, "il2cpp_class_num_fields",   reinterpret_cast<void**>( &class_num_fields ),  nullptr, false );
        Resolve( hMod, "il2cpp_class_get_fields",   reinterpret_cast<void**>( &class_get_fields ),  nullptr, false );
        Resolve( hMod, "il2cpp_field_get_name",     reinterpret_cast<void**>( &field_get_name ),    nullptr, false );
        Resolve( hMod, "il2cpp_field_get_type",     reinterpret_cast<void**>( &field_get_type ),    nullptr, false );
        Resolve( hMod, "il2cpp_field_get_flags",    reinterpret_cast<void**>( &field_get_flags ),   nullptr, false );
        Resolve( hMod, "il2cpp_field_get_offset",   reinterpret_cast<void**>( &field_get_offset ),  nullptr, false );

        // ---- Methods ----
        Resolve( hMod, "il2cpp_class_get_methods",           reinterpret_cast<void**>( &class_get_methods ),          nullptr, false );
        Resolve( hMod, "il2cpp_method_get_name",             reinterpret_cast<void**>( &method_get_name ),            nullptr, false );
        Resolve( hMod, "il2cpp_method_get_flags",            reinterpret_cast<void**>( &method_get_flags ),           nullptr, false );
        Resolve( hMod, "il2cpp_method_get_param_count",      reinterpret_cast<void**>( &method_get_param_count ),     nullptr, false );
        Resolve( hMod, "il2cpp_method_get_param",            reinterpret_cast<void**>( &method_get_param ),           nullptr, false );
        Resolve( hMod, "il2cpp_method_get_param_name",       reinterpret_cast<void**>( &method_get_param_name ),      nullptr, false );
        Resolve( hMod, "il2cpp_method_get_return_type",      reinterpret_cast<void**>( &method_get_return_type ),     nullptr, false );
        Resolve( hMod, "il2cpp_method_get_function_pointer", reinterpret_cast<void**>( &method_get_function_pointer ),nullptr, false );

        // ---- Type / token ----
        Resolve( hMod, "il2cpp_type_get_name",        reinterpret_cast<void**>( &type_get_name ),        nullptr, false );
        Resolve( hMod, "il2cpp_class_get_type_token",  reinterpret_cast<void**>( &class_get_type_token ), nullptr, false );

        // Validate required set
        if ( !get_domain || !get_assemblies || !assembly_get_image ||
             !image_get_name || !image_get_class_count || !image_get_class ) {
            Log( "[ERROR] One or more required IL2CPP functions could not be resolved" );
            return;
        }

        initialized = true;
        Log( "[OK] IL2CPP API initialized" );
        PrintResolveReport( );
    }


    //  PrintResolveReport
    void PrintResolveReport( ) {
        int resolved = 0, failed = 0;
        for ( const auto & e : s_registry ) {
            if ( e.how != ResolveMethod::None ) ++resolved; else ++failed;
        }

        Log( "" );
        Log( "  Resolve Report: " + std::to_string( resolved ) +
             " resolved, " + std::to_string( failed ) + " missing" );
        Log( "  +---------------------------------+----------+" );
        Log( "  | Function                        | Method   |" );
        Log( "  +---------------------------------+----------+" );

        for ( const auto & e : s_registry ) {
            std::ostringstream row;
            row << "  | "
                << std::left << std::setw( 31 ) << std::string( e.exportName ).substr( 7 )  // strip "il2cpp_"
                << " | "
                << std::left << std::setw( 8 )  << ResolveName( e.how )
                << " |";
            Log( row.str( ) );
        }

        Log( "  +---------------------------------+----------+" );
        Log( "" );
    }

} // namespace api
