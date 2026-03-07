#ifndef IL2CPP_CLASS_H
#define IL2CPP_CLASS_H

#include <vector>
#include <string>
#include <cstdint>
#include "il2cpp_api.hxx"

class Il2CppClass {

public:
    Il2CppClassOpaque * klass = nullptr;

    explicit Il2CppClass( Il2CppClassOpaque * k = nullptr ) : klass( k ) { }

    const char * GetName( )      const;
    const char * GetNamespace( ) const;

    bool IsValueType( )  const;
    bool IsInterface( )  const;
    bool IsEnum( )       const;

    uint32_t GetFlags( )     const;
    uint32_t GetTypeToken( ) const;

    Il2CppClass              GetParent( )     const;
    std::vector<Il2CppClass> GetInterfaces( ) const;
    std::vector<Il2CppClass> GetNestedTypes( ) const;


    // ---- Strongly-typed info structs (replaces fragile tuples) ----

    struct FieldInfo {
        uint32_t    flags;
        std::string typeName;
        std::string name;
        int32_t     offset;
    };

    struct ParamInfo {
        std::string typeName;
        std::string name;
    };

    struct MethodInfo {
        uint32_t               flags;
        std::string            returnType;
        std::string            name;
        uintptr_t              rva;        // in-memory function address (0 if unavailable)
        std::vector<ParamInfo> params;
    };

    std::vector<FieldInfo>  GetFields( )  const;
    std::vector<MethodInfo> GetMethods( ) const;

};

#endif // IL2CPP_CLASS_H
