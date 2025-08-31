#ifndef __LOGUE_MEM_HV_SRAM__
#define __LOGUE_MEM_HV_SRAM__

#ifdef __cplusplus
extern "C" {
#endif

#undef hv_malloc
#ifdef TESTMEM
#define hv_malloc(_n)  logue_sram_alloc(_n, __func__)
#else
#define hv_malloc(_n)  logue_sram_alloc(_n)
#endif

#ifdef __cplusplus
}
#endif

#endif

