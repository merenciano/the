#ifndef NYAS_UTILS_STRING_H
#define NYAS_UTILS_STRING_H

#include <string.h>

typedef struct {
	const char *str;
	size_t len;
} nyas_stref;

#define NYAS_litref(LIT)                                                      \
	{                                                                         \
		.str = &LIT[0], .len = (sizeof LIT) - 1                               \
	}

#define NYAS_FXD_STR(SZ)                                                      \
	struct {                                                                  \
		char str[SZ - 1];                                                     \
		unsigned char rem;                                                    \
	}

#define NYAS__STR_FUNCTIONS(SZ) NYAS___STR_FUNCTIONS(SZ)
#define NYAS___STR_FUNCTIONS(SZ)                                              \
	typedef NYAS_FXD_STR(SZ) nyas_str##SZ;                                    \
                                                                              \
	nyas_str##SZ nyas_str##SZ##_from_ref(nyas_stref sref)                     \
	{                                                                         \
		assert(sref.len < SZ);                                                \
		nyas_str##SZ str;                                                     \
		memcpy((char *)str.str, sref.str, sref.len);                          \
		str.rem = SZ - sref.len;                                              \
		str.str[sref.len] = '\0';                                             \
		return str;                                                           \
	}                                                                         \
                                                                              \
	static nyas_stref nyas_str##SZ##_to_ref(nyas_str##SZ *str)                \
	{                                                                         \
		nyas_stref ref = { .str = str->str, .len = SZ - str->rem };           \
		return ref;                                                           \
	}                                                                         \
                                                                              \
	void nyas_str##SZ##_cpy(nyas_str##SZ *str, nyas_str##SZ *other)           \
	{                                                                         \
		unsigned char r = other->rem;                                         \
		memcpy((char *)str->str, (char *)other->str, SZ - r);                 \
		str->rem = r;                                                         \
	}                                                                         \
	int nyas__

#define NYASTR__TYPE(NYASTR_SIZE) nyas_str##NYASTR_SIZE
#define NYASTR_TYPE(NYASTR_SIZE) NYASTR__TYPE(NYASTR_SIZE)

#define NYASTR_SIZE 64

NYAS__STR_FUNCTIONS(NYASTR_SIZE);

void
A(void)
{
	nyas_str64 a, b;
	nyas_str64_cpy(&a, &b);

	nyas_stref ref = nyas_litref("Hola jejeje");
}

#endif // NYAS_UTILS_STRING_H
