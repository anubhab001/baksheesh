// Benchmarks the C reference and the three ARM assembly variants
// (compact, fast, masked) of the BAKSHEESH fixslice implementation.
#include "fixslice_cipher.h"
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

u8 ptext[16] = {0xe3, 0x9c, 0x14, 0x1f, 0xa5, 0x7d, 0xba, 0x43,
                0xf0, 0x8a, 0x85, 0xb6, 0xa9, 0x1f, 0x86, 0xc1};

static void bench(const char* name, encrypt_fn f) {
    u32 rkey[144] = {0,};
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
    bench("ref    ", encrypt_block_ref);
    bench("fast   ", encrypt_block_fast);
    bench("compact", encrypt_block_compact);
    bench("masked ", encrypt_block_masked);
}

void loop() {}
