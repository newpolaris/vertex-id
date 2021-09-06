#ifndef _H_UNIFORM_
#define _H_UNIFORM_

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

template <typename T>
class Uniform {
private:
	Uniform();
	Uniform(const Uniform&);
	Uniform& operator=(const Uniform&);
	~Uniform();
public:
	static void Set(unsigned int slot, const T& value);
	static void Set(unsigned int slot, T* inputArray, unsigned int arrayLength);
	static void Set(unsigned int slot, std::vector<T>& inputArray);
};


template <typename T>
void Uniform<T>::Set(unsigned int slot, const T& value) {
    Set(slot, (T*)&value, 1);
}

template <typename T>
void Uniform<T>::Set(unsigned int slot, std::vector<T>& value) {
    Set(slot, &value[0], (unsigned int)value.size());
}


#endif
