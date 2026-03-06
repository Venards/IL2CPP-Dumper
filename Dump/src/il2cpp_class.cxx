#include "../include/il2cpp_class.hxx"
#include "../include/il2cpp_api.hxx"
#include <windows.h>


const char * Il2CppClass::GetName( ) const {
    return api::class_get_name && klass ? api::class_get_name( klass ) : "Unknown";
}


const char * Il2CppClass::GetNamespace( ) const {
    return api::class_get_namespace && klass ? api::class_get_namespace( klass ) : "";
}


bool Il2CppClass::IsValueType( ) const {
    return api::class_is_valuetype && klass ? api::class_is_valuetype( klass ) : false;
}


bool Il2CppClass::IsInterface( ) const {
    return api::class_is_interface && klass ? api::class_is_interface( klass ) : false;
}


uint32_t Il2CppClass::GetFlags( ) const {
    return api::class_get_flags && klass ? api::class_get_flags( klass ) : 0;
}


uint32_t Il2CppClass::GetTypeToken( ) const {
    return api::class_get_type_token && klass ? api::class_get_type_token( klass ) : 0;
}


Il2CppClass Il2CppClass::GetParent( ) const {
    return Il2CppClass( api::class_get_parent && klass ? api::class_get_parent( klass ) : nullptr );
}


std::vector<Il2CppClass> Il2CppClass::GetInterfaces( ) const {
    std::vector<Il2CppClass> ifaces;

    if ( !klass || !api::class_get_interfaces ) {
        return ifaces;
    }

    void * iter = nullptr;
    while ( auto iface = api::class_get_interfaces( klass, &iter ) ) {
        if ( !iface ) break;
        ifaces.emplace_back( iface );
    }

    return ifaces;
}


std::vector<Il2CppClass::FieldInfo> Il2CppClass::GetFields() const {
    std::vector<FieldInfo> fields;

    if ( !klass || !api::class_get_fields ) return fields;
   

    void * iter = nullptr;
    while (void* field = api::class_get_fields( klass, &iter )) {
        const char* name = api::field_get_name(field);
        void* ftype = api::field_get_type(field);
        uint32_t flags = api::field_get_flags(field);
        int32_t offset = api::field_get_offset(field);
        const char* tname = ftype ? api::type_get_name(ftype) : "object";
        fields.emplace_back(flags, tname, (name ? name : "unknown"), offset);
    }
    return fields;
}


std::vector<Il2CppClass::MethodInfo> Il2CppClass::GetMethods() const {
    std::vector<MethodInfo> methods;

    if (!klass || !api::class_get_methods) {
        return methods;
    }

    void* iter = nullptr;
    while (auto method = api::class_get_methods(klass, &iter)) {
        if (!method) break;

        const char* mname = api::method_get_name ? api::method_get_name(method) : nullptr;
        if (!mname || !*mname) continue;

        uint32_t flags = 0;
        if (api::method_get_flags) api::method_get_flags(method, &flags);

        void* rtype = api::method_get_return_type ? api::method_get_return_type(method) : nullptr;
        const char* rtname = rtype && api::type_get_name ? api::type_get_name(rtype) : "void";


        void* code_ptr = api::method_get_pointer ? api::method_get_pointer(method) : nullptr;

        uintptr_t rva = 0;
        if (code_ptr) {
            static uintptr_t moduleBase = (uintptr_t)GetModuleHandleA("GameAssembly.dll");
            rva = (uintptr_t)code_ptr - moduleBase;
        }

        uint32_t pcount = api::method_get_param_count ? api::method_get_param_count(method) : 0;
        std::vector<ParamInfo> params;
        for (uint32_t i = 0; i < pcount; ++i) {
        }

        methods.push_back({ flags, rtname, mname, params, rva });

    }

    return methods;
}