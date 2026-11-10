// Benchmarks the C reference and the two ARM assembly variants (compact,
// masked) of the GIFT-128 heritage of the cipher.
#include "gift128.h"
#pragma GCC optimize ("-O3")

#define DWT_CONTROL ((volatile u32*)0xE0001000)
#define DWT_CYCCNT  ((volatile u32*)0xE0001004)
#define SCB_DEMCR   ((volatile u32*)0xE000EDFC)
#define NUMBER_OF_LOOP 32

typedef void (*encrypt_fn)(u8*, const u32*, const u8*);

static inline u32 getDwtCyccnt(void) { return *DWT_CYCCNT; }
static inline void enableDwtCyccnt(void) {
    *SCB_DEMCR   |= (1u << 24);
    *DWT_CONTROL |= 1;
    *DWT_CYCCNT   = 0;
}

u8 key[KEY_SIZE]   = {0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
                      0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10};
u8 ptext[BLOCK_SIZE] = {0xe3, 0x9c, 0x14, 0x1f, 0xa5, 0x7d, 0xba, 0x43,
                        0xf0, 0x8a, 0x85, 0xb6, 0xa9, 0x1f, 0x86, 0xc1};

static void bench(const char* name, encrypt_fn f, u32* rkey) {
    u8 output[BLOCK_SIZE];
    enableDwtCyccnt();
    u32 start = getDwtCyccnt();
    for (int i = 0; i < NUMBER_OF_LOOP; i++) f(output, rkey, ptext);
    u32 cycles = (getDwtCyccnt() - start) / NUMBER_OF_LOOP;

    Serial.print(name);
    Serial.print(" cyc: ");
    Serial.print(cycles);
    Serial.print("  out:");
    for (int i = 0; i < BLOCK_SIZE; i++) { Serial.print(" "); Serial.print(output[i], HEX); }
    Serial.println();
}

void setup() {
    Serial.begin(9600);

    u32 rkey_compact[160] = {0,};
    gift128_keyschedule(key, rkey_compact);
    bench("compact", gift128_encrypt_block, rkey_compact);

    u32 rkey_masked[164] = {0,};
    bench("masked ", gift128_encrypt_block_masked, rkey_masked);

    u32 rkey_ref[164] = {0,};
    bench("ref    ", gift128_encrypt_block_ref, rkey_ref);
}

void loop() {}
