#include <cstdio>
#include <vector>
#include <stdarg.h>
#include <iostream>
#include "util.h"

#if _WIN32
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char* _str);
#else
#   if defined(__OBJC__)
#       import <Foundation/NSObjCRuntime.h>
#   else
#       include <CoreFoundation/CFString.h>
extern "C" void NSLog(CFStringRef _format, ...);
#   endif
#endif

void debug_output(const char* message) {
#if _WIN32
    fprintf(stderr, message);
    OutputDebugStringA(message);
#else
#   if defined(__OBJC__)
    NSLog(@"%s", message);
#   else
    NSLog(CFSTR("%s"), message);
#   endif
#endif
}

void trace(const char* format...) {
    const int kLength = 1024;
    char buffer[kLength + 1] = { 0, };

    va_list argList;
    va_start(argList, format);
    int len = vsnprintf(buffer, kLength, format, argList);
    va_end(argList);
    if (len > kLength)
        len = kLength;
    buffer[len] = '\0';

    debug_output(buffer);
}


std::string getFileAsString(const std::string& name) {
    FILE* file = fopen(name.c_str(), "r");
    assert(file);

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    std::vector<char> buffer(size);
    fseek(file, 0, SEEK_SET);
    fread(buffer.data(), size, 1, file);
    fclose(file);
    return std::string(buffer.begin(), buffer.end());
}
