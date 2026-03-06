#include "../include/il2cpp_api.hxx"
#include "../include/utils.hxx"
#include <windows.h>


namespace api {

    bool initialized = false;

    // function pointers
    get_domain_t get_domain = nullptr;
    get_assemblies_t get_assemblies = nullptr;
    assembly_get_image_t assembly_get_image = nullptr;
    image_get_name_t image_get_name = nullptr;

    image_get_class_count_t image_get_class_count = nullptr;
    image_get_class_t image_get_class = nullptr;

    class_get_name_t class_get_name = nullptr;
    class_get_namespace_t class_get_namespace = nullptr;
    class_get_flags_t class_get_flags = nullptr;
    class_get_parent_t class_get_parent = nullptr;
    class_is_valuetype_t class_is_valuetype = nullptr;
    class_is_interface_t class_is_interface = nullptr;
    class_get_interfaces_t class_get_interfaces = nullptr;

    class_num_fields_t class_num_fields = nullptr;
    class_get_fields_t class_get_fields = nullptr;
    field_get_name_t field_get_name = nullptr;
    field_get_type_t field_get_type = nullptr;
    field_get_flags_t field_get_flags = nullptr;
    field_get_offset_t field_get_offset = nullptr;

    class_get_methods_t class_get_methods = nullptr;
    method_get_name_t method_get_name = nullptr;
    method_get_flags_t method_get_flags = nullptr;
    method_get_param_count_t method_get_param_count = nullptr;
    method_get_param_t method_get_param = nullptr;
    method_get_param_name_t method_get_param_name = nullptr;
    method_get_return_type_t method_get_return_type = nullptr;
    method_get_pointer_t method_get_pointer = nullptr;

    type_get_name_t type_get_name = nullptr;
    class_get_type_token_t class_get_type_token = nullptr;

    metadata_string_literal_from_index_t metadata_string_literal_from_index = nullptr;


    void api::init() {
        if (initialized) return;

        HMODULE gameAsm = GetModuleHandleA("GameAssembly.dll");
        if (!gameAsm) {
            Log("GameAssembly.dll not found.");
            return;
        }

        #define RESOLVE_API(name, target) \
            target = (target##_t)GetProcAddress(gameAsm, name); \
            if (!target) Log("[WARN] Could not resolve: " + std::string(name));

        // Core functions - fail early if missing
        RESOLVE_API("il2cpp_domain_get", get_domain);
        RESOLVE_API("il2cpp_domain_get_assemblies", get_assemblies);
        RESOLVE_API("il2cpp_assembly_get_image", assembly_get_image);
        RESOLVE_API("il2cpp_image_get_name", image_get_name);

        // class / image enumeration
        RESOLVE_API("il2cpp_image_get_class_count", image_get_class_count);
        RESOLVE_API("il2cpp_image_get_class", image_get_class);

        // class reflection
        RESOLVE_API("il2cpp_class_get_name", class_get_name);
        RESOLVE_API("il2cpp_class_get_namespace", class_get_namespace);
        RESOLVE_API("il2cpp_class_get_flags", class_get_flags);
        RESOLVE_API("il2cpp_class_get_parent", class_get_parent);

        RESOLVE_API("il2cpp_class_is_valuetype", class_is_valuetype);
        RESOLVE_API("il2cpp_class_is_interface", class_is_interface);
        RESOLVE_API("il2cpp_class_get_interfaces", class_get_interfaces);

        // fields
        RESOLVE_API("il2cpp_class_num_fields", class_num_fields);
        RESOLVE_API("il2cpp_class_get_fields", class_get_fields);
        RESOLVE_API("il2cpp_field_get_name", field_get_name);
        RESOLVE_API("il2cpp_field_get_type", field_get_type);
        RESOLVE_API("il2cpp_field_get_flags", field_get_flags);
        RESOLVE_API("il2cpp_field_get_offset", field_get_offset);

        // methods
        RESOLVE_API("il2cpp_class_get_methods", class_get_methods);
        RESOLVE_API("il2cpp_method_get_name", method_get_name);
        RESOLVE_API("il2cpp_method_get_flags", method_get_flags);
        RESOLVE_API("il2cpp_method_get_param_count", method_get_param_count);
        RESOLVE_API("il2cpp_method_get_param", method_get_param);
        RESOLVE_API("il2cpp_method_get_param_name", method_get_param_name);
        RESOLVE_API("il2cpp_method_get_return_type", method_get_return_type);
        RESOLVE_API("il2cpp_method_get_pointer", method_get_pointer);

        RESOLVE_API("il2cpp_type_get_name", type_get_name);
        RESOLVE_API("il2cpp_class_get_type_token", class_get_type_token);
        RESOLVE_API("il2cpp_metadata_string_literal_from_index", metadata_string_literal_from_index);


        if ( !get_domain || !get_assemblies || !assembly_get_image || !image_get_name ) {
            Log( "[ERROR] Failed to resolve core IL2CPP functions" );
            return;
        }

        initialized = (get_domain != nullptr);
        if (initialized) Log("[OK] IL2CPP API Initialized");
    }

}