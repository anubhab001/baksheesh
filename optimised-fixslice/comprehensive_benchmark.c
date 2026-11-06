/*
 * BAKSHEESH-GIFT fixslice throughput benchmark.
 */

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "baksheesh_gift_fixslice.h"

static inline uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

int main(void) {
    const int NUM_BLOCKS = 100000;
    const int BLOCK_SIZE = 16;
    const u32 total = (u32)NUM_BLOCKS * BLOCK_SIZE;

    u8* plaintext = malloc(total);
    u8* ciphertext = malloc(total);
    for (u32 i = 0; i < total; i++) plaintext[i] = (u8)(i & 0xff);

    u8 key[16];
    for (int i = 0; i < 16; i++) key[i] = (u8)(0x11 * i);

    u32 rkey[144];
    baksheesh_gift_keyschedule(key, rkey);

    baksheesh_gift_encrypt_ecb(ciphertext, plaintext, total, rkey);  // Warm up

    uint64_t start = get_time_ns();
    baksheesh_gift_encrypt_ecb(ciphertext, plaintext, total, rkey);
    uint64_t end = get_time_ns();

    double time_ms = (end - start) / 1000000.0;
    double mbps = (total * 8.0) / (time_ms * 1000.0);
    printf("BAKSHEESH-GIFT fixslice: %.2f MB in %.2f ms = %.2f Mbit/s\n",
           (double)total / (1024 * 1024), time_ms, mbps);

    free(plaintext);
    free(ciphertext);
    return 0;
}
