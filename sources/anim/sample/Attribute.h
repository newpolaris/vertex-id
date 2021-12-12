#ifndef _H_ATTRIBUTE_
#define _H_ATTRIBUTE_

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

template<typename T>
class Attribute {
protected:
	unsigned int mHandle;
	unsigned int mCount;
private:
	Attribute(const Attribute& other);
	Attribute& operator=(const Attribute& other);
	void SetAttribPointer(unsigned int slot);
public:
	Attribute();
	~Attribute();

	void Set(T* inputArray, unsigned int arrayLength);
	void Set(std::vector<T>& input);

	void BindTo(unsigned int slot);
	void UnBindFrom(unsigned int slot);

	unsigned int Count();
	unsigned int GetHandle();
};

template<typename T>
Attribute<T>::Attribute() {
    glGenBuffers(1, &mHandle);
    mCount = 0;
}

template<typename T>
Attribute<T>::~Attribute() {
    glDeleteBuffers(1, &mHandle);
}

template<typename T>
void Attribute<T>::Set(T* inputArray, unsigned int arrayLength) {
    mCount = arrayLength;
    unsigned int size = sizeof(T);

    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    glBufferData(GL_ARRAY_BUFFER, size * mCount, inputArray, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

template<typename T>
void Attribute<T>::Set(std::vector<T>& input) {
    Set(&input[0], (unsigned int)input.size());
}

template<typename T>
void Attribute<T>::BindTo(unsigned int slot) {
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    auto e = glGetError();

    glEnableVertexAttribArray(slot);
    e = glGetError();

    SetAttribPointer(slot);
    e = glGetError();

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    e = glGetError();

}

template<typename T>
void Attribute<T>::UnBindFrom(unsigned int slot) {
    glBindBuffer(GL_ARRAY_BUFFER, mHandle);
    glDisableVertexAttribArray(slot);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

#endif
