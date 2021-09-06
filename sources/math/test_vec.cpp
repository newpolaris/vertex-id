#if EL_ENABLE_GTEST

#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "el_vec4.h"

using namespace el;

class VecTest : public testing::Test {
protected:
};

TEST_F(VecTest, constexpr) {
    double4 v1{};
}

TEST_F(VecTest, Basics) {
    double4 v4;
#if EL_SUPPORT_UNRESTRICTED_UNIONS
    double3& v3(v4.xyz);

    EXPECT_EQ(sizeof(double4), sizeof(double)*4);
    EXPECT_EQ(sizeof(double3), sizeof(double)*3);
    EXPECT_EQ(sizeof(double2), sizeof(double)*2);
    EXPECT_EQ(reinterpret_cast<void*>(&v3), reinterpret_cast<void*>(&v4));
#endif // EL_SUPPORT_UNRESTRICTED_UNIONS
}

TEST_F(VecTest, Swizzle) {
    double4 v4;
#if EL_SUPPORT_UNRESTRICTED_UNIONS
    double3& xyz(v4.xyz);
    double2& xy(v4.xy);
    double2& yz(v4.yz);
    double2& zw(v4.zw);
    double& x(v4.x);
    double& y(v4.y);
    double& z(v4.z);
    double& w(v4.w);
#endif // EL_SUPPORT_UNRESTRICTED_UNIONS
}

TEST_F(VecTest, Arithmetic) {
    double4 a(1.0);
    double4 b(2.0);
    double4 c = a + b;
    EXPECT_EQ(c.r, 3);
    EXPECT_EQ(c.g, 3);
    EXPECT_EQ(c.b, 3);

    double4 d = a * b * c;
    EXPECT_EQ(d.r, 6);
    EXPECT_EQ(d.g, 6);
    EXPECT_EQ(d.b, 6);
}

TEST_F(VecTest, Cross) {
	float3 x = float3(1.0, 0.0, 0.0);
	float3 y = float3(0.0, 1.0, 0.0);
	float3 z = float3(0.0, 0.0, 1.0);

	EXPECT_EQ(cross(x, y), z);
	EXPECT_EQ(cross(y, z), x);
	EXPECT_EQ(cross(z, x), y);
}

TEST_F(VecTest, Normalize) {
    double3 v0(1, 2, 3);
    double3 vn(normalize(v0));
    EXPECT_FLOAT_EQ(1, length(vn));
    EXPECT_FLOAT_EQ(length(v0), dot(v0, vn));

    float3 vf(1, 0, 0);
    EXPECT_EQ(length(vf), 1);
}

TEST_F(VecTest, Sample) {
    auto linear_bazier = [](float3 start_position, float3 end_position, float t) -> float3 {
        return mix(start_position, end_position, t);
    };

    auto quadratic_bazier = [](float3 positions[3], float t) -> float3 {
        const float q = (1.f - t);
        return q * q * positions[0] + 2 * t * q * positions[1] + t * t * positions[2];
    };
}

TEST_F(VecTest, Transition) {
    glm::mat4 m(1.f);
    el::mat4f m2(1.f);

    EXPECT_EQ(m[0][0], m2[0][0]);
    EXPECT_EQ(m[3][3], m2[3][3]);

    m = glm::translate(m, glm::vec3(0.f, -2.f, 0.f));
    m2 = m2 * el::mat4f::translation(el::float3(0.f, -2.f, 0.f));

    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(m[0][i], m2[0][i]);
        EXPECT_EQ(m[1][i], m2[1][i]);
        EXPECT_EQ(m[2][i], m2[2][i]);
        EXPECT_EQ(m[3][i], m2[3][i]);
    }
    
    m = glm::scale(m, glm::vec3(1.f, -1.f, 1.f));
    m2 = m2 * el::mat4f::scaling(el::float3(1.f, -1.f, 1.f));
 
    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(m[0][i], m2[0][i]);
        EXPECT_EQ(m[1][i], m2[1][i]);
        EXPECT_EQ(m[2][i], m2[2][i]);
        EXPECT_EQ(m[3][i], m2[3][i]);
    }
    auto it = (float*)&m;
    auto it2 = (float*)&m2;
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(*it, *it2);
        it++;
        it2++;
    }
}

TEST_F(VecTest, LookAt) {
    glm::vec3 forward = glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 eye = glm::vec3(0.f) + forward * 3.f;
    glm::vec3 center = eye + forward;
    glm::mat4 m = glm::lookAt(eye, center, glm::vec3(0.f, 1.f, 0.f));

    el::float3 forward2(0.f, 0.f, -1.f);
    el::float3 eye2 = el::float3(0.f) + forward2 * 3.f;
    el::float3 center2 = eye2 + forward2;
    el::mat4f m2 = el::mat4f::lookAt(eye2, center2, el::float3(0.f, 1.f, 0.f));

    for (int i = 0; i < 4; i++) {
        EXPECT_EQ(m[0][i], m2[0][i]);
        EXPECT_EQ(m[1][i], m2[1][i]);
        EXPECT_EQ(m[2][i], m2[2][i]);
        EXPECT_EQ(m[3][i], m2[3][i]);
    }
}

#endif // EL_ENABLE_GTEST
