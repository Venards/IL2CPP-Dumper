#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include "../include/utils.hxx"
#include "../include/dumper.hxx"


static void SetupConsole( ) {
    AllocConsole( );

    FILE * fDummy = nullptr;
    freopen_s( &fDummy, "CONOUT$", "w", stdout );
    freopen_s( &fDummy, "CONIN$",  "r", stdin  );

    // Enable ANSI escape codes in Windows 10+ console
    HANDLE hOut = GetStdHandle( STD_OUTPUT_HANDLE );
    if ( hOut != INVALID_HANDLE_VALUE ) {
        DWORD mode = 0;
        GetConsoleMode( hOut, &mode );
        SetConsoleMode( hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING );
    }

    SetConsoleTitleA( "IL2CPP Dumper - @wg5h | Press INSERT to dump" );
}


DWORD WINAPI EntryPoint( LPVOID lpParam ) {
    HMODULE hModule = reinterpret_cast<HMODULE>( lpParam );

    SetupConsole( );

    Log( "=================================================" );
    Log( "  IL2CPP Dumper - @wg5h" );
    Log( "  Press INSERT to begin dump" );
    Log( "=================================================" );

    while ( true ) {
        if ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) {
            // Debounce: wait for key release before proceeding
            while ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) Sleep( 10 );

            Log( "" );
            Log( "[>>>] Starting dump..." );

            Dumper dumper;

            if ( dumper.images.empty( ) ) {
                Log( "[ERROR] No assemblies found — dump aborted" );
            } else {
                Log( "[OK] Found " + std::to_string( dumper.images.size( ) ) + " assemblies" );
                dumper.DumpAllToFiles( );
            }

            break;
        }

        Sleep( 50 );
    }

    Log( "" );
    Log( "Press ENTER to close..." );
    std::cin.get( );

    FreeConsole( );
    FreeLibraryAndExitThread( hModule, 0 );
    return 0;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD reason, LPVOID lpReserved ) {
    if ( reason == DLL_PROCESS_ATTACH ) {
        DisableThreadLibraryCalls( hModule );
        CreateThread( nullptr, 0, EntryPoint, hModule, 0, nullptr );
    }
    return TRUE;
}
