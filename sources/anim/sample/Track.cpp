#include "Track.h"

template Track<float, 1>;
template Track<vec3, 3>;
template Track<quat, 4>;


template<> float Track<float, 1>::Cast(float* value) {
	return value[0];
}

template<> vec3 Track<vec3, 3>::Cast(float* value) {
	return vec3(value[0], value[1], value[2]);
}

template<> quat Track<quat, 4>::Cast(float* value) {
	quat r = quat(value[0], value[1], value[2], value[3]);
	return normalized(r);
}
