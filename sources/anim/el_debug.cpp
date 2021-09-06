#include "el_debug.h"

#include <cstdio>
#include <cstdarg>

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

namespace el {

    void trace(const char* format...)
    {
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

    void debug_output(const char* message)
    {
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

}
