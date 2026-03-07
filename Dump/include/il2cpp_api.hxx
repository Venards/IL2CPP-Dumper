#ifndef IL2CPP_API_H
#define IL2CPP_API_H

#include <cstddef>
#include <cstdint>
#include <string>

// Opaque forward declarations for type safety
struct Il2CppDomainOpaque;
struct Il2CppAssemblyOpaque;
struct Il2CppImageOpaque;
struct Il2CppClassOpaque;
struct Il2CppFieldOpaque;
struct Il2CppMethodOpaque;
struct Il2CppTypeOpaque;

namespace api {

    extern bool initialized;

    enum class ResolveMethod : uint8_t {
        None    = 0,
        Export  = 1,   // GetProcAddress
        Pattern = 2,   // pattern scan fallback
    };

    const char * ResolveName( ResolveMethod m );

    // ---- Function pointer types ----
    typedef Il2CppDomainOpaque *    ( __fastcall * get_domain_t              )( );
    typedef Il2CppAssemblyOpaque ** ( __fastcall * get_assemblies_t          )( Il2CppDomainOpaque *, size_t * );
    typedef Il2CppImageOpaque *     ( __fastcall * assembly_get_image_t      )( Il2CppAssemblyOpaque * );
    typedef const char *            ( __fastcall * image_get_name_t          )( Il2CppImageOpaque * );
    typedef size_t                  ( __fastcall * image_get_class_count_t   )( Il2CppImageOpaque * );
    typedef Il2CppClassOpaque *     ( __fastcall * image_get_class_t         )( Il2CppImageOpaque *, size_t );
    typedef const char *            ( __fastcall * class_get_name_t          )( Il2CppClassOpaque * );
    typedef const char *            ( __fastcall * class_get_namespace_t     )( Il2CppClassOpaque * );
    typedef uint32_t                ( __fastcall * class_get_flags_t         )( Il2CppClassOpaque * );
    typedef Il2CppClassOpaque *     ( __fastcall * class_get_parent_t        )( Il2CppClassOpaque * );
    typedef bool                    ( __fastcall * class_is_valuetype_t      )( Il2CppClassOpaque * );
    typedef bool                    ( __fastcall * class_is_interface_t      )( Il2CppClassOpaque * );
    typedef bool                    ( __fastcall * class_is_enum_t           )( Il2CppClassOpaque * );
    typedef Il2CppClassOpaque *     ( __fastcall * class_get_interfaces_t    )( Il2CppClassOpaque *, void ** );
    typedef Il2CppClassOpaque *     ( __fastcall * class_get_nested_types_t  )( Il2CppClassOpaque *, void ** );
    typedef size_t                  ( __fastcall * class_num_fields_t        )( Il2CppClassOpaque * );
    typedef Il2CppFieldOpaque *     ( __fastcall * class_get_fields_t        )( Il2CppClassOpaque *, void ** );
    typedef const char *            ( __fastcall * field_get_name_t          )( Il2CppFieldOpaque * );
    typedef Il2CppTypeOpaque *      ( __fastcall * field_get_type_t          )( Il2CppFieldOpaque * );
    typedef uint32_t                ( __fastcall * field_get_flags_t         )( Il2CppFieldOpaque * );
    typedef int32_t                 ( __fastcall * field_get_offset_t        )( Il2CppFieldOpaque * );
    typedef Il2CppMethodOpaque *    ( __fastcall * class_get_methods_t           )( Il2CppClassOpaque *, void ** );
    typedef const char *            ( __fastcall * method_get_name_t             )( Il2CppMethodOpaque * );
    typedef uint32_t                ( __fastcall * method_get_flags_t            )( Il2CppMethodOpaque *, uint32_t * );
    typedef uint32_t                ( __fastcall * method_get_param_count_t      )( Il2CppMethodOpaque * );
    typedef Il2CppTypeOpaque *      ( __fastcall * method_get_param_t            )( Il2CppMethodOpaque *, uint32_t );
    typedef const char *            ( __fastcall * method_get_param_name_t       )( Il2CppMethodOpaque *, uint32_t );
    typedef Il2CppTypeOpaque *      ( __fastcall * method_get_return_type_t      )( Il2CppMethodOpaque * );
    typedef const void *            ( __fastcall * method_get_function_pointer_t )( Il2CppMethodOpaque * );
    typedef const char *            ( __fastcall * type_get_name_t               )( Il2CppTypeOpaque * );
    typedef uint32_t                ( __fastcall * class_get_type_token_t         )( Il2CppClassOpaque * );

    // ---- Instances ----
    extern get_domain_t                    get_domain;
    extern get_assemblies_t                get_assemblies;
    extern assembly_get_image_t            assembly_get_image;
    extern image_get_name_t                image_get_name;
    extern image_get_class_count_t         image_get_class_count;
    extern image_get_class_t               image_get_class;
    extern class_get_name_t                class_get_name;
    extern class_get_namespace_t           class_get_namespace;
    extern class_get_flags_t               class_get_flags;
    extern class_get_parent_t              class_get_parent;
    extern class_is_valuetype_t            class_is_valuetype;
    extern class_is_interface_t            class_is_interface;
    extern class_is_enum_t                 class_is_enum;
    extern class_get_interfaces_t          class_get_interfaces;
    extern class_get_nested_types_t        class_get_nested_types;
    extern class_num_fields_t              class_num_fields;
    extern class_get_fields_t              class_get_fields;
    extern field_get_name_t                field_get_name;
    extern field_get_type_t                field_get_type;
    extern field_get_flags_t               field_get_flags;
    extern field_get_offset_t              field_get_offset;
    extern class_get_methods_t             class_get_methods;
    extern method_get_name_t               method_get_name;
    extern method_get_flags_t              method_get_flags;
    extern method_get_param_count_t        method_get_param_count;
    extern method_get_param_t              method_get_param;
    extern method_get_param_name_t         method_get_param_name;
    extern method_get_return_type_t        method_get_return_type;
    extern method_get_function_pointer_t   method_get_function_pointer;
    extern type_get_name_t                 type_get_name;
    extern class_get_type_token_t          class_get_type_token;

    void init( );
    void PrintResolveReport( );

} // namespace api

#endif // IL2CPP_API_H
