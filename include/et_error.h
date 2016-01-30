#ifndef __ET_ERROR_H__
#define __ET_ERROR_H__

#include <stdio.h>

// Caution: depend gcc
#define et_critical(fmt, ...)  \
	fprintf(stderr, "critical: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define et_error(fmt, ...)  \
	fprintf(stderr, "error: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define et_warning(fmt, ...)  \
	fprintf(stderr, "warinig: %s()[%d]: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define et_debug(fmt, ...)  \
	fprintf(stdout, "debug: %s()[%d]: "fmt"", __func__, __LINE__, ## __VA_ARGS__)

#ifdef __ET_TEST__
#endif // __ET_TEST__

#endif // __ET_ERROR_H__

