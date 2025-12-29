#pragma GCC optimize ("-O3")
#include <stdio.h>
#include <string.h>

//#include <encrypt.h>
//#include <endian.h>
//#include <gift128.h>
//#include <key_schedule.h>
typedef unsigned char u8;
typedef unsigned int u32;


#define DWT_CONTROL ((volatile u32*)0xE0001000)
#define DWT_CYCCNT ((volatile u32*)0xE0001004)
#define SCB_DEMCR ((volatile u32*)0xE000EDFC)

#define NUMBER_OF_LOOP 32

#define ROR(x,y)                      \
  (((x) >> (y)) | ((x) << (32 - (y))))
#define BYTE_ROR_2(x)                   \
  ((((x) >> 2) & 0x3f3f3f3f)  | (((x) & 0x03030303) << 6))
#define BYTE_ROR_4(x)                     \
  ((((x) >> 4) & 0x0f0f0f0f)  | (((x) & 0x0f0f0f0f) << 4))
#define BYTE_ROR_6(x)                     \
  ((((x) >> 6) & 0x03030303)  | (((x) & 0x3f3f3f3f) << 2))
#define HALF_ROR_4(x)                     \
  ((((x) >> 4) & 0x0fff0fff)  | (((x) & 0x000f000f) << 12))
#define HALF_ROR_8(x)                     \
  ((((x) >> 8) & 0x00ff00ff)  | (((x) & 0x00ff00ff) << 8))
#define HALF_ROR_12(x)                    \
  ((((x) >> 12)& 0x000f000f)  | (((x) & 0x0fff0fff) << 4))
#define NIBBLE_ROR_1(x)                   \
  ((((x) >> 1) & 0x77777777)  | (((x) & 0x11111111) << 3))
#define NIBBLE_ROR_2(x)                   \
  ((((x) >> 2) & 0x33333333)  | (((x) & 0x33333333) << 2))
#define NIBBLE_ROR_3(x)                   \
  ((((x) >> 3) & 0x11111111)  | (((x) & 0x77777777) << 1))

#define SWAPMOVE(a, b, mask, n)               \
  tmp = (b ^ (a >> n)) & mask;              \
  b ^= tmp;                       \
  a ^= (tmp << n);

#define SBOX(s0, s1, s2, s3)                \
  s1 ^= s0 & s2;                      \
  s0 ^= s1 & s3;                      \
  s2 ^= s0 | s1;                      \
  s3 ^= s2;                       \
  s1 ^= s3;                       \
  s3 ^= 0xffffffff;                   \
  s2 ^= s0 & s1;

#define INV_SBOX(s0, s1, s2, s3)              \
  s2 ^= s3 & s1;                      \
  s0 ^= 0xffffffff;                   \
  s1 ^= s0;                       \
  s0 ^= s2;                       \
  s2 ^= s3 | s1;                      \
  s3 ^= s1 & s0;                      \
  s1 ^= s3 & s2;

#define QUINTUPLE_ROUND(state, rkey, rconst) ({       \
  SBOX(state[0], state[1], state[2], state[3]);     \
  state[3] = NIBBLE_ROR_1(state[3]);            \
  state[1] = NIBBLE_ROR_2(state[1]);            \
  state[2] = NIBBLE_ROR_3(state[2]);            \
  state[0] ^= (rkey)[0];                  \
  state[1] ^= (rkey)[1];                  \
  state[2] ^= (rkey)[2];                  \
  state[3] ^= (rkey)[3];                  \
  state[0] ^= (rconst)[0];                \
  SBOX(state[3], state[1], state[2], state[0]);     \
  state[0] = HALF_ROR_4(state[0]);            \
  state[1] = HALF_ROR_8(state[1]);            \
  state[2] = HALF_ROR_12(state[2]);           \
  state[0] ^= (rkey)[4];                  \
  state[1] ^= (rkey)[5];                  \
  state[2] ^= (rkey)[6];                  \
  state[3] ^= (rkey)[7];                  \
  state[3] ^= (rconst)[1];                \
  SBOX(state[0], state[1], state[2], state[3]);     \
  state[3] = ROR(state[3], 16);             \
  state[2] = ROR(state[2], 16);             \
  SWAPMOVE(state[1], state[1], 0x55555555, 1);      \
  SWAPMOVE(state[2], state[2], 0x00005555, 1);      \
  SWAPMOVE(state[3], state[3], 0x55550000, 1);      \
  state[0] ^= (rkey)[8];                  \
  state[1] ^= (rkey)[9];                  \
  state[2] ^= (rkey)[10];                  \
  state[3] ^= (rkey)[11];                  \
  state[0] ^= (rconst)[2];                \
  SBOX(state[3], state[1], state[2], state[0]);     \
  state[0] = BYTE_ROR_6(state[0]);            \
  state[1] = BYTE_ROR_4(state[1]);            \
  state[2] = BYTE_ROR_2(state[2]);            \
  state[0] ^= (rkey)[12];                  \
  state[1] ^= (rkey)[13];                  \
  state[2] ^= (rkey)[14];                  \
  state[3] ^= (rkey)[15];                  \
  state[3] ^= (rconst)[3];                \
  SBOX(state[0], state[1], state[2], state[3]);     \
  state[3] = ROR(state[3], 24);             \
  state[1] = ROR(state[1], 16);             \
  state[2] = ROR(state[2], 8);              \
  state[0] ^= (rkey)[16];                  \
  state[1] ^= (rkey)[17];                  \
  state[2] ^= (rkey)[18];                  \
  state[3] ^= (rkey)[19];                  \
  state[0] ^= (rconst)[4];                \
  state[0] ^= state[3];                 \
  state[3] ^= state[0];                 \
  state[0] ^= state[3];                 \
})

#define INV_QUINTUPLE_ROUND(state, rkey, rconst) ({     \
  state[0] ^= state[3];                 \
  state[3] ^= state[0];                 \
  state[0] ^= state[3];                 \
  state[1] ^= (rkey)[8];                  \
  state[2] ^= (rkey)[9];                  \
  state[0] ^= (rconst)[4];                \
  state[3] = ROR(state[3], 8);              \
  state[1] = ROR(state[1], 16);             \
  state[2] = ROR(state[2], 24);             \
  INV_SBOX(state[3], state[1], state[2], state[0]);   \
  state[1] ^= (rkey)[6];                  \
  state[2] ^= (rkey)[7];                  \
  state[3] ^= (rconst)[3];                \
  state[0] = BYTE_ROR_2(state[0]);            \
  state[1] = BYTE_ROR_4(state[1]);            \
  state[2] = BYTE_ROR_6(state[2]);            \
  INV_SBOX(state[0], state[1], state[2], state[3]);   \
  state[1] ^= (rkey)[4];                  \
  state[2] ^= (rkey)[5];                  \
  state[0] ^= (rconst)[2];                \
  SWAPMOVE(state[3], state[3], 0x55550000, 1);      \
  SWAPMOVE(state[1], state[1], 0x55555555, 1);      \
  SWAPMOVE(state[2], state[2], 0x00005555, 1);      \
  state[3] = ROR(state[3], 16);             \
  state[2] = ROR(state[2], 16);             \
  INV_SBOX(state[3], state[1], state[2], state[0]);   \
  state[1] ^= (rkey)[2];                  \
  state[2] ^= (rkey)[3];                  \
  state[3] ^= (rconst)[1];                \
  state[0] = HALF_ROR_12(state[0]);           \
  state[1] = HALF_ROR_8(state[1]);            \
  state[2] = HALF_ROR_4(state[2]);            \
  INV_SBOX(state[0], state[1], state[2], state[3]);   \
  state[1] ^= (rkey)[0];                  \
  state[2] ^= (rkey)[1];                  \
  state[0] ^= (rconst)[0];                \
  state[3] = NIBBLE_ROR_3(state[3]);            \
  state[1] = NIBBLE_ROR_2(state[1]);            \
  state[2] = NIBBLE_ROR_1(state[2]);            \
  INV_SBOX(state[3], state[1], state[2], state[0]);   \
})

#define U32BIG(x)                      \
  ((((x) & 0x000000FF) << 24) | (((x) & 0x0000FF00) << 8) | \
   (((x) & 0x00FF0000) >> 8) | (((x) & 0xFF000000) >> 24))

#define U8BIG(x, y)                     \
  (x)[0] = (y) >> 24;                   \
  (x)[1] = ((y) >> 16) & 0xff;              \
  (x)[2] = ((y) >> 8) & 0xff;               \
  (x)[3] = (y) & 0xff;

const u32 rconst[40] = {
  0x10000008, 0x80018000, 0x54000002, 0x01010181,
  0x8000001f, 0x10888880, 0x6001e000, 0x51500002,
  0x03030180, 0x8000002f, 0x10088880, 0x60016000,
  0x41500002, 0x03030080, 0x80000027, 0x10008880,
  0x4001e000, 0x11500002, 0x03020180, 0x8000002b,
  0x10080880, 0x60014000, 0x01400002, 0x02020080,
  0x80000021, 0x10000080, 0x0001c000, 0x51000002,
  0x03010180, 0x8000002e, 0x10088800, 0x60012000,
  0x40500002, 0x01030080, 0x80000006, 0x10008808,
  0xc001a000, 0x14500002, 0x01020181, 0x8000001a
};



#define REARRANGE_RKEY_0(x) ({      \
  SWAPMOVE(x, x, 0x00550055, 9);    \
  SWAPMOVE(x, x, 0x000f000f, 12);   \
  SWAPMOVE(x, x, 0x00003333, 18);   \
  SWAPMOVE(x, x, 0x000000ff, 24);   \
})

#define REARRANGE_RKEY_1(x) ({      \
  SWAPMOVE(x, x, 0x11111111, 3);    \
  SWAPMOVE(x, x, 0x03030303, 6);    \
  SWAPMOVE(x, x, 0x000f000f, 12);   \
  SWAPMOVE(x, x, 0x000000ff, 24);   \
})

#define REARRANGE_RKEY_2(x) ({      \
  SWAPMOVE(x, x, 0x0000aaaa, 15);   \
  SWAPMOVE(x, x, 0x00003333, 18);   \
  SWAPMOVE(x, x, 0x0000f0f0, 12);   \
  SWAPMOVE(x, x, 0x000000ff, 24);   \
})

#define REARRANGE_RKEY_3(x) ({      \
  SWAPMOVE(x, x, 0x0a0a0a0a, 3);    \
  SWAPMOVE(x, x, 0x00cc00cc, 6);    \
  SWAPMOVE(x, x, 0x0000f0f0, 12);   \
  SWAPMOVE(x, x, 0x000000ff, 24);   \
})

#define KEY_UPDATE(x)                     \
  (((x) >> 12) & 0x0000000f)  | (((x) & 0x00000fff) << 4) |   \
  (((x) >> 2) & 0x3fff0000) | (((x) & 0x00030000) << 14)

#define KEY_TRIPLE_UPDATE_0(x)                  \
  (ROR((x) & 0x33333333, 24)  | ROR((x) & 0xcccccccc, 16))

#define KEY_DOUBLE_UPDATE_1(x)                  \
  ((((x) >> 4) & 0x0f000f00)  | (((x) & 0x0f000f00) << 4) |   \
  (((x) >> 6) & 0x00030003) | (((x) & 0x003f003f) << 2))

#define KEY_TRIPLE_UPDATE_1(x)                  \
  ((((x) >> 6) & 0x03000300)  | (((x) & 0x3f003f00) << 2) |   \
  (((x) >> 5) & 0x00070007) | (((x) & 0x001f001f) << 3))

#define KEY_DOUBLE_UPDATE_2(x)                  \
  (ROR((x) & 0xaaaaaaaa, 24)  | ROR((x) & 0x55555555, 16))

#define KEY_TRIPLE_UPDATE_2(x)                  \
  (ROR((x) & 0x55555555, 24)  | ROR((x) & 0xaaaaaaaa, 20))

#define KEY_DOUBLE_UPDATE_3(x)                  \
  ((((x) >> 2) & 0x03030303)  | (((x) & 0x03030303) << 2) |   \
  (((x) >> 1) & 0x70707070) | (((x) & 0x10101010) << 3))

#define KEY_TRIPLE_UPDATE_3(x)                  \
  ((((x) >> 18) & 0x00003030) | (((x) & 0x01010101) << 3) |   \
  (((x) >> 14) & 0x0000c0c0)  | (((x) & 0x0000e0e0) << 15)| \
  (((x) >> 1) & 0x07070707) | (((x) & 0x00001010) << 19))

#define KEY_DOUBLE_UPDATE_4(x)                  \
  ((((x) >> 4)  & 0x0fff0000) | (((x) & 0x000f0000) << 12) |  \
  (((x) >> 8)  & 0x000000ff)  | (((x) & 0x000000ff) << 8))

#define KEY_TRIPLE_UPDATE_4(x)                  \
  ((((x) >> 6)  & 0x03ff0000) | (((x) & 0x003f0000) << 10) |  \
  (((x) >> 4)  & 0x00000fff)  | (((x) & 0x0000000f) << 12))

/*****************************************************************************
* The first 20 rkeys are computed using the classical representation before
* being rearranged into fixsliced representations depending on round numbers.
* The 60 remaining rkeys are directly computed in fixsliced representations.
*****************************************************************************/


/*****************************************************************************
* Rearranges the input in a row-wise bitsliced manner.
*****************************************************************************/
void packing(u32* state, const u8* input) {
  u32 tmp;
  state[0] =  (input[6] << 24)  | (input[7] << 16)  |
        (input[14] << 8)  | input[15];
  state[1] =  (input[4] << 24)  | (input[5] << 16)  |
        (input[12] << 8)  | input[13];
  state[2] =  (input[2] << 24)  | (input[3] << 16)  |
        (input[10] << 8)  | input[11];
  state[3] =  (input[0] << 24)  | (input[1] << 16)  |
        (input[8] << 8)   | input[9];
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
  output[0] = state[3] >> 24; output[1] = (state[3] >> 16) & 0xff;
  output[2] = state[2] >> 24; output[3] = (state[2] >> 16) & 0xff;
  output[4] = state[1] >> 24; output[5] = (state[1] >> 16) & 0xff;
  output[6] = state[0] >> 24; output[7] = (state[0] >> 16) & 0xff;
  output[8] = (state[3] >> 8) & 0xff; output[9] = state[3] & 0xff;
  output[10] = (state[2] >> 8) & 0xff; output[11] = state[2] & 0xff;
  output[12] = (state[1] >> 8) & 0xff; output[13] = state[1] & 0xff;
  output[14] = (state[0] >> 8) & 0xff; output[15] = state[0] & 0xff;
}

/*****************************************************************************
* Encryption of 128-bit blocks using GIFT-128 in ECB mode.
* Note that 'ptext_len' must be a mutliple of 16.
*****************************************************************************/

#define BLOCK_SIZE  16
#define KEY_SIZE  16
/*****************************************************************************
* Decryption of 128-bit blocks using GIFT-128 in ECB mode.
* Note that 'ctext_len' must be a mutliple of 16.
*****************************************************************************/
void setup();
void loop();

static inline u32 getDwtCyccnt(void);
static inline void resetDwtCyccnt(void);
static inline void enableDwtCyccnt(void);


u8 key[3][KEY_SIZE] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //1st key
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,  //2nd key
  0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10},
  {0xd0, 0xf5, 0xc5, 0x9a, 0x77, 0x00, 0xd3, 0xe7,  //3rd key
  0x99, 0x02, 0x8f, 0xa9, 0xf9, 0x0a, 0xd8, 0x37}
};
u8 ptext[3][BLOCK_SIZE] = {
  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //1st plaintext
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
  {0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,  //2nd plaintext
  0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10},
  {0xe3, 0x9c, 0x14, 0x1f, 0xa5, 0x7d, 0xba, 0x43,  //3rd plaintext
  0xf0, 0x8a, 0x85, 0xb6, 0xa9, 0x1f, 0x86, 0xc1}
};
u8 ctext[3][BLOCK_SIZE] = {
  {0xcd, 0x0b, 0xd7, 0x38, 0x38, 0x8a, 0xd3, 0xf6,  //1st ciphertext
  0x68, 0xb1, 0x5a, 0x36, 0xce, 0xb6, 0xff, 0x92},
  {0x84, 0x22, 0x24, 0x1a, 0x6d, 0xbf, 0x5a, 0x93,  //2nd ciphertext
  0x46, 0xaf, 0x46, 0x84, 0x09, 0xee, 0x01, 0x52},
  {0x13, 0xed, 0xe6, 0x7c, 0xbd, 0xcc, 0x3d, 0xbf,  //3rd ciphertext
  0x40, 0x0a, 0x62, 0xd6, 0x97, 0x72, 0x65, 0xea}
};
int gift128_encrypt_ecb(u8* ctext, const u8* ptext, u32 ptext_len, const u32* rkey) {
  u32 tmp, state[4];//, rkey[80];
  //precompute_rkeys(rkey, key);
  while(ptext_len > 0) {
    packing(state, ptext);
    state[0] ^= rkey[0];
    state[1] ^= rkey[1];
    state[2] ^= rkey[2];
    state[3] ^= rkey[3];
    for(int i = 0; i < 40; i+=5)
      QUINTUPLE_ROUND(state, rkey + (i+1)*4, rconst + i);
    unpacking(ctext, state);
    ptext += BLOCK_SIZE;
    ctext += BLOCK_SIZE;
    ptext_len -= BLOCK_SIZE;
  }
  return 0;
}


void setup() {
Serial.begin(9600);
u32 start, elapsed;
    u32 total_cycle;
    u32 rkey[164] = {0,};
u8 output[BLOCK_SIZE];


    
    enableDwtCyccnt();
    resetDwtCyccnt();

  start = getDwtCyccnt();
  
  
  for (int i = 0; i < NUMBER_OF_LOOP; i++) {
      // CANDY(plainText0, plainText1, cipherText0, cipherText1, key);
      //gift128_encrypt_ecb(output, ptext[i], BLOCK_SIZE, rkey);
  }
    // measure elapsed
  elapsed     = getDwtCyccnt() - start;
  total_cycle = elapsed / NUMBER_OF_LOOP;
   Serial.print("cyc: ");
  Serial.println(total_cycle);
  Serial.println(total_cycle/16);
  Serial.println(output[0]);

}

void loop() {
  // put your main code here, to run repeatedly:

}

static inline u32 getDwtCyccnt(void) { return *DWT_CYCCNT; }

static inline void resetDwtCyccnt(void) {
    *DWT_CYCCNT = 0;  // reset the counter
}

static inline void enableDwtCyccnt(void) {
    *SCB_DEMCR   = *SCB_DEMCR | (1u << 24);  // TRCENA = 1
    *DWT_CONTROL = *DWT_CONTROL | 1;  // enable the counter (CYCCNTENA = 1)
    *DWT_CYCCNT  = 0;                 // reset the counter
}
