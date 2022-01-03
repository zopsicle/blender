#pragma once

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ImBuf;

bool imb_is_a_krita(const unsigned char *mem, const size_t size);

struct ImBuf *imb_load_krita(const unsigned char *mem, size_t size, int flags, char *colorspace);

#ifdef __cplusplus
}
#endif
