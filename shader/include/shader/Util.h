#ifndef SHADER_UTIL_H
#define SHADER_UTIL_H

#ifdef GLSL
#define CONST_SPEC_MATRIX(CONST_ID, NAME) \
	layout(constant_id = CONST_ID + 0) const float NAME##0 = 0.0; \
	layout(constant_id = CONST_ID + 1) const float NAME##1 = 0.0; \
	layout(constant_id = CONST_ID + 2) const float NAME##2 = 0.0; \
	layout(constant_id = CONST_ID + 3) const float NAME##3 = 0.0; \
	layout(constant_id = CONST_ID + 4) const float NAME##4 = 0.0; \
	layout(constant_id = CONST_ID + 5) const float NAME##5 = 0.0; \
	layout(constant_id = CONST_ID + 6) const float NAME##6 = 0.0; \
	layout(constant_id = CONST_ID + 7) const float NAME##7 = 0.0; \
	layout(constant_id = CONST_ID + 8) const float NAME##8 = 0.0; \
	layout(constant_id = CONST_ID + 9) const float NAME##9 = 0.0; \
	layout(constant_id = CONST_ID + 10) const float NAME##10 = 0.0; \
	layout(constant_id = CONST_ID + 11) const float NAME##11 = 0.0; \
	layout(constant_id = CONST_ID + 12) const float NAME##12 = 0.0; \
	layout(constant_id = CONST_ID + 13) const float NAME##13 = 0.0; \
	layout(constant_id = CONST_ID + 14) const float NAME##14 = 0.0; \
	layout(constant_id = CONST_ID + 15) const float NAME##15 = 0.0; \
	mat4 NAME = mat4(NAME##0, NAME##1, NAME##2, NAME##3, NAME##4, NAME##5, NAME##6, NAME##7, NAME##8, NAME##9, \
	                 NAME##10, NAME##11, NAME##12, NAME##13, NAME##14, NAME##15);
#endif

#endif
