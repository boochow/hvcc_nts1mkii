#include "logue_mem.h"
#ifdef DEBUG
#include <stdio.h>
#endif

#ifndef UNIT_HEAP_SIZE
#define UNIT_HEAP_SIZE (1024 * 3)
#endif

#ifndef UNIT_SDRAM_SIZE
#define UNIT_SDRAM_SIZE (1024 * 1000)
#endif

static unsigned char heap[UNIT_HEAP_SIZE];
#ifndef DEBUG
static unsigned char *sdram = NULL;
static size_t heap_offset = 0;
static size_t sdram_offset = 0;
#else
static unsigned char sdram[UNIT_SDRAM_SIZE];
size_t heap_offset = 0;
size_t sdram_offset = 0;
#endif

void init_sdram(unsigned char* (*func)(unsigned int)) {
#ifndef DEBUG
    sdram = (unsigned char *)func(UNIT_SDRAM_SIZE);
    if (!sdram)
        sdram_offset = UNIT_SDRAM_SIZE;
#endif
}

void* logue_malloc(size_t size) {
    if ((size > SDMALLOC_THRESHOLD) && (sdram != NULL)) {
        if (sdram_offset + size > UNIT_SDRAM_SIZE) {
#ifdef DEBUG
            printf("sdram_alloc: %ld : NG\n", size);
#endif
            return NULL;
        }
        void* ptr = &sdram[sdram_offset];
        sdram_offset += size;
#ifdef DEBUG
        printf("sdram_alloc: %ld : OK\n", size);
#endif
        return ptr;
    } else {
        if (heap_offset + size > UNIT_HEAP_SIZE) {
#ifdef DEBUG
            printf("malloc: %ld : NG\n", size);
#endif
            return NULL;
        }
        void* ptr = &heap[heap_offset];
        heap_offset += size;
#ifdef DEBUG
        printf("malloc: %ld : OK\n", size);
#endif
        return ptr;
    }
}

void logue_free(void *ptr) {
    // do nothing
}
