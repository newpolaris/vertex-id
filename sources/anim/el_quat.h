#ifndef __EL_QUAT_H__
#define __EL_QUAT_H__

#include "el_vec4.h"

namespace el {
namespace details {

    template <typename T>
    class EL_EMPTY_BASES TQuat  :
        public TVecAddOperators<TVec4, T>
    {
    public:

        union {
            struct { T x, y, z, w; };
            TVec4<T> xyzw;
            TVec3<T> xyz;
            TVec2<T> xy;
        };

        TQuat() : x(0), y(0), z(0), w(1) {}

        template <typename A, typename B, typename C, typename D>
        TQuat(A x, B y, C z, D w) : x(x), y(y), z(z), w(w) {}

        template <typename A, typename B>
        TQuat(const TVec3<A>& v, B w) : x(v.x), y(v.y), z(v.z), w(w) {}

        // initialize from a quaternion of a different type
        template<typename A, typename = enable_if_arithmetic_t<A>>
        explicit TQuat(const TQuat<A>& v): x(v.x), y(v.y), z(v.z), w(v.w) {}

        template <typename A, typename B>
        constexpr static TQuat fromAxisAngle(const TVec3<A>& axis, B angle) {
            return TQuat(std::sin(angle * A(0.5)) * normalize(axis), std::cos(angle * B(0.5)));
        }
    };

} // namespace detail

typedef details::TQuat<double> quat;
typedef details::TQuat<float> quatf;

} // namespace el

#endif // __EL_QUAT_H__
