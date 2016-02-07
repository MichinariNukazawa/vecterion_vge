#ifndef __PV_ERROR_H__
#define __PV_ERROR_H__

#include <stdio.h>

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
	fprintf(stdout, "debug: %s()[%d]: "fmt"", __func__, __LINE__, ## __VA_ARGS__)

#ifdef __PV_TEST__
#endif // __PV_TEST__

#endif // __PV_ERROR_H__

