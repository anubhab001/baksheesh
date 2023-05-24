#include "fixslice_cipher.h"

#pragma GCC optimize ("-O3")
#include <stdio.h>
#include <string.h>



#define DWT_CONTROL ((volatile u32*)0xE0001000)
#define DWT_CYCCNT ((volatile u32*)0xE0001004)
#define SCB_DEMCR ((volatile u32*)0xE000EDFC)

#define NUMBER_OF_LOOP 32



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



void setup() {
Serial.begin(9600);
u32 start, elapsed;
    u32 total_cycle;
    
u8 output[BLOCK_SIZE];
  
    u32 rkey[144] = {0,};
    //gift128_keyschedule(key[0], rkey);
    
    enableDwtCyccnt();
    resetDwtCyccnt();

  start = getDwtCyccnt();
  
  
  for (int i = 0; i < NUMBER_OF_LOOP; i++) {
     encrypt_block(output, rkey, ptext[2] );
  }
    // measure elapsed
  elapsed     = getDwtCyccnt() - start;
  total_cycle = elapsed / NUMBER_OF_LOOP;
  
  
  printf("test");
   Serial.print("cyc: ");
  Serial.println(total_cycle);
  Serial.println(total_cycle/16);
  Serial.print("out: ");
  Serial.print(output[0],HEX);
  Serial.print(" ");
  Serial.print(output[1],HEX);
  Serial.print(" ");
  Serial.print(output[2],HEX);
  Serial.print(" ");
  Serial.print(output[3],HEX);
  Serial.print(" ");
  Serial.print(output[4],HEX);
  Serial.print(" ");
  Serial.print(output[5],HEX);
  Serial.print(" ");
  Serial.print(output[6],HEX);
  Serial.print(" ");
  Serial.print(output[7],HEX);
  Serial.print(" ");
  Serial.print(output[8],HEX);
  Serial.print(" ");
  Serial.print(output[9],HEX);
  Serial.print(" ");
  Serial.print(output[10],HEX);
  Serial.print(" ");
  Serial.print(output[11],HEX);
  Serial.print(" ");
  Serial.print(output[12],HEX);
  Serial.print(" ");
  Serial.print(output[13],HEX);
  Serial.print(" ");
  Serial.print(output[14],HEX);
  Serial.print(" ");
  Serial.println(output[15],HEX);
  

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
