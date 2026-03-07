#include "../include/il2cpp_class.hxx"
#include "../include/il2cpp_api.hxx"


const char * Il2CppClass::GetName( ) const {
    return ( api::class_get_name && klass ) ? api::class_get_name( klass ) : "Unknown";
}


const char * Il2CppClass::GetNamespace( ) const {
    return ( api::class_get_namespace && klass ) ? api::class_get_namespace( klass ) : "";
}


bool Il2CppClass::IsValueType( ) const {
    return ( api::class_is_valuetype && klass ) ? api::class_is_valuetype( klass ) : false;
}


bool Il2CppClass::IsInterface( ) const {
    return ( api::class_is_interface && klass ) ? api::class_is_interface( klass ) : false;
}


bool Il2CppClass::IsEnum( ) const {
    return ( api::class_is_enum && klass ) ? api::class_is_enum( klass ) : false;
}


uint32_t Il2CppClass::GetFlags( ) const {
    return ( api::class_get_flags && klass ) ? api::class_get_flags( klass ) : 0;
}


uint32_t Il2CppClass::GetTypeToken( ) const {
    return ( api::class_get_type_token && klass ) ? api::class_get_type_token( klass ) : 0;
}


Il2CppClass Il2CppClass::GetParent( ) const {
    Il2CppClassOpaque * parent = ( api::class_get_parent && klass )
        ? api::class_get_parent( klass )
        : nullptr;
    return Il2CppClass( parent );
}


std::vector<Il2CppClass> Il2CppClass::GetInterfaces( ) const {
    std::vector<Il2CppClass> ifaces;
    if ( !klass || !api::class_get_interfaces ) return ifaces;

    void * iter = nullptr;
    while ( Il2CppClassOpaque * iface = api::class_get_interfaces( klass, &iter ) ) {
        ifaces.emplace_back( iface );
    }
    return ifaces;
}


std::vector<Il2CppClass> Il2CppClass::GetNestedTypes( ) const {
    std::vector<Il2CppClass> nested;
    if ( !klass || !api::class_get_nested_types ) return nested;

    void * iter = nullptr;
    while ( Il2CppClassOpaque * n = api::class_get_nested_types( klass, &iter ) ) {
        nested.emplace_back( n );
    }
    return nested;
}


std::vector<Il2CppClass::FieldInfo> Il2CppClass::GetFields( ) const {
    std::vector<FieldInfo> fields;
    if ( !klass || !api::class_get_fields ) return fields;

    void * iter = nullptr;
    while ( Il2CppFieldOpaque * field = api::class_get_fields( klass, &iter ) ) {
        const char * name = api::field_get_name ? api::field_get_name( field ) : nullptr;
        if ( !name || !*name ) continue;

        int32_t  offset = api::field_get_offset ? api::field_get_offset( field ) : -1;
        uint32_t flags  = api::field_get_flags  ? api::field_get_flags( field )  : 0;

        Il2CppTypeOpaque * ftype  = api::field_get_type ? api::field_get_type( field ) : nullptr;
        const char * typeName     = ( ftype && api::type_get_name ) ? api::type_get_name( ftype ) : "object";

        fields.push_back( { flags, typeName ? typeName : "object", name, offset } );
    }
    return fields;
}


std::vector<Il2CppClass::MethodInfo> Il2CppClass::GetMethods( ) const {
    std::vector<MethodInfo> methods;
    if ( !klass || !api::class_get_methods ) return methods;

    void * iter = nullptr;
    while ( Il2CppMethodOpaque * method = api::class_get_methods( klass, &iter ) ) {
        const char * mname = api::method_get_name ? api::method_get_name( method ) : nullptr;
        if ( !mname || !*mname ) continue;

        // Fix: method_get_flags returns the flags; iflags is the implementation flags
        uint32_t iflags = 0;
        uint32_t flags  = api::method_get_flags ? api::method_get_flags( method, &iflags ) : 0;

        Il2CppTypeOpaque * rtype   = api::method_get_return_type ? api::method_get_return_type( method ) : nullptr;
        const char * rtname        = ( rtype && api::type_get_name ) ? api::type_get_name( rtype ) : "void";

        // Get function pointer (RVA / in-memory address)
        uintptr_t rva = 0;
        if ( api::method_get_function_pointer ) {
            const void * fp = api::method_get_function_pointer( method );
            rva = reinterpret_cast<uintptr_t>( fp );
        }

        uint32_t pcount = api::method_get_param_count ? api::method_get_param_count( method ) : 0;

        std::vector<ParamInfo> params;
        params.reserve( pcount );

        for ( uint32_t i = 0; i < pcount; ++i ) {
            Il2CppTypeOpaque * ptype = api::method_get_param ? api::method_get_param( method, i ) : nullptr;
            const char * ptname      = ( ptype && api::type_get_name ) ? api::type_get_name( ptype ) : "object";

            const char * pname = api::method_get_param_name ? api::method_get_param_name( method, i ) : nullptr;

            // Build fallback name as std::string to avoid dangling pointer bug
            std::string resolvedName = pname ? pname : ( "arg" + std::to_string( i ) );

            params.push_back( { ptname ? ptname : "object", resolvedName } );
        }

        methods.push_back( { flags, rtname ? rtname : "void", mname, rva, std::move( params ) } );
    }
    return methods;
}
