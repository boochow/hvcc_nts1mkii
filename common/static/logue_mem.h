#ifndef __LOGUE_MEM__
#define __LOGUE_MEM__

#include <stddef.h>  // for size_t

#ifdef __cplusplus
extern "C" {
#endif

#define SDMALLOC_THRESHOLD 1536
extern void init_sdram(unsigned char* (*func)(unsigned int));
extern void *logue_malloc(size_t size);
extern void logue_free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif
