#ifndef NYAS_CORE_DEFS_H
#define NYAS_CORE_DEFS_H

#define NYAS_ASSERT(X) (assert(X))
#define NYAS_PRINT printf

#define NYAS__LOG_PREFIX(LL) NYAS_PRINT("Nyas [LL] ")
#define NYAS__LOG_LOC NYAS_PRINT("%s(%u)", __FILE__, __LINE__)

#define NYAS_LOG(...)        \
	NYAS__LOG_PREFIX(Log);   \
	NYAS__LOG_LOC;           \
	NYAS_PRINT(":\n\t");     \
	NYAS_PRINT(__VA_ARGS__); \
	NYAS_PRINT("\n")

#define NYAS_LOG_WARN(...)     \
	NYAS__LOG_PREFIX(Warning); \
	NYAS__LOG_LOC;             \
	NYAS_PRINT(":\n\t");       \
	NYAS_PRINT(__VA_ARGS__);   \
	NYAS_PRINT("\n")

#define NYAS_LOG_ERR(...)    \
	NYAS__LOG_PREFIX(Error); \
	NYAS__LOG_LOC;           \
	NYAS_PRINT(":\n\t");     \
	NYAS_PRINT(__VA_ARGS__); \
	NYAS_PRINT("\n")

#define NYAS_ALLOC nyas_alloc
#define NYAS_CALLOC nyas_calloc
#define NYAS_REALLOC nyas_realloc
#define NYAS_FREE nyas_free

#define NYAS_UNINIT (-5000)
#define NYAS_DELETED (-5001)
#define NYAS_MARKED_FOR_DELETE (-5002)
#define NYAS_INVALID (-5003)
#define NYAS_INACTIVE (-5004)
#define NYAS_DEFAULT (-5005)
#define NYAS_IGNORE (-5006)

#define NYAS_KB(X) ((X)*1024)
#define NYAS_MB(X) (NYAS_KB(X) * 1024)
#define NYAS_GB(X) (NYAS_MB((uint64_t)X) * 1024)

#define NYAS_BYTE_TO_KB(X) (X / 1024.0f)
#define NYAS_BYTE_TO_MB(X) (X / 1048576.0f)

enum nyas_errorcode {
	NYAS_EC_FAIL = 0,
	NYAS_EC_SUCCESS = 1,
	NYAS_EC_ALLOC = -100,
	NYAS_EC_FILE = -200,
};

#endif // NYAS_CORE_DEFS_H
