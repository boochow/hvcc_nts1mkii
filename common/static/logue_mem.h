#ifndef __LOGUE_MEM__
#define __LOGUE_MEM__

#include <stddef.h>  // for size_t

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDRAM_ALLOC_THRESHOLD
#define SDRAM_ALLOC_THRESHOLD 256
#endif

#ifdef TESTMEM
extern void init_sdram(unsigned char* (*func)(unsigned int));
extern void *logue_malloc(size_t size, const char *caller);
extern void *logue_sram_alloc(size_t size, const char *caller);
extern void *logue_sdram_alloc(size_t size, const char *caller);
extern void *logue_realloc(void *ptr, size_t size, const char *caller);
extern void logue_free(void *ptr, const char *caller);
#else
extern void init_sdram(unsigned char* (*func)(unsigned int));
extern void *logue_malloc(size_t size);
extern void *logue_sram_alloc(size_t size);
extern void *logue_sdram_alloc(size_t size);
extern void *logue_realloc(void *ptr, size_t size);
extern void logue_free(void *ptr);
#endif
#ifdef __cplusplus
}
#endif

#endif
