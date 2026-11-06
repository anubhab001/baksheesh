#include <Arduino.h>

/*
#include <stdint.h>
#include <stdio.h>
#include <string.h>
*/
#define ENDIAN_H_

#define U32BIG(x)                                         \
    ((((x)&0x000000FF) << 24) | (((x)&0x0000FF00) << 8) | \
     (((x)&0x00FF0000) >> 8) | (((x)&0xFF000000) >> 24))

#define U8BIG(x, y)              \
    (x)[0] = (y) >> 24;          \
    (x)[1] = ((y) >> 16) & 0xff; \
    (x)[2] = ((y) >> 8) & 0xff;  \
    (x)[3] = (y)&0xff;

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

#define ROUND 16
#define NUMBER_OF_LOOP 32

#define Nb 16

#define DWT_CONTROL ((volatile u32*)0xE0001000)
#define DWT_CYCCNT ((volatile u32*)0xE0001004)
#define SCB_DEMCR ((volatile u32*)0xE000EDFC)

#define BLOCK_SIZE 16
#define KEY_SIZE 16

typedef unsigned char u8;
typedef unsigned int u32;

int baksheesh_encrypt_ecb(u8* ctext, const u8* ptext, u32 ptext_len,
                          const u32* key);
void setup();
void loop();

static inline u32 getDwtCyccnt(void);
static inline void resetDwtCyccnt(void);
static inline void enableDwtCyccnt(void);

u8 key[3][KEY_SIZE] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1st key
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0},
    {0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,  // 2nd key
     0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10},
    {0xd0, 0xf5, 0xc5, 0x9a, 0x77, 0x00, 0xd3, 0xe7,  // 3rd key
     0x99, 0x02, 0x8f, 0xa9, 0xf9, 0x0a, 0xd8, 0x37}};
u8 ptext[3][BLOCK_SIZE] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // 1st plaintext
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,  // 2nd plaintext
     0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10},
    {0xe3, 0x9c, 0x14, 0x1f, 0xa5, 0x7d, 0xba, 0x43,  // 3rd plaintext
     0xf0, 0x8a, 0x85, 0xb6, 0xa9, 0x1f, 0x86, 0xc1}};
u8 ctext[3][BLOCK_SIZE] = {
    {0xcd, 0x0b, 0xd7, 0x38, 0x38, 0x8a, 0xd3, 0xf6,  // 1st ciphertext
     0x68, 0xb1, 0x5a, 0x36, 0xce, 0xb6, 0xff, 0x92},
    {0x84, 0x22, 0x24, 0x1a, 0x6d, 0xbf, 0x5a, 0x93,  // 2nd ciphertext
     0x46, 0xaf, 0x46, 0x84, 0x09, 0xee, 0x01, 0x52},
    {0x13, 0xed, 0xe6, 0x7c, 0xbd, 0xcc, 0x3d, 0xbf,  // 3rd ciphertext
     0x40, 0x0a, 0x62, 0xd6, 0x97, 0x72, 0x65, 0xea}};

const u32 rconst[35 * 3] = {
    0x00000000, 0x00000000, 0x00000000, 0x00000040, 0x00000000, 0x00000000,
    0x00000000, 0x00001000, 0x00000000, 0x00200000, 0x00000000, 0x00000000,
    0x00000040, 0x00010000, 0x00000000, 0x00000000, 0x00001000, 0x00800000,
    0x00200040, 0x00000000, 0x00000000, 0x00000040, 0x00011000, 0x00000000,
    0x00200000, 0x00001000, 0x00800000, 0x00200000, 0x00010000, 0x00000000,
    0x00000040, 0x00010000, 0x00800000, 0x00000040, 0x00001000, 0x00800000,
    0x00200040, 0x00001000, 0x00000000, 0x00200040, 0x00011000, 0x00000000,
    0x00200040, 0x00011000, 0x00800000, 0x00200000, 0x00011000, 0x00800000,
    0x00200000, 0x00010000, 0x00800000, 0x00000000, 0x00010000, 0x00800000,
    0x00000040, 0x00000000, 0x00800000, 0x00000040, 0x00001000, 0x00000000,
    0x00200000, 0x00001000, 0x00000000, 0x00200040, 0x00010000, 0x00000000,
    0x00000040, 0x00011000, 0x00800000, 0x00200040, 0x00001000, 0x00800000,
    0x00200000, 0x00011000, 0x00000000, 0x00200040, 0x00010000, 0x00800000,
    0x00000000, 0x00011000, 0x00800000, 0x00200040, 0x00000000, 0x00800000,
    0x00000000, 0x00011000, 0x00000000, 0x00200000, 0x00000000, 0x00800000,
    0x00000000, 0x00010000, 0x00000000, 0x00000000, 0x00000000, 0x00800000};

#define U32BIG(x)                                         \
    ((((x)&0x000000FF) << 24) | (((x)&0x0000FF00) << 8) | \
     (((x)&0x00FF0000) >> 8) | (((x)&0xFF000000) >> 24))

#define U8BIG(x, y)              \
    (x)[0] = (y) >> 24;          \
    (x)[1] = ((y) >> 16) & 0xff; \
    (x)[2] = ((y) >> 8) & 0xff;  \
    (x)[3] = (y)&0xff;

#define SWAPMOVE(a, b, mask, n)          \
    ({                                   \
        u32 tmp = (b ^ (a >> n)) & mask; \
        b ^= tmp;                        \
        a ^= (tmp << n);                 \
    })

#define SBOX(s0, s1, s2, s3)  \
    {                         \
        u32 t0, t1, t2, t3;   \
        t0 = s0 | s2;         \
        t0 ^= s1;             \
        t2 = 0xffffffff ^ s2; \
        t1 = t2 | t0;         \
        t1 ^= s0;             \
        t0 ^= s3;             \
        t3 = s0 | s1;         \
        s1 = t3 ^ t1;         \
        s3 = t0 ^ t3;         \
        s0 = t1 ^ t2;         \
        s2 = s0 ^ t1;         \
    }                         \
    while (0)

#define QUINTUPLE_ROUND(state, rkey, rconst)          \
    ({                                                \
        SBOX(state[0], state[1], state[2], state[3]); \
        state[3] = NIBBLE_ROR_1(state[3]);            \
        state[1] = NIBBLE_ROR_2(state[1]);            \
        state[2] = NIBBLE_ROR_3(state[2]);            \
        state[0] ^= (rconst)[0];                      \
        state[0] ^= (rkey)[0];                        \
        state[1] ^= (rkey)[1];                        \
        state[2] ^= (rkey)[2];                        \
        state[3] ^= (rkey)[3];                        \
        SBOX(state[3], state[1], state[2], state[0]); \
        state[0] = HALF_ROR_4(state[0]);              \
        state[1] = HALF_ROR_8(state[1]);              \
        state[2] = HALF_ROR_12(state[2]);             \
        state[3] ^= (rconst)[1];                      \
        state[0] ^= (rkey)[4];                        \
        state[1] ^= (rkey)[5];                        \
        state[2] ^= (rkey)[6];                        \
        state[3] ^= (rkey)[7];                        \
        SBOX(state[0], state[1], state[2], state[3]); \
        state[3] = ROR(state[3], 16);                 \
        state[2] = ROR(state[2], 16);                 \
        SWAPMOVE(state[1], state[1], 0x55555555, 1);  \
        SWAPMOVE(state[2], state[2], 0x00005555, 1);  \
        SWAPMOVE(state[3], state[3], 0x55550000, 1);  \
        state[0] ^= (rconst)[2];                      \
        state[0] ^= (rkey)[8];                        \
        state[1] ^= (rkey)[9];                        \
        state[2] ^= (rkey)[10];                       \
        state[3] ^= (rkey)[11];                       \
        SBOX(state[3], state[1], state[2], state[0]); \
        state[0] = BYTE_ROR_6(state[0]);              \
        state[1] = BYTE_ROR_4(state[1]);              \
        state[2] = BYTE_ROR_2(state[2]);              \
        state[3] ^= (rconst)[3];                      \
        state[0] ^= (rkey)[12];                       \
        state[1] ^= (rkey)[13];                       \
        state[2] ^= (rkey)[14];                       \
        state[3] ^= (rkey)[15];                       \
        SBOX(state[0], state[1], state[2], state[3]); \
        state[3] = ROR(state[3], 24);                 \
        state[1] = ROR(state[1], 16);                 \
        state[2] = ROR(state[2], 8);                  \
        state[0] ^= (rconst)[4];                      \
        state[0] ^= (rkey)[16];                       \
        state[1] ^= (rkey)[17];                       \
        state[2] ^= (rkey)[18];                       \
        state[3] ^= (rkey)[19];                       \
        state[0] ^= state[3];                         \
        state[3] ^= state[0];                         \
        state[0] ^= state[3];                         \
    })

// interleave the nibble for fixsliced implementation

/*****************************************************************************
 * Rearranges the input in a row-wise bitsliced manner.
 *****************************************************************************/
void packing(u32* state, const u8* input) {
    u32 tmp;
    state[0] =
        (input[6] << 24) | (input[7] << 16) | (input[14] << 8) | input[15];
    state[1] =
        (input[4] << 24) | (input[5] << 16) | (input[12] << 8) | input[13];
    state[2] =
        (input[2] << 24) | (input[3] << 16) | (input[10] << 8) | input[11];
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

/*****************************************************************************
 * Fills the output from the internal state.
 *****************************************************************************/
void unpacking(u8* output, u32* state) {
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

/*****************************************************************************
 * Encryption of 128-bit blocks using GIFT-128 in ECB mode.
 * Note that 'ptext_len' must be a mutliple of 16.
 *****************************************************************************/
int baksheesh_encrypt_ecb(u8* ctext, const u8* ptext, u32 ptext_len,
                          const u32* key) {
    u32 tmp, state[4];  //, rkey[144];
    // precompute_rkeys(rkey, key);
    while (ptext_len > 0) {
        packing(state, ptext);
        //for (int i = 0; i < 35; i += 5)
        QUINTUPLE_ROUND(state, key + 0 * 4, rconst + 0);
        QUINTUPLE_ROUND(state, key + 5 * 4, rconst + 5);
        QUINTUPLE_ROUND(state, key + 10 * 4, rconst + 10);
        QUINTUPLE_ROUND(state, key + 15 * 4, rconst + 15);
        QUINTUPLE_ROUND(state, key + 20 * 4, rconst + 20);
        QUINTUPLE_ROUND(state, key + 25 * 4, rconst + 25);
        //QUINTUPLE_ROUND(state, key + 30 * 4, rconst + 30);
        unpacking(ctext, state);
        ptext += BLOCK_SIZE;
        ctext += BLOCK_SIZE;
        ptext_len -= BLOCK_SIZE;
    }
    return 0;
}

int main(void) {
    u32 start, elapsed;
    u32 total_cycle;

    u8 output[BLOCK_SIZE];

    u32 key[144] = {0};

    int i;

    setup();
    enableDwtCyccnt();
    resetDwtCyccnt();
#if 0
    printf("plaintext0: ");
    for (i = 0; i < Nb; i++) {
        printf("%02x", ptext[i]);
    }
    printf("\n");

    printf("plaintext1: ");
    for (i = 0; i < Nb; i++) {
        printf("%02x", ptext[i]);
    }
    printf("\n");
#endif
    // target function
    // start
    //Serial.printf("baksheesh:");
    start = getDwtCyccnt();
    //for (int i = 0; i < NUMBER_OF_LOOP; i++) {
        // CANDY(plainText0, plainText1, cipherText0, cipherText1, key);
    baksheesh_encrypt_ecb(output, ptext[0], BLOCK_SIZE, key);
   // }
    // measure elapsed
    elapsed     = getDwtCyccnt() - start;
    total_cycle = elapsed;
    //Serial.printf("cyc: ")
    //Serial.println(total_cycle);
     Serial.printf("cyc: %u\n", total_cycle);

    for (int i = 0; i < 16; i++) {
        output[i] = output[i] + 1;
        printf("%02x", output[i]);
    }
#if 0
    printf("ciphertext0: ");
    for (i = 0; i < Nb; i++) {
        printf("%02x", cipherText0[i]);
    }
    printf("\n");
    printf("ciphertext1: ");
    for (i = 0; i < Nb; i++) {
        printf("%02x", cipherText1[i]);
    }
    printf("\n");
#endif
    Serial.flush();

    for (;;) {
        loop();
    }
    return 0;
}

void setup() {
    // Serial.begin(19200);
    //Serial.begin(115200);
    // Sberial.begin(300);
   //  Serial.begin(57600);
    //Serial.begin(9600);
    Serial.begin(300);
    while (!Serial) {
    }
    // Serial.println("Start Serial Monitoring");
}

void loop() {}

static inline u32 getDwtCyccnt(void) { return *DWT_CYCCNT; }

static inline void resetDwtCyccnt(void) {
    *DWT_CYCCNT = 0;  // reset the counter
}

static inline void enableDwtCyccnt(void) {
    *SCB_DEMCR   = *SCB_DEMCR | (1u << 24);  // TRCENA = 1
    *DWT_CONTROL = *DWT_CONTROL | 1;  // enable the counter (CYCCNTENA = 1)
    *DWT_CYCCNT  = 0;                 // reset the counter
}
