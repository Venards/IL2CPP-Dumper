# IL2CPP Dumper for Arknights: Endfield & Unity Games

Simple injectable DLL that dumps IL2CPP metadata from Unity games (classes, fields, methods, namespaces, interfaces, etc.).

### Features

1. Dumps every assembly from `GameAssembly.dll`

2. Recovers full metadata including:

   * `Classes`, `Fields`, `Methods`, `Namespaces`, `Interfaces`

3. Advanced metadata recovery:

   * Field Offset Recovery
   * Relative Virtual Address (RVA)
   * Global String Discovery
   * Namespace Grouping

4. Two output formats:

   * **Classic C# style** – readable `.cs` files with proper syntax, access modifiers, inheritance, and interfaces
   * **AI-friendly format** – structured plain text designed for LLMs, parsing, and automated tooling

5. Simple activation system:

   * Inject the DLL
   * Press **INSERT** to start dumping

### Output

* Normal dumps -> `C:\IL2CPP_Dump_Normal\`
* AI dumps -> `C:\IL2CPP_Dump_AI\`
* Log file -> `C:\IL2CPPDump_Log.txt`

### Supported IL2CPP Versions

* IL2CPP metadata `≈27–29`
* Unity `2021.3` – `2023.x` – `2024.x` builds

### Update Log (DD/MM/YY)

**V1.0 release – 03/07/26 – 5:30 AM**

* Added Relative Virtual Address (RVA)
* Added Field Offset Recovery
* Added Global String Discovery
* Added Namespace Grouping to mimic original source structure

> This is a fork from [DeftSolutions-dev/IL2CPP-Dumper](https://github.com/DeftSolutions-dev/IL2CPP-Dumper)
