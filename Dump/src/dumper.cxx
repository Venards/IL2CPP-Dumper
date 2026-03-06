#include "../include/dumper.hxx"
#include "../include/il2cpp_api.hxx"
#include "../include/utils.hxx"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <map>
#include <vector>
#include <iomanip>

Dumper::Dumper() {
    api::init();
    Sleep(500);
    if (!api::initialized) return;

    void* domain = api::get_domain();
    if (!domain) {
        Log("[ERROR] No domain");
        return;
    }

    size_t count = 0;
    void** assemblies = api::get_assemblies(domain, &count);
    if (!assemblies || count == 0) {
        Log("[ERROR] No assemblies");
        return;
    }

    for (size_t i = 0; i < count; ++i) {
        void* ass = assemblies[i];
        if (!ass) continue;

        void* img = api::assembly_get_image(ass);
        if (!img) continue;

        const char* name = api::image_get_name(img);
        if (!name || !*name) continue;

        images.emplace_back(img);
    }
}

void Dumper::DumpStrings(const std::string& folder) {
    Log("Extracting strin literals");


    std::ofstream out(folder + "strings.txt");
    std::ofstream jOut(folder + "strings.json");
    if (!out || !jOut) return;

    out << "// IL2CPP String Literal Dump\n\n";
    jOut << "[\n";

    bool first = true;
    for (uint32_t i = 0; ; ++i) {
        const char* str = api::metadata_string_literal_from_index(i);

        if (!str || !*str) {
            bool foundMore = false;
            for (uint32_t j = 1; j <= 50; ++j) {
                if (api::metadata_string_literal_from_index(i + j)) {
                    foundMore = true;
                    break;
                }
            }
            if (!foundMore) break;
            continue;
        }
        
        out << "Index[" << i << "]: " << str << "\n";

        if (!first) jOut << ",\n";
        jOut << " { \"Index\": " << i << ", \"Value\": \"" << str << "\" }";
        first = false;


        if (i % 5000 == 0 && i != 0) {
            Log(std::string(" Processing string #") + std::to_string(i));
        }
    }

    jOut << "\n";
    out.close();
    jOut.close();
    Log("Done! String databases created.");
}

void Dumper::DumpAssembly(const Il2CppImage& img, bool aiMode) {
    std::string asmName = img.GetName();
    std::string folder = aiMode ? "C:\\IL2CPP_Dump_AI\\" : "C:\\IL2CPP_Dump_Normal\\";

    std::error_code ec;
    std::filesystem::create_directories(folder, ec);

    std::map<std::string, std::vector<Il2CppClass>> ns_classes;
    for (size_t i = 0; i < img.GetClassCount(); ++i) {
        auto cls = img.GetClassByIndex(i);
        if (!cls.klass) continue;
        ns_classes[cls.GetNamespace()].push_back(cls);
    }

    std::stringstream cs;
    cs << "// Remastered IL2CPP Dump | Assembly: " << asmName << "\n\n";

    std::stringstream json;
    json << "{\n  \"Assembly\": \"" << asmName << "\",\n  \"Classes\": [\n";

    bool firstClass = true;

    for (const auto& [ns, classes] : ns_classes) {
        if (!aiMode && !ns.empty()) cs << "namespace " << ns << "\n{\n";

        for (const auto& cls : classes) {
            if (!firstClass) json << ",\n";
            firstClass = false;

            json << "    {\n      \"Name\": \"" << cls.GetName() << "\",\n";
            json << "      \"Namespace\": \"" << ns << "\",\n";
            json << "      \"Fields\": [\n";
            json << "      \"Methods\": [\n";

            cs << "    public class " << cls.GetName() << "\n    {\n";

            auto fields = cls.GetFields();
            cs << "      // Fields\n";
            json << "      \"Fields\": [\n";

            for (size_t i = 0; i < fields.size(); ++i) {
                uint32_t f_flags; std::string f_type, f_name; int32_t f_offset;
                std::tie(f_flags, f_type, f_name, f_offset) = fields[i];

                cs << "      " << GetAccessModifier(f_flags) << " " << f_type << " " << f_name << "; // Offset: 0x"
                   << std::hex << std::uppercase << f_offset << std::dec << "\n";

                json << "      { \"Name\": \"" << f_name << "\", \"Type\": \"" << f_type << "\", \"Offset\": \"0x"
                     << std::hex << std::uppercase << f_offset << std::dec << "\n";

                if (i < fields.size() - 1) json << ",";
                json << "\n";
            }
            json << "      ],\n";


            auto methods = cls.GetMethods();
            for (size_t i = 0; i < methods.size(); ++i) {
                uint32_t mf; std::string rt, mn;
                std::vector<Il2CppClass::ParamInfo> ps;
                uintptr_t rva = 0;
                std::tie(mf, rt, mn, ps, rva) = methods[i];

                cs << "        " << rt << " " << mn << "(); // RVA: 0x";
                cs << std::hex << std::uppercase << static_cast<unsigned __int64>(rva) << std::dec << "\n";

                json << "        { \"Name\": \"" << mn << "\", \"RVA\": \"0x";
                json << std::hex << std::uppercase << static_cast<unsigned __int64>(rva) << std::dec;
                json << "\" }";

                if (i < methods.size() - 1) json << ",";
                json << "\n";
            }

            cs << "    }\n\n";
            json << "      ]\n    }";
        }
        if (!aiMode && !ns.empty()) cs << "}\n";
    }
    json << "\n  ]\n}";

    std::ofstream out(folder + asmName + ".cs");
    if (out) out << cs.str();

    std::ofstream jOut(folder + asmName + ".json");
    if (jOut) jOut << json.str();

    Log("Dumped Assembly and JSON Map: " + asmName);
}

void Dumper::DumpAllToFiles() {
    std::string normalPath = "C\\IL2CPP_Dump_Normal\\";
    std::string aiPath = "C:\\IL2CPP_Dump_AI";


    EnsureDirectory(normalPath);
    EnsureDirectory(aiPath);

    Log("Starting String Discovery...");
    DumpStrings(normalPath);


    for (const auto& img : images) {
        Log(std::string("Dumping: ") + img.GetName());
        DumpAssembly(img, false);
        DumpAssembly(img, true);
    }

    Log("\nDone!");
    Log("\n[SUCCESS] All metadata recovered!");
    Log("Check C\\IL2CPP_Dump_Normal\\strings.txt for your game data.");
    Log("  Normal (C#):      C:\\IL2CPP_Dump_Normal\\");
    Log("  AI-friendly:      C:\\IL2CPP_Dump_AI\\");
}