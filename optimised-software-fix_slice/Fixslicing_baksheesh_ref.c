/* THIS IS KEPT AS-IS FROM ORIGINAL FIX-SLICE PROJECT (ON GIFT)
 * This implementation is based on the Fixslicing method described in the paper:
 * "Fixslicing: A New GIFT Representation" by Aadil A. and Thomas Peyrin.
 *
 * The paper can be found at: https://eprint.iacr.org/2020/412.pdf
 *
 * The code repository for the reference implementation can be found at:
 * https://github.com/aadomn/gift
 *
 * The encryption functions have been adapted based on the
 * techniques outlined in the paper and its reference implementation.
 *
 * Note: Further optimization for performance may be required for key schedule.
 */

#include <stdio.h>
#include <string.h>


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
#define NUMBER_OF_LOOP 35

#define Nb 16

#define BLOCK_SIZE 16
#define KEY_SIZE 16

#define NUM_TEST_VECTORS 8

typedef unsigned char u8;
typedef unsigned int u32;

int RoundConstant[35] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};

int baksheesh_encrypt_ecb(u8* ctext, const u8* ptext, u32 ptext_len,
                          const u32* key);

// These arrays are stored in big-endian format, meaning the most significant byte is stored first.
uint8_t key[NUM_TEST_VECTORS][BLOCK_SIZE] = {
    // Key 1
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // Key 2
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // Key 3
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // Key 4
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // Key 5
    { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
    // Key 6
    { 0x76, 0x54, 0x32, 0x10, 0x03, 0x20, 0x32, 0x03,
      0x20, 0x32, 0x03, 0x20, 0x32, 0x03, 0x20, 0x32 },
    // Key 7
    { 0x23, 0x02, 0x30, 0x23, 0x02, 0x30, 0x23, 0x02,
      0x30, 0x23, 0x02, 0x30, 0x01, 0x23, 0x45, 0x67 },
    // Key 8
    { 0x59, 0x20, 0xef, 0xfb, 0x52, 0xbc, 0x61, 0xe3,
      0x3a, 0x98, 0x42, 0x53, 0x21, 0xe7, 0x69, 0x15 }
};

uint8_t plaintext[NUM_TEST_VECTORS][BLOCK_SIZE] = {
    // Plaintext 1
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // Plaintext 2
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07 },
    // Plaintext 3
    { 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    // Plaintext 4
    { 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
      0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44 },
    // Plaintext 5
    { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
      0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 },
    // Plaintext 6
    { 0x78, 0x9a, 0x78, 0x9a, 0x78, 0x9a, 0x78, 0x9a,
      0x78, 0x9a, 0x78, 0x9a, 0x78, 0x9a, 0x78, 0x9a },
    // Plaintext 7
    { 0xb6, 0xe4, 0x78, 0x9a, 0xb6, 0xe4, 0x78, 0x9a,
      0xb6, 0xe4, 0x78, 0x9a, 0xb6, 0xe4, 0x78, 0x9a },
    // Plaintext 8
    { 0xe6, 0x51, 0x75, 0x31, 0xab, 0xf6, 0x3f, 0x3d,
      0x78, 0x05, 0xe1, 0x26, 0x94, 0x3a, 0x08, 0x1c }
};

uint8_t ciphertext[NUM_TEST_VECTORS][BLOCK_SIZE] = {
    // Ciphertext 1
    { 0xc0, 0x02, 0xbe, 0x5e, 0x64, 0xc7, 0x8a, 0x72,
      0xab, 0x9a, 0x34, 0x39, 0x51, 0x83, 0x52, 0xaa },
    // Ciphertext 2
    { 0x6f, 0x7d, 0x77, 0x46, 0xea, 0xf0, 0xd9, 0x7a,
      0x15, 0x40, 0x79, 0xf6, 0xbd, 0x84, 0x64, 0x38 },
    // Ciphertext 3
    { 0x1b, 0xa3, 0x36, 0x37, 0x34, 0xc0, 0x9a, 0x29,
      0xf6, 0x7c, 0x23, 0xbb, 0xb2, 0xcc, 0xcc, 0x05 },
    // Ciphertext 4
    { 0x7a, 0xd3, 0x30, 0x36, 0x67, 0xb2, 0xaf, 0x6d,
      0xee, 0xf4, 0x34, 0xdd, 0x11, 0x0d, 0x7f, 0xb8 },
    // Ciphertext 5
    { 0x80, 0x6f, 0x0c, 0xf4, 0x5b, 0x94, 0xf0, 0x37,
      0x02, 0x06, 0x97, 0x5f, 0xe7, 0x8a, 0xc1, 0x0f },
    // Ciphertext 6
    { 0xae, 0x65, 0x4b, 0x53, 0x33, 0xb8, 0x76, 0x58,
      0x4f, 0x8e, 0x8d, 0xd5, 0x4f, 0x4e, 0x49, 0x0a },
    // Ciphertext 7
    { 0x3d, 0xbb, 0xdf, 0x7f, 0xe2, 0x54, 0xcc, 0x0b,
      0xe3, 0x96, 0xa7, 0x53, 0x44, 0x2d, 0xcc, 0xad },
    // Ciphertext 8
    { 0xfc, 0x7e, 0x61, 0xfe, 0xe3, 0xd5, 0x87, 0x30,
      0x8c, 0xa7, 0xbc, 0x59, 0x4e, 0xbf, 0x32, 0x44 }
};

#define SWAPMOVE(a, b, mask, n)          \
({                                   \
u32 tmp = (b ^ (a >> n)) & mask; \
b ^= tmp;                        \
a ^= (tmp << n);                 \
})

#define SBOX(s0, s1, s2, s3)  \
{                         \
u32 t;\
t = s1 | s0;\
s2 ^= t;\
t = s0 | (~s2);\
s1 ^= t;\
t = s1 & s2;\
s0 ^=t;\
s1 ^=s3;\
s2 ^= s1;\
s3 ^= s0;\
s0 ^= s2;\
s1 = ~s1;\
}\

#define QUINTUPLE_ROUND(s0, s1, s2, s3, rkey)          \
({                                                \
SBOX(s0, s1, s2, s3); \
s2 = NIBBLE_ROR_1(s2);            \
s0 = NIBBLE_ROR_2(s0);            \
s3 = NIBBLE_ROR_3(s3);            \
s2 ^= (rkey)[0];\
s0 ^= (rkey)[1];\
s3 ^= (rkey)[2];\
s1 ^= (rkey)[3];\
SBOX(s2, s0, s3, s1); \
s3 = HALF_ROR_4(s3);              \
s2 = HALF_ROR_8(s2);              \
s1 = HALF_ROR_12(s1);             \
s3 ^= (rkey)[4];\
s2 ^= (rkey)[5];\
s1 ^= (rkey)[6];\
s0 ^= (rkey)[7];\
SBOX(s3, s2, s1, s0); \
s1 = ROR(s1, 16);                 \
s0 = ROR(s0, 16);                 \
SWAPMOVE(s3, s3, 0x55555555, 1);  \
SWAPMOVE(s0, s0, 0x00005555, 1);  \
SWAPMOVE(s1, s1, 0x55550000, 1);  \
s1 ^= (rkey)[8];\
s3 ^= (rkey)[9];\
s0 ^= (rkey)[10];\
s2 ^= (rkey)[11];\
SBOX(s1, s3, s0, s2); \
s0 = BYTE_ROR_6(s0);              \
s1 = BYTE_ROR_4(s1);              \
s2 = BYTE_ROR_2(s2);              \
s0 ^= (rkey)[12];\
s1 ^= (rkey)[13];\
s2 ^= (rkey)[14];\
s3 ^= (rkey)[15];\
SBOX(s0, s1, s2, s3); \
s2 = ROR(s2, 24);                 \
s0 = ROR(s0, 16);                 \
s3 = ROR(s3, 8);                  \
s2 ^= (rkey)[16];\
s0 ^= (rkey)[17];\
s3 ^= (rkey)[18];\
s1 ^= (rkey)[19];\
})


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


int baksheesh_encrypt_ecb(u8* ctext, const u8* ptext, u32 ptext_len,
                          const u32* rkey) {
    u32 tmp, state[4];
    
    while (ptext_len > 0) {
        
        packing(state, ptext);
        state[0]^=rkey[0];
        state[1]^=rkey[1];
        state[2]^=rkey[2];
        state[3]^=rkey[3];
        
        QUINTUPLE_ROUND(state[0], state[1], state[2], state[3], rkey + 4);
        QUINTUPLE_ROUND(state[2], state[0], state[3], state[1], rkey + 24);
        QUINTUPLE_ROUND(state[3], state[2], state[1], state[0], rkey + 44);
        QUINTUPLE_ROUND(state[1], state[3], state[0], state[2], rkey + 64);
        QUINTUPLE_ROUND(state[0], state[1], state[2], state[3], rkey + 84);
        QUINTUPLE_ROUND(state[2], state[0], state[3], state[1], rkey + 104);
        QUINTUPLE_ROUND(state[3], state[2], state[1], state[0], rkey + 124);
        
        tmp = state[0];
        state[0] = state[1];
        state[1] = state[3];
        state[3] = state[2];
        state[2] = tmp;
        
        
        unpacking(ctext, state);
        ptext += BLOCK_SIZE;
        ctext += BLOCK_SIZE;
        ptext_len -= BLOCK_SIZE;
    }
    return 0;
}

void nibble_swap_roundkey(unsigned char SubKey[36][16]) {
    int nibble_orders[5][32] = {
        {24, 16, 8, 0, 25, 17, 9, 1, 26, 18, 10, 2, 27, 19, 11, 3, 28, 20, 12, 4, 29, 21, 13, 5, 30, 22, 14, 6, 31, 23, 15, 7},
        {30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3, 1},
        {15, 31, 14, 30, 13, 29, 12, 28, 11, 27, 10, 26, 9, 25, 8, 24, 7, 23, 6, 22, 5, 21, 4, 20, 3, 19, 2, 18, 1, 17, 0, 16},
        {3, 7, 11, 15, 19, 23, 27, 31, 2, 6, 10, 14, 18, 22, 26, 30, 1, 5, 9, 13, 17, 21, 25, 29, 0, 4, 8, 12, 16, 20, 24, 28},
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}
    };
    
    for (int i = 0; i < 35; i++) {
        unsigned char tmp[32];
        
        for (int j = 0; j < 16; j++) {
            tmp[31 - (2 * j)] = (SubKey[i + 1][j] >> 4) & 0x0F;
            tmp[31 - (2 * j + 1)] = SubKey[i + 1][j] & 0x0F;
        }
        
        unsigned char nibbles[32];
        for (int j = 0; j < 32; j++) {
            nibbles[j] = tmp[nibble_orders[i % 5][j]];
        }
        
        for (int j = 0; j < 16; j++) {
            SubKey[i + 1][j] = (nibbles[31 - (2 * j)] << 4) | (nibbles[31 - (2 * j + 1)] & 0x0F);
        }
    }
}

void KeySchedule(unsigned char Key[16], unsigned char SubKey[36][16]){
    
    unsigned char tmp[128];
    unsigned char rotated_key[128];
    
    
    
    for(int i=0;i<16;i++){
        SubKey[0][i] = Key[i];
    }
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            tmp[(i * 8) + j] = (SubKey[0][15-i] >> (j)) & 0x1;
        }
    }
    
    
    for(int i =0;i<35;i++){
        //rotation
        for(int j = 0; j < 127; j++){
            rotated_key[j] = tmp[j + 1];
        }
        rotated_key[127] = tmp[0];
        
        for(int j = 0; j < 128; j++){
            tmp[j] = rotated_key[j];
        }
        
        
        rotated_key[8] ^= RoundConstant[i] & (0x1);
        rotated_key[13] ^= (RoundConstant[i]>>1) & (0x1);
        rotated_key[19] ^= (RoundConstant[i]>>2) & (0x1);
        rotated_key[35] ^= (RoundConstant[i]>>3) & (0x1);
        rotated_key[67] ^= (RoundConstant[i]>>4) & (0x1);
        rotated_key[106] ^= (RoundConstant[i]>>5) & (0x1);
        
        
        for (int j = 0; j < 16; j++) {
            SubKey[i+1][15-j] = 0;
            for (int k = 0; k < 8; k++) {
                SubKey[i+1][15-j] |= (rotated_key[(j * 8) + k] & 0x1) << (k);
            }
        }
    }
    
    
}


void roundkey_packing(unsigned char SubKey[36][16], u32 rkey[144]){
    nibble_swap_roundkey(SubKey);
    for(int i =0;i<36;i++){
        packing(&(rkey[i*4]), SubKey[i]);
    }
}


void print_hex(const uint8_t* byte_array, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", byte_array[i]);
    }
    printf("\n");
}


int compare_arrays(const uint8_t* array1, const uint8_t* array2, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (array1[i] != array2[i]) {
            return 0;
        }
    }
    return 1; // Arrays are equal
}

int main() {
    u8 output[BLOCK_SIZE] = {0};
    unsigned char SubKey[36][16];
    u32 rkey[144] = {0,};

    // Loop over all test vectors
    for (int i = 0; i < NUM_TEST_VECTORS; i++) {
        printf("Test Vector %d:\n", i + 1);
        
        // Print the key
        printf("Key: ");
        print_hex(key[i], BLOCK_SIZE);
        
        // Print the plaintext
        printf("Plaintext: ");
        print_hex(plaintext[i], BLOCK_SIZE);
        
        // Generate round keys (Needs optimization for performance)
        KeySchedule(key[i], SubKey);
        roundkey_packing(SubKey, rkey);
        
        // Perform encryption
        baksheesh_encrypt_ecb(output, plaintext[i], BLOCK_SIZE, rkey);
        
        // Print the output ciphertext
        printf("Output Ciphertext: ");
        print_hex(output, BLOCK_SIZE);
        
        printf("Expected Ciphertext: ");
        print_hex(ciphertext[i], BLOCK_SIZE);
        
        // Compare the output with the expected ciphertext
        if (compare_arrays(output, ciphertext[i], BLOCK_SIZE)) {
            printf("Match: Yes\n");
        } else {
            printf("Match: No\n");
        }
        
        printf("\n");
    }

    return 0;
}
