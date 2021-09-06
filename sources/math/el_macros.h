#ifndef __EL_MACROS_H__
#define __EL_MACROS_H__

#include <type_traits>

#include "el_platform.h"

#if EL_COMP_GCC || EL_COMP_CLANG
#	define EL_ALIGN_DECL(_align, _decl) _decl __attribute__((aligned(_align)))
#elif EL_COMP_MSVC
#	define EL_ALIGN_DECL(_align, _decl) __declspec(align(_align)) _decl
#endif

#if EL_COMP_MSVC // eq. with _MSC_VER

// https://stackoverflow.com/questions/12701469/why-is-the-empty-base-class-optimization-ebo-is-not-working-in-msvc
// MSVS does not support EBO for multiple inheritance
#   define EL_EMPTY_BASES __declspec(empty_bases)

#else

#   define EL_EMPTY_BASES 

#endif

// https://docs.microsoft.com/en-us/previous-versions/hh567368(v=vs.140)
// Unrestricted unions
#define EL_SUPPORT_UNRESTRICTED_UNIONS 0
#if _MSC_VER >= 1900
#undef EL_SUPPORT_UNRESTRICTED_UNIONS
#define EL_SUPPORT_UNRESTRICTED_UNIONS 1
#endif

#endif // __EL_MACROS_H__
