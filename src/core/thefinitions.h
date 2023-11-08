#ifndef THE_THEFINITIONS_H
#define THE_THEFINITIONS_H

#define THE_ASSERT(X, TEXT) (assert(X))
#define THE_LOG_ERROR(FMT, ...)                                               \
	printf("ERROR @ %s(%u): \n\t" FMT "\n", __FILE__, __LINE__, __VA_ARGS__)
#define THE_SLOG_ERROR(FMT)                                                   \
	printf("ERROR @ %s(%u): \n\t" FMT "\n", __FILE__, __LINE__)
#define THE_LOG_WARNING(FMT, ...)                                             \
	printf("WARNING @ %s(%u): \n\t" FMT "\n", __FILE__, __LINE__, __VA_ARGS__)
#define THE_SLOG_WARNING(FMT)                                                 \
	printf("WARNING @ %s(%u): \n\t" FMT "\n", __FILE__, __LINE__)
#define THE_LOG(FMT, ...) printf(FMT "\n", __VA_ARGS__)
#define THE_SLOG(FMT) printf(FMT "\n")

#define NYAS_LOG(...)                                                         \
	printf("Nyas @ %s(%u): \n\t", __FILE__, __LINE__);                        \
	printf(__VA_ARGS__);                                                      \
	printf("\n")

#define THE_UNINIT (-5000)
#define THE_DELETED (-5001)
#define THE_MARKED_FOR_DELETE (-5002)
#define THE_INVALID (-5003)
#define THE_INACTIVE (-5004)
#define THE_DEFAULT (-5005)
#define THE_IGNORE (-5006)

#define THE_KB(X) ((X)*1024)
#define THE_MB(X) (THE_KB(X) * 1024)
#define THE_GB(X) (THE_MB((uint64_t)X) * 1024)

#define THE_BYTE_TO_KB(X) (X / 1024.0f)
#define THE_BYTE_TO_MB(X) (X / 1048576.0f)

enum THE_ErrorCode {
	THE_EC_FAIL = 0,
	THE_EC_SUCCESS = 1,
	THE_EC_ALLOC = -100,
	THE_EC_FILE = -200,
};

#endif
