#include "../include/utils.hxx"
#include <iostream>
#include <direct.h>
#include <cerrno>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

std::ofstream logFile( "C:\\IL2CPPDump_Log.txt" );

// Mutex to protect console + file writes from multiple threads
static std::mutex s_logMutex;


void Log( const std::string & msg ) {
    std::lock_guard<std::mutex> lock( s_logMutex );
    std::cout << msg << "\n";
    if ( logFile.is_open( ) ) {
        logFile << msg << "\n";
        logFile.flush( );
    }
}


void EnsureDirectory( const std::string & path ) {
    if ( _mkdir( path.c_str( ) ) != 0 && errno != EEXIST ) {
        Log( "[ERROR] Failed to create directory: " + path );
    }
}


std::string GetAccessModifier( uint32_t flags ) {
    // Bottom 3 bits are the accessibility field (ECMA-335 II.23.1.5)
    uint32_t access = flags & 0x0007;

    switch ( access ) {
        case 0x0001: return "private";
        case 0x0002: return "private protected";  // FamANDAssem
        case 0x0003: return "internal";            // Assembly
        case 0x0004: return "protected";           // Family
        case 0x0005: return "protected internal";  // FamORAssem
        case 0x0006: return "public";
        default:     return "private";
    }
}


std::string GetCurrentTimestamp( ) {
    auto now = std::chrono::system_clock::now( );
    std::time_t t = std::chrono::system_clock::to_time_t( now );
    std::tm tm_buf{};
    localtime_s( &tm_buf, &t );

    std::ostringstream ss;
    ss << std::put_time( &tm_buf, "%Y-%m-%d %H:%M:%S" );
    return ss.str( );
}
