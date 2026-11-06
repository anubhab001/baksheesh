/*
 * BAKSHEESH-GIFT Heritage Fixslice Implementation
 * 
 * Optimized fixslice implementation using correct BAKSHEESH S-box from existing implementation
 * This uses the GIFT heritage S-box structure (4-bit) with BAKSHEESH specifications
 */

#ifndef BAKSHEESH_GIFT_FIXSLICE_H
#define BAKSHEESH_GIFT_FIXSLICE_H

#include <stdint.h>

typedef uint32_t u32;
typedef uint8_t u8;

// Bit manipulation macros from existing BAKSHEESH fixslice implementation
#define ROR(x, y) (((x) >> (y)) | ((x) << (32 - (y))))
#define BYTE_ROR_2(x) ((((x) >> 2) & 0x3f3f3f3f) | (((x)&0x03030303) << 6))
#define BYTE_ROR_4(x) ((((x) >> 4) & 0x0f0f0f0f) | (((x)&0x0f0f0f0f) << 4))
#define BYTE_ROR_6(x) ((((x) >> 6) & 0x03030303) | (((x)&0x3f3f3f3f) << 2))
#define HALF_ROR_4(x) ((((x) >> 4) & 0x0fff0fff) | (((x)&0x000f000f) << 12))
#define HALF_ROR_8(x) ((((x) >> 8) & 0x00ff00ff) | (((x)&0x00ff00ff) << 8))
#define HALF_ROR_12(x) ((((x) >> 12) & 0x000f000f) | (((x)&0x0fff0fff) << 4))
#define NIBBLE_ROR_1(x) ((((x) >> 1) & 0x77777777) | (((x)&0x11111111) << 3))
#define NIBBLE_ROR_2(x) ((((x) >> 2) & 0x33333333) | (((x)&0x33333333) << 2))
#define NIBBLE_ROR_3(x) ((((x) >> 3) & 0x11111111) | (((x)&0x77777777) << 1))

#define SWAPMOVE(a, b, mask, n) do { \
    u32 tmp = (b ^ (a >> n)) & mask; \
    b ^= tmp; \
    a ^= (tmp << n); \
} while(0)

// BAKSHEESH S-box (4-bit) from the existing fixslice implementation
// This is the correct S-box for BAKSHEESH (not GIFT's S-box)
#define BAKSHEESH_SBOX(s0, s1, s2, s3) do { \
    u32 t; \
    t = s1 | s0; \
    s2 ^= t; \
    t = s0 | (~s2); \
    s1 ^= t; \
    t = s1 & s2; \
    s0 ^= t; \
    s1 ^= s3; \
    s2 ^= s1; \
    s3 ^= s0; \
    s0 ^= s2; \
    s1 = ~s1; \
} while(0)

// BAKSHEESH-GIFT quintuple round - exact copy from working implementation
#define BAKSHEESH_GIFT_QUINTUPLE_ROUND(s0, s1, s2, s3, rkey) do { \
    BAKSHEESH_SBOX(s0, s1, s2, s3); \
    s2 = NIBBLE_ROR_1(s2); \
    s0 = NIBBLE_ROR_2(s0); \
    s3 = NIBBLE_ROR_3(s3); \
    s2 ^= (rkey)[0]; \
    s0 ^= (rkey)[1]; \
    s3 ^= (rkey)[2]; \
    s1 ^= (rkey)[3]; \
    BAKSHEESH_SBOX(s2, s0, s3, s1); \
    s3 = HALF_ROR_4(s3); \
    s2 = HALF_ROR_8(s2); \
    s1 = HALF_ROR_12(s1); \
    s3 ^= (rkey)[4]; \
    s2 ^= (rkey)[5]; \
    s1 ^= (rkey)[6]; \
    s0 ^= (rkey)[7]; \
    BAKSHEESH_SBOX(s3, s2, s1, s0); \
    s1 = ROR(s1, 16); \
    s0 = ROR(s0, 16); \
    SWAPMOVE(s3, s3, 0x55555555, 1); \
    SWAPMOVE(s0, s0, 0x00005555, 1); \
    SWAPMOVE(s1, s1, 0x55550000, 1); \
    s1 ^= (rkey)[8]; \
    s3 ^= (rkey)[9]; \
    s0 ^= (rkey)[10]; \
    s2 ^= (rkey)[11]; \
    BAKSHEESH_SBOX(s1, s3, s0, s2); \
    s0 = BYTE_ROR_6(s0); \
    s1 = BYTE_ROR_4(s1); \
    s2 = BYTE_ROR_2(s2); \
    s0 ^= (rkey)[12]; \
    s1 ^= (rkey)[13]; \
    s2 ^= (rkey)[14]; \
    s3 ^= (rkey)[15]; \
    BAKSHEESH_SBOX(s0, s1, s2, s3); \
    s2 = ROR(s2, 24); \
    s0 = ROR(s0, 16); \
    s3 = ROR(s3, 8); \
    s2 ^= (rkey)[16]; \
    s0 ^= (rkey)[17]; \
    s3 ^= (rkey)[18]; \
    s1 ^= (rkey)[19]; \
} while(0)

// Function declarations
void baksheesh_gift_packing(u32* state, const u8* input);
void baksheesh_gift_unpacking(u8* output, u32* state);
void baksheesh_gift_keyschedule(const u8* key, u32* rkey);
int baksheesh_gift_encrypt_ecb(u8* ctext, const u8* ptext, u32 ptext_len, const u32* rkey);

// Round constants for BAKSHEESH
extern int baksheesh_rconst[35];

#endif