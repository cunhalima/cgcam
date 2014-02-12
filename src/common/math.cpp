#include <cassert>
#include "../defs.h"

void M_VectorCopy(vec3_t dst, const vec3_t src) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
}