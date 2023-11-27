#ifndef NYAS_LOG_H
#define NYAS_LOG_H

#include <stdio.h>
#include <assert.h>

#define NYAS_PRINT printf

#define NYAS__LOG_LOC NYAS_PRINT("%s(%u)", __FILE__, __LINE__)

#define NYAS_LOG(...)        \
	NYAS_PRINT("Nyas [Log] ");   \
	NYAS__LOG_LOC;           \
	NYAS_PRINT(":\n\t");     \
	NYAS_PRINT(__VA_ARGS__); \
	NYAS_PRINT("\n")

#define NYAS_LOG_WARN(...)     \
	NYAS_PRINT("Nyas [Warning] "); \
	NYAS__LOG_LOC;             \
	NYAS_PRINT(":\n\t");       \
	NYAS_PRINT(__VA_ARGS__);   \
	NYAS_PRINT("\n")

#define NYAS_LOG_ERR(...)    \
	NYAS_PRINT("Nyas [Error] "); \
	NYAS__LOG_LOC;           \
	NYAS_PRINT(":\n\t");     \
	NYAS_PRINT(__VA_ARGS__); \
	NYAS_PRINT("\n")

// Asserts
#define NYAS_ASSERT(X) (assert(X))

#endif // NYAS_LOG_H
