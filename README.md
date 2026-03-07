# IL2CPP Dumper — Unity Games

Simple injectable DLL that dumps IL2CPP metadata from Unity games.

> Fork of [DeftSolutions-dev/IL2CPP-Dumper](https://github.com/DeftSolutions-dev/IL2CPP-Dumper)

---

### Features
- Dumps every assembly from `GameAssembly.dll`
- Full metadata recovery — classes, fields, methods, namespaces, interfaces, enums, nested types
- Field offsets, RVAs, access modifiers, inheritance, generics
- Two output formats:
  - **C# style** — readable `.cs` files with proper syntax
  - **AI-friendly** — structured plain text for LLMs and automated tooling
- Press **INSERT** to dump

### What's New (v1.2.0)
- SEH-safe memory reads — bad pointers no longer crash the dumper
- IDA-style pattern scanner with Boyer-Moore-Horspool — faster, works on stripped exports
- Function resolver — falls back to pattern scan if exports are missing
- RVA calculator with file offset lookup for IDA/Ghidra workflows
- Resolve report printed at startup showing how each function was found

### Output
| Path | Contents |
|------|----------|
| `C:\IL2CPP_Dump_Normal\` | C# style `.cs` files |
| `C:\IL2CPP_Dump_AI\` | AI-friendly `.txt` files |
| `C:\IL2CPPDump_Log.txt` | Session log + resolve report |

### Supported
- IL2CPP metadata `≈27–29`
- Unity `2021.3` → `2024.x`
- x64 only
