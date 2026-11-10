/****************************************************************************
* Portable C reference, used as the correctness baseline for the three
* assembly variants (compact/fast/masked) benchmarked alongside it.
****************************************************************************/
#include "fixslice_cipher.h"

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

#define SWAPMOVE(a, b, mask, n)          \
    ({                                   \
        u32 tmp = (b ^ (a >> n)) & mask; \
        b ^= tmp;                        \
        a ^= (tmp << n);                 \
    })

#define SBOX(s0, s1, s2, s3)      \
    {                              \
        u32 t;                    \
        t = ~(s2 & s1);           \
        s0 = s0 ^ t;               \
        s3 = s3 ^ s0;               \
        t = ~s0;                   \
        t = t & s2;                 \
        s1 = s1 ^ t;                 \
        t = s1 & s0;                  \
        s2 = s2 ^ t;                   \
        s1 = s1 ^ s3;                   \
        s0 = s0 ^ s1;                    \
        s2 = s2 ^ s0;                     \
    }

#define QUINTUPLE_ROUND(state, rkey)              \
    ({                                             \
        SBOX(state[0], state[1], state[2], state[3]); \
        state[1] = NIBBLE_ROR_1(state[1]);         \
        state[3] = NIBBLE_ROR_2(state[3]);         \
        state[0] = NIBBLE_ROR_3(state[0]);         \
        state[1] ^= (rkey)[0];                     \
        state[3] ^= (rkey)[1];                      \
        state[0] ^= (rkey)[2];                       \
        state[2] ^= (rkey)[3];                        \
        SBOX(state[1], state[3], state[0], state[2]);  \
        state[3] = HALF_ROR_4(state[3]);                 \
        state[2] = HALF_ROR_8(state[2]);                  \
        state[1] = HALF_ROR_12(state[1]);                  \
        state[3] ^= (rkey)[4];                              \
        state[2] ^= (rkey)[5];                               \
        state[1] ^= (rkey)[6];                                \
        state[0] ^= (rkey)[7];                                 \
        SBOX(state[3], state[2], state[1], state[0]);          \
        state[2] = ROR(state[2], 16);                            \
        state[3] = ROR(state[3], 16);                              \
        SWAPMOVE(state[0], state[0], 0x55555555, 1);                \
        SWAPMOVE(state[3], state[3], 0x00005555, 1);                 \
        SWAPMOVE(state[2], state[2], 0x55550000, 1);                  \
        state[2] ^= (rkey)[8];                                         \
        state[0] ^= (rkey)[9];                                          \
        state[3] ^= (rkey)[10];                                          \
        state[1] ^= (rkey)[11];                                           \
        SBOX(state[2], state[0], state[3], state[1]);                      \
        state[0] = BYTE_ROR_6(state[0]);                                    \
        state[1] = BYTE_ROR_4(state[1]);                                     \
        state[2] = BYTE_ROR_2(state[2]);                                      \
        state[0] ^= (rkey)[12];                                               \
        state[1] ^= (rkey)[13];                                                \
        state[2] ^= (rkey)[14];                                                 \
        state[3] ^= (rkey)[15];                                                  \
        SBOX(state[0], state[1], state[2], state[3]);                            \
        state[1] = ROR(state[1], 24);                                             \
        state[3] = ROR(state[3], 16);                                              \
        state[0] = ROR(state[0], 8);                                               \
        state[1] ^= (rkey)[16];                                                     \
        state[3] ^= (rkey)[17];                                                      \
        state[0] ^= (rkey)[18];                                                       \
        state[2] ^= (rkey)[19];                                                        \
    })

static void packing(u32* state, const u8* input) {
    state[0] = (input[6] << 24) | (input[7] << 16) | (input[14] << 8) | input[15];
    state[1] = (input[4] << 24) | (input[5] << 16) | (input[12] << 8) | input[13];
    state[2] = (input[2] << 24) | (input[3] << 16) | (input[10] << 8) | input[11];
    state[3] = (input[0] << 24) | (input[1] << 16) | (input[8] << 8) | input[9];
    SWAPMOVE(state[0], state[0], 0x0a0a0a0a, 3);
    SWAPMOVE(state[0], state[0], 0x00cc00cc, 6);
    SWAPMOVE(state[1], state[1], 0x0a0a0a0a, 3);
    SWAPMOVE(state[1], state[1], 0x00cc00cc, 6);
    SWAPMOVE(state[2], state[2], 0x0a0a0a0a, 3);
    SWAPMOVE(state[2], state[2], 0x00cc00cc, 6);
    SWAPMOVE(state[3], state[3], 0x0a0a0a0a, 3);
    SWAPMOVE(state[3], state[3], 0x00cc00cc, 6);
    SWAPMOVE(state[0], state[1], 0x000f000f, 4);
    SWAPMOVE(state[0], state[2], 0x000f000f, 8);
    SWAPMOVE(state[0], state[3], 0x000f000f, 12);
    SWAPMOVE(state[1], state[2], 0x00f000f0, 4);
    SWAPMOVE(state[1], state[3], 0x00f000f0, 8);
    SWAPMOVE(state[2], state[3], 0x0f000f00, 4);
}

static void unpacking(u8* output, u32* state) {
    SWAPMOVE(state[2], state[3], 0x0f000f00, 4);
    SWAPMOVE(state[1], state[3], 0x00f000f0, 8);
    SWAPMOVE(state[1], state[2], 0x00f000f0, 4);
    SWAPMOVE(state[0], state[3], 0x000f000f, 12);
    SWAPMOVE(state[0], state[2], 0x000f000f, 8);
    SWAPMOVE(state[0], state[1], 0x000f000f, 4);
    SWAPMOVE(state[3], state[3], 0x00cc00cc, 6);
    SWAPMOVE(state[3], state[3], 0x0a0a0a0a, 3);
    SWAPMOVE(state[2], state[2], 0x00cc00cc, 6);
    SWAPMOVE(state[2], state[2], 0x0a0a0a0a, 3);
    SWAPMOVE(state[1], state[1], 0x00cc00cc, 6);
    SWAPMOVE(state[1], state[1], 0x0a0a0a0a, 3);
    SWAPMOVE(state[0], state[0], 0x00cc00cc, 6);
    SWAPMOVE(state[0], state[0], 0x0a0a0a0a, 3);
    output[0]  = state[3] >> 24;
    output[1]  = (state[3] >> 16) & 0xff;
    output[2]  = state[2] >> 24;
    output[3]  = (state[2] >> 16) & 0xff;
    output[4]  = state[1] >> 24;
    output[5]  = (state[1] >> 16) & 0xff;
    output[6]  = state[0] >> 24;
    output[7]  = (state[0] >> 16) & 0xff;
    output[8]  = (state[3] >> 8) & 0xff;
    output[9]  = state[3] & 0xff;
    output[10] = (state[2] >> 8) & 0xff;
    output[11] = state[2] & 0xff;
    output[12] = (state[1] >> 8) & 0xff;
    output[13] = state[1] & 0xff;
    output[14] = (state[0] >> 8) & 0xff;
    output[15] = state[0] & 0xff;
}

void encrypt_block_ref(u8* out, const u32* rkey, const u8* block) {
    u32 state[4];
    packing(state, block);
    for (int i = 0; i < 35; i += 5) {
        QUINTUPLE_ROUND(state, rkey + i * 4);
    }
    unpacking(out, state);
}
