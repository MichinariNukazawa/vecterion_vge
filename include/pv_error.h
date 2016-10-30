#ifndef include_PV_ERROR_H
#define include_PV_ERROR_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define pv_assert(hr) \
	do{ \
		if(!(hr)){ \
			fprintf(stderr, "pv assert: %s()[%d]:'%s'\n", __func__, __LINE__, #hr); \
			assert(hr); \
		} \
	}while(0);

#define pv_assertf(hr, fmt, ...) \
	do{ \
		if(!(hr)){ \
			fprintf(stderr, "pv_assertf: %s()[%d]: "fmt"\n", \
					__func__, __LINE__, ## __VA_ARGS__); \
			assert(hr); \
		} \
	}while(0);

// CAUTION: depend gcc
#define pv_bug(fmt, ...)  \
	fprintf(stderr, "BUG: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define pv_fixme(fmt, ...)  \
	fprintf(stderr, "FIXME: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)

#define pv_critical(fmt, ...)  \
	fprintf(stderr, "critical: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define pv_error(fmt, ...)  \
	fprintf(stderr, "error: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define pv_warning(fmt, ...)  \
	fprintf(stderr, "warning: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define pv_debug(fmt, ...)  \
	fprintf(stdout, "debug: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)

#ifdef include_PV_TEST
#endif // include_PV_TEST

#endif // include_PV_ERROR_H

