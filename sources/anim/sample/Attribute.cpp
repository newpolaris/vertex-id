#pragma warning(disable : 26451)
#include "Attribute.h"
#undef APIENTRY


#include "vec2.h"
#include "vec3.h"
#include "vec4.h"
#include "quat.h"

template Attribute<int>;
template Attribute<float>;
template Attribute<vec2>;
template Attribute<vec3>;
template Attribute<vec4>;
template Attribute<ivec4>;
template Attribute<quat>;

template Attribute<glm::vec2>;
template Attribute<glm::vec3>;
template Attribute<glm::vec4>;


template<typename T>
unsigned int Attribute<T>::Count() {
	return mCount;
}

template<typename T>
unsigned int Attribute<T>::GetHandle() {
	return mHandle;
}

template<>
void Attribute<int>::SetAttribPointer(unsigned int slot) {
	glVertexAttribIPointer(slot, 1, GL_INT, 0, (void*)0);
}

template<>
void Attribute<ivec4>::SetAttribPointer(unsigned int slot) {
	glVertexAttribIPointer(slot, 4, GL_INT, 0, (void*)0);
}

template<>
void Attribute<float>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<>
void Attribute<vec2>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<>
void Attribute<vec3>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<>
void Attribute<vec4>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<>
void Attribute<quat>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<>
void Attribute<glm::vec2>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<>
void Attribute<glm::vec3>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
}

template<>
void Attribute<glm::vec4>::SetAttribPointer(unsigned int slot) {
	glVertexAttribPointer(slot, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
}
