#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <windows.h>
#include <fstream>
#include <mutex>

// Global log file (thread-safe writes via Log())
extern std::ofstream logFile;

void Log( const std::string & msg );
void EnsureDirectory( const std::string & path );
std::string GetAccessModifier( uint32_t flags );
std::string GetCurrentTimestamp( );

#endif // UTILS_H
