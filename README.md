Forked by @wg5h on discord.
This fork is personalized to my liking
## BEWARE THIS IS STILL IN IT'S TESTING PERIOD. EXPECT BUGS!


-- Update log (DD/MM/YY)

V1.0 release - 03/07/26 - 5:30 am
 - [+] Added Relative Virtual Address
     - IDA Pro and Ghidra Support
 - [+] Added Field Offset Recovery
 - [+] Added Global String Discovery
 - [+] Namespace Grouping to mimic original source code hierarchy 


# IL2CPP Dumper for Arknights: Endfield & Unity Games

Simple injectable DLL that dumps IL2CPP metadata from Unity games (classes, fields, methods, namespaces, interfaces, etc.).

Originally created for **Arknights: Endfield**.

### Features
- Dumps every assembly from `GameAssembly.dll`
- Two output formats:
  - **Classic C# style** - readable `.cs` files with proper syntax, access modifiers, inheritance & interfaces
  - **AI-friendly** - structured plain text, perfect for LLMs, code generation or parsing
- Activation: press **INSERT** after DLL injection
- Output paths:
  - Normal dumps -> `C:\IL2CPP_Dump_Normal\`
  - AI dumps    -> `C:\IL2CPP_Dump_AI\`
- Log file -> `C:\IL2CPPDump_Log.txt`

### Supported IL2CPP versions
IL2CPP metadata `≈27-29` (Unity 2021.3 - 2023.x - 2024.x builds)

### Example outputs

**Classic C# format** (looks almost like real source code):

![Classic C# dump example](https://github.com/user-attachments/assets/9af853d4-249c-46c7-bdd5-498f3b191180)

**AI-friendly structured format** (easy for AI tools):

![AI-friendly dump example](https://github.com/user-attachments/assets/cc9e636c-6ca3-4c1d-af51-2ee3bf7e0444)

### Usage
1. Inject the DLL into the game process (any injector works)
2. Press **INSERT** while in-game
3. Wait for completion (watch the console and log file)

Great for reverse engineering, modding, cheat development or feeding game structure into AI models.

Contributions, issues and stars are welcome!
