#include "Uniform.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "mat4.h"
#include "quat.h"

template Uniform<int>;
template Uniform<ivec4>;
template Uniform<ivec2>;
template Uniform<float>;
template Uniform<vec2>;
template Uniform<vec3>;
template Uniform<vec4>;
template Uniform<quat>;
template Uniform<mat4>;

template Uniform<glm::ivec4>;
template Uniform<glm::ivec2>;
template Uniform<glm::vec3>;
template Uniform<glm::vec4>;
template Uniform<glm::mat4>;

#define UNIFORM_IMPL(gl_func, tType, dType) \
template<> \
void Uniform<tType>::Set(unsigned int slot, tType* data, unsigned int length) { \
	gl_func(slot, (GLsizei)length, (dType*)&data[0]); \
}

UNIFORM_IMPL(glUniform1iv, int, int)
UNIFORM_IMPL(glUniform4iv, ivec4, int)
UNIFORM_IMPL(glUniform2iv, ivec2, int)
UNIFORM_IMPL(glUniform1fv, float, float)
UNIFORM_IMPL(glUniform2fv, vec2, float)
UNIFORM_IMPL(glUniform3fv, vec3, float)
UNIFORM_IMPL(glUniform4fv, vec4, float)
UNIFORM_IMPL(glUniform4fv, quat, float)

UNIFORM_IMPL(glUniform4iv, glm::ivec4, int)
UNIFORM_IMPL(glUniform2iv, glm::ivec2, int)
UNIFORM_IMPL(glUniform2fv, glm::vec2, float)
UNIFORM_IMPL(glUniform3fv, glm::vec3, float)
UNIFORM_IMPL(glUniform4fv, glm::vec4, float)

template<>
void Uniform<mat4>::Set(unsigned int slot, mat4* inputArray, unsigned int arrayLength) {
	glUniformMatrix4fv(slot, (GLsizei)arrayLength, false, (float*)&inputArray[0]);
}

template<>
void Uniform<glm::mat4>::Set(unsigned int slot, glm::mat4* inputArray, unsigned int arrayLength) {
	glUniformMatrix4fv(slot, (GLsizei)arrayLength, false, (float*)&inputArray[0]);
}

template <typename T>
void Uniform<T>::Set(unsigned int slot, const T& value) {
	Set(slot, (T*)&value, 1);
}

template <typename T>
void Uniform<T>::Set(unsigned int slot, std::vector<T>& value) {
	Set(slot, &value[0], (unsigned int)value.size());
}
