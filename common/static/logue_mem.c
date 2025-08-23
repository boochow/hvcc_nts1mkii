#include "logue_mem.h"

#ifndef UNIT_HEAP_SIZE
#define UNIT_HEAP_SIZE (1024 * 3)
#endif

#ifndef UNIT_SDRAM_SIZE
#define UNIT_SDRAM_SIZE 10485760
#endif

static unsigned char heap[UNIT_HEAP_SIZE];
// sdram will be allocated in init_sdram()
static unsigned char *sdram = NULL;

#ifdef DEBUG
#include <stdio.h>
#define DLOG(...) printf(__VA_ARGS__)
// make these variables visible from testmem.c
size_t heap_offset = 0;
size_t sdram_offset = 0;
#else
#define DLOG(...) ((void) 0)
static size_t heap_offset = 0;
static size_t sdram_offset = 0;
#endif

void init_sdram(unsigned char* (*func)(unsigned int)) {
    if ((UNIT_SDRAM_SIZE) > 0) {
        sdram = (unsigned char *)func(UNIT_SDRAM_SIZE);
    }
    if (!sdram)
        sdram_offset = UNIT_SDRAM_SIZE;
}

#ifdef DEBUG
void *logue_malloc(size_t size, const char *caller) {
#else
void* logue_malloc(size_t size) {
#endif
    void *ptr = NULL;
#if SDRAM_ALLOC_THRESHOLD > 0
#ifdef DEBUG
    ptr = logue_sdram_alloc(size, __func__);
#else
    ptr = logue_sdram_alloc(size);
#endif
#endif
    if (ptr == NULL) {
#ifdef DEBUG
        ptr = logue_sram_alloc(size, __func__);
#else
        ptr = logue_sram_alloc(size);
#endif
    }
#ifdef DEBUG
    if (ptr == NULL) {
        DLOG("malloc: %ld : NG (%s)\n", size, caller);
    } else {
        DLOG("malloc: %ld : OK (%s)\n", size, caller);
    }
#endif
    return ptr;
}

#ifdef DEBUG
void *logue_sram_alloc(size_t size, const char *caller) {
#else
void* logue_sram_alloc(size_t size) {
#endif
    if (heap_offset + size > UNIT_HEAP_SIZE) {
        DLOG("sram_malloc: %ld : NG (%s)\n", size, caller);
        return NULL;
    }
    void* ptr = &heap[heap_offset];
    heap_offset += size;
    DLOG("sram_alloc: %ld : OK (%s)\n", size, caller);
    return ptr;
}

#ifdef DEBUG
void *logue_sdram_alloc(size_t size, const char *caller) {
#else
void* logue_sdram_alloc(size_t size) {
#endif
    if ((size >= SDRAM_ALLOC_THRESHOLD) && (sdram != NULL)) {
        if (sdram_offset + size > UNIT_SDRAM_SIZE) {
            DLOG("sdram_alloc: %ld : NG (%s)\n", size, caller);
            return NULL;
        }
        void* ptr = &sdram[sdram_offset];
        sdram_offset += size;
        DLOG("sdram_alloc: %ld : OK (%s)\n", size, caller);
        return ptr;
    } else {
        return NULL;
    }
}

#ifdef DEBUG
void *logue_realloc(void *ptr, size_t size, const char *caller) {
#else
void *logue_realloc(void *ptr, size_t size) {
#endif
#ifdef DEBUG
    logue_free(ptr, __func__);
    ptr = logue_malloc(size, __func__);
#else
    logue_free(ptr);
    ptr = logue_malloc(size);
#endif

    DLOG("realloc: %ld : %s (%s)\n", size, (ptr ? "OK" : "NG"), caller);

    return ptr;
}

#ifdef DEBUG
void logue_free(void *ptr, const char *caller) {
    (void) ptr;
    (void) caller;
#else
void logue_free(void *ptr) {
    (void) ptr;
#endif
    // do nothing
}
