#ifndef __LOGUE_MEM_HV__
#define __LOGUE_MEM_HV__

#include "logue_mem.h"

#ifdef __cplusplus
extern "C" {
#endif

#undef hv_malloc
#undef hv_realloc
#undef hv_free
#ifdef DEBUG
#define hv_malloc(_n)  logue_malloc(_n, __func__)
#define hv_realloc(a, b) logue_realloc(a, b, __func__)
#define hv_free(x)  logue_free(x, __func__)
#else
#define hv_malloc(_n)  logue_malloc(_n)
#define hv_realloc(a, b) logue_realloc(a, b)
#define hv_free(x)  logue_free(x)
#endif

#ifdef __cplusplus
}
#endif

#endif
