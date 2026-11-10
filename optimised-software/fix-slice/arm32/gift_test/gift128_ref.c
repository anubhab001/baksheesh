/****************************************************************************
* Portable C reference for the GIFT-128 heritage of the cipher, used as the
* correctness baseline for the compact and masked ARM assembly variants
* benchmarked alongside it.
****************************************************************************/
#include "gift128.h"

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

#define SWAPMOVE(a, b, mask, n)      \
    tmp = (b ^ (a >> n)) & mask;     \
    b ^= tmp;                        \
    a ^= (tmp << n);

#define SBOX(s0, s1, s2, s3) \
    s1 ^= s0 & s2;           \
    s0 ^= s1 & s3;           \
    s2 ^= s0 | s1;           \
    s3 ^= s2;                \
    s1 ^= s3;                \
    s3 ^= 0xffffffff;        \
    s2 ^= s0 & s1;

#define QUINTUPLE_ROUND(state, rkey, rconst) ({           \
    SBOX(state[0], state[1], state[2], state[3]);         \
    state[3] = NIBBLE_ROR_1(state[3]);                     \
    state[1] = NIBBLE_ROR_2(state[1]);                     \
    state[2] = NIBBLE_ROR_3(state[2]);                     \
    state[0] ^= (rkey)[0]; state[1] ^= (rkey)[1];          \
    state[2] ^= (rkey)[2]; state[3] ^= (rkey)[3];          \
    state[0] ^= (rconst)[0];                               \
    SBOX(state[3], state[1], state[2], state[0]);          \
    state[0] = HALF_ROR_4(state[0]);                       \
    state[1] = HALF_ROR_8(state[1]);                       \
    state[2] = HALF_ROR_12(state[2]);                      \
    state[0] ^= (rkey)[4]; state[1] ^= (rkey)[5];          \
    state[2] ^= (rkey)[6]; state[3] ^= (rkey)[7];          \
    state[3] ^= (rconst)[1];                               \
    SBOX(state[0], state[1], state[2], state[3]);          \
    state[3] = ROR(state[3], 16);                          \
    state[2] = ROR(state[2], 16);                          \
    SWAPMOVE(state[1], state[1], 0x55555555, 1);           \
    SWAPMOVE(state[2], state[2], 0x00005555, 1);           \
    SWAPMOVE(state[3], state[3], 0x55550000, 1);           \
    state[0] ^= (rkey)[8]; state[1] ^= (rkey)[9];          \
    state[2] ^= (rkey)[10]; state[3] ^= (rkey)[11];        \
    state[0] ^= (rconst)[2];                               \
    SBOX(state[3], state[1], state[2], state[0]);          \
    state[0] = BYTE_ROR_6(state[0]);                       \
    state[1] = BYTE_ROR_4(state[1]);                       \
    state[2] = BYTE_ROR_2(state[2]);                       \
    state[0] ^= (rkey)[12]; state[1] ^= (rkey)[13];        \
    state[2] ^= (rkey)[14]; state[3] ^= (rkey)[15];        \
    state[3] ^= (rconst)[3];                               \
    SBOX(state[0], state[1], state[2], state[3]);          \
    state[3] = ROR(state[3], 24);                          \
    state[1] = ROR(state[1], 16);                          \
    state[2] = ROR(state[2], 8);                           \
    state[0] ^= (rkey)[16]; state[1] ^= (rkey)[17];        \
    state[2] ^= (rkey)[18]; state[3] ^= (rkey)[19];        \
    state[0] ^= (rconst)[4];                               \
    state[0] ^= state[3]; state[3] ^= state[0]; state[0] ^= state[3]; \
})

static const u32 rconst[40] = {
    0x10000008, 0x80018000, 0x54000002, 0x01010181,
    0x8000001f, 0x10888880, 0x6001e000, 0x51500002,
    0x03030180, 0x8000002f, 0x10088880, 0x60016000,
    0x41500002, 0x03030080, 0x80000027, 0x10008880,
    0x4001e000, 0x11500002, 0x03020180, 0x8000002b,
    0x10080880, 0x60014000, 0x01400002, 0x02020080,
    0x80000021, 0x10000080, 0x0001c000, 0x51000002,
    0x03010180, 0x8000002e, 0x10088800, 0x60012000,
    0x40500002, 0x01030080, 0x80000006, 0x10008808,
    0xc001a000, 0x14500002, 0x01020181, 0x8000001a};

static void packing(u32* state, const u8* input) {
    state[0] = (input[6] << 24) | (input[7] << 16) | (input[14] << 8) | input[15];
    state[1] = (input[4] << 24) | (input[5] << 16) | (input[12] << 8) | input[13];
    state[2] = (input[2] << 24) | (input[3] << 16) | (input[10] << 8) | input[11];
    state[3] = (input[0] << 24) | (input[1] << 16) | (input[8] << 8) | input[9];
    u32 tmp;
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
    u32 tmp;
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
    output[0]  = state[3] >> 24; output[1]  = (state[3] >> 16) & 0xff;
    output[2]  = state[2] >> 24; output[3]  = (state[2] >> 16) & 0xff;
    output[4]  = state[1] >> 24; output[5]  = (state[1] >> 16) & 0xff;
    output[6]  = state[0] >> 24; output[7]  = (state[0] >> 16) & 0xff;
    output[8]  = (state[3] >> 8) & 0xff; output[9]  = state[3] & 0xff;
    output[10] = (state[2] >> 8) & 0xff; output[11] = state[2] & 0xff;
    output[12] = (state[1] >> 8) & 0xff; output[13] = state[1] & 0xff;
    output[14] = (state[0] >> 8) & 0xff; output[15] = state[0] & 0xff;
}

void gift128_encrypt_block_ref(u8* out, const u32* rkey, const u8* block) {
    u32 state[4];
    packing(state, block);
    state[0] ^= rkey[0]; state[1] ^= rkey[1];
    state[2] ^= rkey[2]; state[3] ^= rkey[3];
    for (int i = 0; i < 40; i += 5)
        QUINTUPLE_ROUND(state, rkey + (i + 1) * 4, rconst + i);
    unpacking(out, state);
}
