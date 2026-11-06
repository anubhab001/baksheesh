/*
 * BAKSHEESH (GIFT heritage) compact reference for smart-card style targets.
 * 128-bit block and key, 35 rounds, nibble-serial state, round keys on the fly.
 */

#include <stdio.h>
#include <string.h>

#define PRINT_CT

#define Rounds 35

// Known-answer test: all-zero key and plaintext
const unsigned char Key[32] = {0};
const unsigned char Pt[32]  = {0};
const char ExpectedCT[]     = "c002be5e64c78a72ab9a3439518352aa";

// SBox
const unsigned char SBox[16]    = {3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1};
const unsigned char InvSBox[16] = {1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9};

// Bit permutation
const unsigned char PermBits[128] = {0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3, 4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7, 8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11, 12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15, 16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19, 20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23, 24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27, 28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31};
const unsigned char PermBitsInv[128] = {0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63, 64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127, 12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59, 76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123, 8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55, 72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119, 4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51, 68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115};

// 6-bit round constants and their tap positions
const unsigned char RC[Rounds] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};
const unsigned char TP[6] = {8, 13, 19, 35, 67, 106};

void key_update(unsigned char *key);
void key_update_dec(unsigned char *key);
void enc128(unsigned char *state, unsigned char *key);
void dec128(unsigned char *state, unsigned char *key);

int main(void)
{
    unsigned char P[32], K[32];
    char ct[33];
    for (int i = 0; i < 32; i++) { P[i] = Pt[i]; K[i] = Key[i]; }

    enc128(P, K);
    for (int i = 0; i < 32; i++) sprintf(ct + i, "%x", P[31 - i]);

#ifdef PRINT_CT
    printf("Ciphertext: %s\n", ct);
#endif
    printf("Known-answer test: %s\n", strcmp(ct, ExpectedCT) == 0 ? "PASSED" : "FAILED");

    for (int i = 0; i < 32; i++) K[i] = Key[i];
    dec128(P, K);
    int ok = 1;
    for (int i = 0; i < 32; i++) if (P[i] != Pt[i]) ok = 0;
    printf("Round-trip test:   %s\n", ok ? "PASSED" : "FAILED");
    return 0;
}

void enc128(unsigned char *state, unsigned char *key)
{
    unsigned char bits[128], perm[128];

    // Whitening key
    for (int i = 0; i < 32; i++) state[i] ^= key[i];

    for (int r = 0; r < Rounds; r++) {
        // SubCells
        for (int i = 0; i < 32; i++) state[i] = SBox[state[i]];

        // PermBits
        for (int i = 0; i < 32; i++)
            for (int j = 0; j < 4; j++) bits[4 * i + j] = (state[i] >> j) & 0x1;
        for (int i = 0; i < 128; i++) perm[PermBits[i]] = bits[i];

        // AddConstant
        for (int b = 0; b < 6; b++) perm[TP[b]] ^= (RC[r] >> b) & 0x1;

        for (int i = 0; i < 32; i++) {
            state[i] = 0;
            for (int j = 0; j < 4; j++) state[i] ^= perm[4 * i + j] << j;
        }

        // AddRoundKey
        key_update(key);
        for (int i = 0; i < 32; i++) state[i] ^= key[i];
    }
}

void dec128(unsigned char *state, unsigned char *key)
{
    unsigned char bits[128], perm[128];

    // Advance the key to the last round
    for (int i = 0; i < Rounds; i++) key_update(key);

    for (int r = 0; r < Rounds; r++) {
        // AddRoundKey
        for (int i = 0; i < 32; i++) state[i] ^= key[i];
        key_update_dec(key);

        // Inverse PermBits with AddConstant
        for (int i = 0; i < 32; i++)
            for (int j = 0; j < 4; j++) bits[4 * i + j] = (state[i] >> j) & 0x1;
        for (int b = 0; b < 6; b++) bits[TP[b]] ^= (RC[Rounds - 1 - r] >> b) & 0x1;
        for (int i = 0; i < 128; i++) perm[PermBitsInv[i]] = bits[i];
        for (int i = 0; i < 32; i++) {
            state[i] = 0;
            for (int j = 0; j < 4; j++) state[i] ^= perm[4 * i + j] << j;
        }

        // Inverse SubCells
        for (int i = 0; i < 32; i++) state[i] = InvSBox[state[i]];
    }

    // Whitening key
    for (int i = 0; i < 32; i++) state[i] ^= key[i];
}

void key_update(unsigned char *key)
{
    unsigned char tmp[128], buf[128];
    for (int g = 0; g < 8; g++) {
        unsigned int w = ((unsigned int)key[4 * g] << 12) | ((unsigned int)key[4 * g + 1] << 8)
                       | ((unsigned int)key[4 * g + 2] << 4) | (unsigned int)key[4 * g + 3];
        for (int s = 0; s < 16; s++) tmp[16 * g + s] = (w >> s) & 0x1;
    }
    for (int i = 0; i < 127; i++) buf[i] = tmp[i + 1];
    buf[127] = tmp[0];
    for (int g = 0; g < 8; g++) {
        unsigned int w = 0;
        for (int s = 0; s < 16; s++) w |= ((unsigned int)buf[16 * g + s]) << s;
        key[4 * g] = (w >> 12) & 0xF; key[4 * g + 1] = (w >> 8) & 0xF;
        key[4 * g + 2] = (w >> 4) & 0xF; key[4 * g + 3] = w & 0xF;
    }
}

void key_update_dec(unsigned char *key)
{
    unsigned char tmp[128], buf[128];
    for (int g = 0; g < 8; g++) {
        unsigned int w = ((unsigned int)key[4 * g] << 12) | ((unsigned int)key[4 * g + 1] << 8)
                       | ((unsigned int)key[4 * g + 2] << 4) | (unsigned int)key[4 * g + 3];
        for (int s = 0; s < 16; s++) tmp[16 * g + s] = (w >> s) & 0x1;
    }
    for (int i = 0; i < 127; i++) buf[i + 1] = tmp[i];
    buf[0] = tmp[127];
    for (int g = 0; g < 8; g++) {
        unsigned int w = 0;
        for (int s = 0; s < 16; s++) w |= ((unsigned int)buf[16 * g + s]) << s;
        key[4 * g] = (w >> 12) & 0xF; key[4 * g + 1] = (w >> 8) & 0xF;
        key[4 * g + 2] = (w >> 4) & 0xF; key[4 * g + 3] = w & 0xF;
    }
}
