#ifndef __EL_PREDEFINE_H__
#define __EL_PREDEFINE_H__

#define EL_CONFIG_DEBUG 0

#if defined(_DEBUG) || !defined(NDEBUG)
#   undef EL_CONFIG_DEBUG
#   define EL_CONFIG_DEBUG 1
#endif

// target: osx legacy support (framebuffer and vao supported)
#ifndef EL_BUILD_OPENGL
#define EL_BUILD_OPENGL 1
#endif
// target: ios and android legacy
#ifndef EL_BUILD_OPENGL_ES2
#define EL_BUILD_OPENGL_ES2 0
#endif
#ifndef EL_BUILD_OPENGL_ES3
#define EL_BUILD_OPENGL_ES3 0
#endif
// target: 4.3 over; not supported by osx (< 4.1)
#ifndef EL_BUILD_OPENGL_CORE
#define EL_BUILD_OPENGL_CORE 0
#endif
#ifndef EL_BUILD_METAL
#define EL_BUILD_METAL 0
#endif

//#if defined(__cplusplus)
    #define _EL_NAME_BEGIN namespace el {
    #define _EL_NAME_END }
    #define _EL_NAME ::el::
//#else
//#endif

#endif // __EL_PREDEFINE_H__
