/*
 * BAKSHEESH-GIFT Heritage Fixslice Implementation
 * 
 * Optimized implementation using correct BAKSHEESH S-box
 */

#include "baksheesh_gift_fixslice.h"

// Round constants for BAKSHEESH (same as in existing implementation)
int baksheesh_rconst[35] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};

// Reverse the 32-nibble order of a 16-byte value to match the BAKSHEESH byte convention
static void baksheesh_reverse_nibbles(const u8* in, u8* out) {
    for (int k = 0; k < 16; k++) {
        u8 b = in[15 - k];
        out[k] = ((b & 0x0f) << 4) | ((b >> 4) & 0x0f);
    }
}

// Packing function (input is reversed into the spec nibble order first)
void baksheesh_gift_packing(u32* state, const u8* raw) {
    u8 input[16];
    baksheesh_reverse_nibbles(raw, input);
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

// Unpacking function - exact copy from working implementation
void baksheesh_gift_unpacking(u8* output, u32* state) {
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

// BAKSHEESH-GIFT ECB encryption - adapted from working implementation  
int baksheesh_gift_encrypt_ecb(u8* ctext, const u8* ptext, u32 ptext_len, const u32* rkey) {
    u32 tmp, state[4];
    
    while (ptext_len > 0) {
        baksheesh_gift_packing(state, ptext);
        
        // Add initial round key
        state[0] ^= rkey[0];
        state[1] ^= rkey[1];
        state[2] ^= rkey[2];
        state[3] ^= rkey[3];
        
        // 7 quintuple rounds (35 rounds total)
        BAKSHEESH_GIFT_QUINTUPLE_ROUND(state[0], state[1], state[2], state[3], rkey + 4);
        BAKSHEESH_GIFT_QUINTUPLE_ROUND(state[2], state[0], state[3], state[1], rkey + 24);
        BAKSHEESH_GIFT_QUINTUPLE_ROUND(state[3], state[2], state[1], state[0], rkey + 44);
        BAKSHEESH_GIFT_QUINTUPLE_ROUND(state[1], state[3], state[0], state[2], rkey + 64);
        BAKSHEESH_GIFT_QUINTUPLE_ROUND(state[0], state[1], state[2], state[3], rkey + 84);
        BAKSHEESH_GIFT_QUINTUPLE_ROUND(state[2], state[0], state[3], state[1], rkey + 104);
        BAKSHEESH_GIFT_QUINTUPLE_ROUND(state[3], state[2], state[1], state[0], rkey + 124);
        
        // Final state permutation (from existing implementation)
        tmp = state[0];
        state[0] = state[1];
        state[1] = state[3];
        state[3] = state[2];
        state[2] = tmp;
        
        baksheesh_gift_unpacking(ctext, state);
        ptext += 16;
        ctext += 16;
        ptext_len -= 16;
    }
    return 0;
}

// Rotate the 128-bit key left by one bit in the nibble packing (each group of 4 nibbles
// forms a 16-bit word, LSB first)
static void baksheesh_gift_key_rotate(u8* key) {
    u8 tmp[128], buf[128];
    for (int g = 0; g < 8; g++) {
        u32 w = ((u32)key[4*g] << 12) | ((u32)key[4*g+1] << 8) | ((u32)key[4*g+2] << 4) | (u32)key[4*g+3];
        for (int s = 0; s < 16; s++) tmp[16*g + s] = (w >> s) & 1;
    }
    for (int i = 0; i < 127; i++) buf[i] = tmp[i+1];
    buf[127] = tmp[0];
    for (int g = 0; g < 8; g++) {
        u32 w = 0;
        for (int s = 0; s < 16; s++) w |= ((u32)buf[16*g + s]) << s;
        key[4*g] = (w >> 12) & 0xf; key[4*g+1] = (w >> 8) & 0xf;
        key[4*g+2] = (w >> 4) & 0xf; key[4*g+3] = w & 0xf;
    }
}

// Round-constant tap positions (bit index, MSB first within each nibble)
static const int baksheesh_gift_rc_loc[6] = {11, 14, 16, 32, 64, 105};

// Fold round constant RC[r] into a 16-byte round-key value at the tap positions
static void baksheesh_gift_fold_rc(u8* sk, int r) {
    for (int j = 0; j < 6; j++) {
        if ((baksheesh_rconst[r] >> j) & 1) {
            int p = baksheesh_gift_rc_loc[j];
            int n = p / 4, b = 3 - (p % 4);
            sk[n / 2] ^= 1 << ((n & 1) ? b : b + 4);
        }
    }
}

// Per-round nibble rearrangement matching the accumulated quintuple-round permutation
static void baksheesh_gift_rearrange(u8 SubKey[36][16]) {
    static const int ord[5][32] = {
        {24,16,8,0,25,17,9,1,26,18,10,2,27,19,11,3,28,20,12,4,29,21,13,5,30,22,14,6,31,23,15,7},
        {30,28,26,24,22,20,18,16,14,12,10,8,6,4,2,0,31,29,27,25,23,21,19,17,15,13,11,9,7,5,3,1},
        {15,31,14,30,13,29,12,28,11,27,10,26,9,25,8,24,7,23,6,22,5,21,4,20,3,19,2,18,1,17,0,16},
        {3,7,11,15,19,23,27,31,2,6,10,14,18,22,26,30,1,5,9,13,17,21,25,29,0,4,8,12,16,20,24,28},
        {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31}
    };
    for (int i = 0; i < 35; i++) {
        u8 tmp[32], nb[32];
        for (int j = 0; j < 16; j++) {
            tmp[31 - (2*j)] = (SubKey[i+1][j] >> 4) & 0xf;
            tmp[31 - (2*j+1)] = SubKey[i+1][j] & 0xf;
        }
        for (int j = 0; j < 32; j++) nb[j] = tmp[ord[i%5][j]];
        for (int j = 0; j < 16; j++)
            SubKey[i+1][j] = (nb[31 - (2*j)] << 4) | (nb[31 - (2*j+1)] & 0xf);
    }
}

// BAKSHEESH-GIFT key schedule: expand the 128-bit key into rkey[144] for the fixslice encrypt
void baksheesh_gift_keyschedule(const u8* key, u32* rkey) {
    u8 SubKey[36][16], nibs[32];
    for (int k = 0; k < 16; k++) { nibs[2*k] = key[k] >> 4; nibs[2*k+1] = key[k] & 0xf; }
    for (int k = 0; k < 16; k++) SubKey[0][k] = key[k];
    for (int r = 0; r < 35; r++) {
        baksheesh_gift_key_rotate(nibs);
        for (int k = 0; k < 16; k++) SubKey[r+1][k] = (nibs[2*k] << 4) | nibs[2*k+1];
        baksheesh_gift_fold_rc(SubKey[r+1], r);
    }
    baksheesh_gift_rearrange(SubKey);
    for (int i = 0; i < 36; i++) baksheesh_gift_packing(&rkey[i*4], SubKey[i]);
}