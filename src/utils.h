#ifndef _utils_h
#define _utils_h

#include <stdio.h>
#include <stdlib.h>

#include "tvm.h"

#define array_lenght(x) (sizeof(x) / sizeof((x)[0]))

#define unwrap(x) \
	do { \
		int res = (x); \
		if (res != TVM_OK) \
			return res; \
	} while (0);

#endif
