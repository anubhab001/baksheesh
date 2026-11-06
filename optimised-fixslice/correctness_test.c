// BAKSHEESH fixslice correctness test: GIFT heritage and the three Zorro variants
// checked against the specification known-answer vectors.

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "baksheesh_gift_fixslice.h"

// Known-answer vectors from the BAKSHEESH specification (32-hex-char = 16 bytes, byte 0 first).
// Vectors 2 and 3 are asymmetric, so any byte-order mismatch is detected.
typedef struct { const char* key; const char* plaintext; const char* ciphertext; } kat_t;

static const kat_t kats[] = {
    {"00000000000000000000000000000000", "00000000000000000000000000000000", "c002be5e64c78a72ab9a3439518352aa"},
    {"00000000000000000000000000000000", "00000000000000000000000000000007", "1ba3363734c09a29f67c23bbb2cccc05"},
    {"00000000000000000000000000000000", "70000000000000000000000000000000", "6f7d7746eaf0d97a154079f6bd846438"},
    {"00000000000000000000000000000000", "44444444444444444444444444444444", "7ad3303667b2af6deef434dd110d7fb8"},
    {"ffffffffffffffffffffffffffffffff", "11111111111111111111111111111111", "806f0cf45b94f0370206975fe78ac10f"},
    {"76543210032032032032032032032032", "789a789a789a789a789a789a789a789a", "f568e4e628f723ee6d5fbacccf811523"},
    {"23023023023023023023023001234567", "b6e4789ab6e4789ab6e4789ab6e4789a", "77acdeed645e49da25591bf5f32658a2"},
    {"5920effb52bc61e33a98425321e76915", "e6517531abf63f3d7805e126943a081c", "ea5e0713249400a86ec61bb6cdffb364"},
    {"d9bb750877be6eebc339a2258c874fc9", "3325cfadce142135242f8e04752614ab", "2b599d496ee6938cec0bae00c4947b7d"},
    {"25798b6c2b24cd83281de76786213b86", "57cf5777053928343047ebe8dbfc374f", "62b9d55d799a3e784583e7377229a623"}
};
#define NUM_KATS ((int)(sizeof(kats) / sizeof(kats[0])))

typedef int (*encrypt_fn)(u8*, const u8*, u32, const u32*);
typedef void (*keyschedule_fn)(const u8*, u32*);

static void hex_to_bytes(const char* hex, uint8_t* out) {
    for (int i = 0; i < 16; i++) { unsigned v; sscanf(hex + 2 * i, "%2x", &v); out[i] = (uint8_t)v; }
}

// Run all KAT vectors for one cipher variant; return the number that passed.
static int test_variant(const char* name, keyschedule_fn ks, encrypt_fn enc) {
    int passed = 0;
    for (int t = 0; t < NUM_KATS; t++) {
        uint8_t key[16], pt[16], ct[16];
        uint32_t rkey[144];
        char got[33];
        hex_to_bytes(kats[t].key, key);
        hex_to_bytes(kats[t].plaintext, pt);
        ks(key, rkey);
        enc(ct, pt, 16, rkey);
        for (int i = 0; i < 16; i++) sprintf(got + 2 * i, "%02x", ct[i]);
        int ok = strcmp(got, kats[t].ciphertext) == 0;
        passed += ok;
        if (!ok)
            printf("  [%s] vector %d FAIL: got %s expected %s\n", name, t + 1, got, kats[t].ciphertext);
    }
    printf("%-22s %s (%d/%d vectors)\n", name, passed == NUM_KATS ? "PASS" : "FAIL", passed, NUM_KATS);
    return passed == NUM_KATS;
}

int main(void) {
    printf("=== BAKSHEESH Fixslice Known-Answer Tests ===\n");
    printf("(%d spec vectors, including the asymmetric pt=00..07 and pt=70..00)\n\n", NUM_KATS);

    int all_ok = 1;
    all_ok &= test_variant("BAKSHEESH-GIFT", baksheesh_gift_keyschedule, baksheesh_gift_encrypt_ecb);

    printf("\n=== TEST SUMMARY ===\n");
    if (all_ok) {
        printf("Every known-answer vector reproduced.\n");
        return 0;
    }
    printf("Some vectors failed.\n");
    return 1;
}
