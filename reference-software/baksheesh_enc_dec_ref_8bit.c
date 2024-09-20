
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Uncomment to see intermediate results
//#define PRINT_PT_ALL
#define PRINT_CT
#define PRINT_ROUND_KEY
#define ROUND 35

// Key
const unsigned char Key[32] = //{0xd,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,}; //3
{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,}; //1
//{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,}; //2
//{0xd,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,}; //3
//{0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf, 0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf, 0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf, 0xf,0xf,}; //4
//{0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2, 0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2, 0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2, 0x3,0x2,}; //5
//{0x3,0x2,0x0,0x1,0x8,0x9,0xa,0xb,0x3,0x2, 0x0,0x1,0x8,0x9,0xa,0xb,0x3,0x2,0x0,0x1, 0x8,0x9,0xa,0xb,0x3,0x2,0x0,0x1,0x8,0x9, 0xa,0xb,}; //6
//{0xb,0x4,0xc,0x3,0xe,0x7,0x5,0x8,0xf,0x1, 0x2,0x0,0x5,0x6,0x4,0x4,0x0,0xb,0xa,0x9, 0x3,0x2,0x1,0x0,0xb,0xe,0xd,0xb,0xa,0x9, 0x1,0x5,}; //7

// Plaintext
const unsigned char Pt[32] = {0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4, 0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4, 0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4, 0x4,0x4,}; //3
//{0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,}; //1
//{0x7,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,}; //2
//{0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4, 0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4, 0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4,0x4, 0x4,0x4,}; //3
//{0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe, 0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe, 0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe,0xe, 0xe,0x2,}; //4
//{0x6,0x7,0x8,0x9,0x6,0x7,0x8,0x9,0x6,0x7, 0x8,0x9,0x6,0x7,0x8,0x9,0x6,0x7,0x8,0x9, 0x6,0x7,0x8,0x9,0x6,0x7,0x8,0x9,0x6,0x7, 0x8,0x9,}; //5
//{0xc,0xd,0xf,0xe,0xc,0xd,0xf,0xe,0xc,0xd, 0xf,0xe,0xc,0xd,0xf,0xe,0xc,0xd,0xf,0xe, 0xc,0xd,0xf,0xe,0xc,0xd,0xf,0xe,0xc,0xd, 0xf,0xe,}; //6
//{0x2,0x3,0xe,0xb,0xa,0x1,0x6,0xb,0x5,0x2, 0x4,0x1,0x1,0x0,0x7,0xd,0x8,0x3,0x4,0x1, 0x0,0x9,0xc,0xb,0x8,0x6,0xc,0x5,0xd,0x4, 0xb,0x4,}; //7

//Sbox
const unsigned char SBox[16] = {3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1};
const unsigned char InvSBox[16] = {1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9};

//bit permutation
const unsigned char PermBits[128] = {0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3, 4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7, 8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11, 12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15, 16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19, 20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23, 24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27, 28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31};
const unsigned char PermBitsInv[128] = {0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63, 64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127, 12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59, 76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123, 8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55, 72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119, 4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51, 68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115};

// round constants
const unsigned char RC[ROUND] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};
// tap positions for round contants
const unsigned char TP[6] = {8, 13, 19, 35, 67, 106};

void Baksheesh_enc(unsigned char *state, unsigned char *key);
void Key_update(unsigned char *key);
void Baksheesh_dec(unsigned char *state, unsigned char *key);
void Key_update_dec(unsigned char *key);

//to check the internal values
void Display_state_nibble(unsigned char *state);
void Display_state_bit(unsigned char *state);
/***************************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************************/

int main(void){

  unsigned char P[32];
  unsigned char K[32];

  for(int i = 0; i < 32; i++){
    P[i] = Pt[i];
    K[i] = Key[i];
  }

  //encryption
  printf("Encryption\n");

  printf("Key       : ");
  Display_state_nibble(K);

  printf("Plaintext : ");
  Display_state_nibble(P);

  Baksheesh_enc(P, K);

  printf("Ciphertext: ");
  Display_state_nibble(P);


  //decryption
  for(int i = 0; i < 32; i++){
    K[i] = Key[i];
  }
  printf("Decryption\n");

  printf("Key       : ");
  Display_state_nibble(K);

  printf("Ciphertext: ");
  Display_state_nibble(P);

  Baksheesh_dec(P, K);

  printf("Plaintext : ");
  Display_state_nibble(P);


  return 0;
}

/***************************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************************/

void Baksheesh_enc(unsigned char *state, unsigned char *key){

  //whitening key
  for(int i = 0; i < 32; i++){
    state[i] ^= key[i];
  }

  //round function
  for(int r = 0; r < ROUND; r++){

    //subcells
    for(int i = 0; i < 32; i++){
      state[i] = SBox[state[i]];
    }

    //permbits
    //convert nibble-wise variables into bit-wise variables
    unsigned char tmp[128];
    unsigned char buf[128];
    for(int i = 0; i < 32; i++){
      for(int j = 0; j < 4; j++){
        tmp[(i * 4) + j] = (state[i] >> j) & 0x1;
      }
    }
    //bit permutation
    for(int i = 0; i < 128; i++){
      buf[PermBits[i]] = tmp[i];
    }

    // addconstant
    buf[TP[0]] ^= RC[r] & 0x1;
    buf[TP[1]] ^= (RC[r] >> 1) & 0x1;
    buf[TP[2]] ^= (RC[r] >> 2) & 0x1;
    buf[TP[3]] ^= (RC[r] >> 3) & 0x1;
    buf[TP[4]] ^= (RC[r] >> 4) & 0x1;
    buf[TP[5]] ^= (RC[r] >> 5) & 0x1;

    //convert bit-wise variables into nibble-wise variables
    for(int i = 0; i < 32; i++){
      state[i] = buf[(4 * i)] ^ (buf[(4 * i) + 1] << 1) ^ (buf[(4 * i) + 2] << 2) ^ (buf[(4 * i) + 3] << 3);
    }

    //keyupdate
    Key_update(key);

    //addroundkey
    for(int i = 0; i < 32; i++){
      state[i] ^= (key[i] & 0b1101);
    }

  }
}

void Baksheesh_dec(unsigned char *state, unsigned char *key){

  //set key
  for(int i = 0; i < ROUND; i++){
    Key_update(key);
  }

  //round function
  for(int r = 0; r < ROUND; r++){


    //addroundkey
    for(int i = 0; i < 32; i++){
      state[i] ^= (key[i] & 0b1101);
    }

    Key_update_dec(key);

    //permbits
    //convert nibble-wise variables into bit-wise variables
    unsigned char tmp[128];
    unsigned char buf[128];
    for(int i = 0; i < 32; i++){
      for(int j = 0; j < 4; j++){
        tmp[(i * 4) + j] = (state[i] >> j) & 0x1;
      }
    }
    //addconstant
    tmp[TP[0]] ^= RC[ROUND - 1 - r] & 0x1;
    tmp[TP[1]] ^= (RC[ROUND - 1 - r] >> 1) & 0x1;
    tmp[TP[2]] ^= (RC[ROUND - 1 - r] >> 2) & 0x1;
    tmp[TP[3]] ^= (RC[ROUND - 1 - r] >> 3) & 0x1;
    tmp[TP[4]] ^= (RC[ROUND - 1 - r] >> 4) & 0x1;
    tmp[TP[5]] ^= (RC[ROUND - 1 - r] >> 5) & 0x1;

    //bit permutation
    for(int i = 0; i < 128; i++){
      buf[PermBitsInv[i]] = tmp[i];
    }
    //convert bit-wise variables into nibble-wise variables
    for(int i = 0; i < 32; i++){
      state[i] = buf[(4 * i)] ^ (buf[(4 * i) + 1] << 1) ^ (buf[(4 * i) + 2] << 2) ^ (buf[(4 * i) + 3] << 3);
    }

    //subcells
    for(int i = 0; i < 32; i++){
      state[i] = InvSBox[state[i]];
    }


  }

//  Key_update_dec(key);

  //whitening key
  for(int i = 0; i < 32; i++){
    state[i] ^= key[i];
  }

}

void Key_update(unsigned char *key){

  //convert nibble-wise variables into bit-wise variables
  unsigned char tmp[128];
  unsigned char buf[128];

  for(int i = 0; i < 32; i++){
    for(int j = 0; j < 4; j++){
      tmp[(i * 4) + j] = (key[i] >> j) & 0x1;
    }
  }

  //rotation
  for(int i = 0; i < 127; i++){
    buf[i] = tmp[i + 1];
  }
  buf[127] = tmp[0];

  //convert bit-wise variables into nibble-wise variables
  for(int i = 0; i < 32; i++){
    key[i] = buf[(4 * i)] ^ (buf[(4 * i) + 1] << 1) ^ (buf[(4 * i) + 2] << 2) ^ (buf[(4 * i) + 3] << 3);
  }

}

void Key_update_dec(unsigned char *key){

  //convert nibble-wise variables into bit-wise variables
  unsigned char tmp[128];
  unsigned char buf[128];

  for(int i = 0; i < 32; i++){
    for(int j = 0; j < 4; j++){
      tmp[(i * 4) + j] = (key[i] >> j) & 0x1;
    }
  }

  //rotation
  for(int i = 0; i < 127; i++){
     buf[i + 1] = tmp[i];
  }
  buf[0] = tmp[127];

  //convert bit-wise variables into nibble-wise variables
  for(int i = 0; i < 32; i++){
    key[i] = buf[(4 * i)] ^ (buf[(4 * i) + 1] << 1) ^ (buf[(4 * i) + 2] << 2) ^ (buf[(4 * i) + 3] << 3);
  }

}

/***************************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************************/


void Display_state_nibble(unsigned char *state){

  for(int i = 0; i < 32; i++){
    printf("%x", state[32 - i - 1]);
  }
  printf("\n");

}

void Display_state_bit(unsigned char *state){

  printf(" ");
  for(int i = 0; i < 32; i++){
    for(int j = 0; j < 4; j++){
      printf("%x", ((state[32 - i - 1] >> j) & 0x1));
    }
    printf(" ");
  }
  printf("\n");
}

/*
Cross-check with this test vector
Encryption
Key       : 00000000000000000000000000000000
Plaintext : 44444444444444444444444444444444
Ciphertext: 7ad3303667b2af6deef434dd110d7fb8
*/