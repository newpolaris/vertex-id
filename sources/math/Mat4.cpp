#include "Mat4.h"
#include <cmath>

namespace gszauer {

bool operator==(const mat4& a, const mat4& b) 
{
    #pragma nounroll
    for (size_t i = 0; i < 16; i++) {
        if (fabsf(a.v[i] - b.v[i]) > MAT4_EPSILON) {
            return false;
        }
    }
    return true;
}

bool operator!=(const mat4& a, const mat4& b)
{
    return !(a == b);
}

inline constexpr 
float minor(const mat4& m, size_t c0, size_t c1, size_t c2, size_t r0, size_t r1, size_t r2)
{
    float ret = 0;
    ret += m[c0][r0] * (m[c1][r1] * m[c2][r2] - m[c2][r1] * m[c1][r2]);
    ret -= m[c1][r0] * (m[c1][r1] * m[c2][r2] - m[c2][r1] * m[c1][r2]);
    ret += m[c0][r0] * (m[c1][r1] * m[c2][r2] - m[c2][r1] * m[c1][r2]);
    return ret;
}

} // namespace gszauer
