
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Uncomment to see intermediate results
//#define PRINT_PT_ALL
#define PRINT_CT
#define PRINT_ROUND_KEY
#define ROUND 35

//SBox
const unsigned char SBox[16] = {3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1};
const unsigned char InvSBox[16] = {1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9};

//Bit permutation
const unsigned char PermBits[128] = {0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3, 4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7, 8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11, 12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15, 16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19, 20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23, 24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27, 28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31};
const unsigned char PermBitsInv[128] = {0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63, 64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127, 12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59, 76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123, 8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55, 72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119, 4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51, 68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115};

//Round constants
const unsigned char RC[ROUND] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};
//Tap positions for round constants
const unsigned char TP[6] = {8, 13, 19, 35, 67, 106};

void Baksheesh_enc(unsigned char *state, unsigned char *key);
void Key_update(unsigned char *key);
void Baksheesh_dec(unsigned char *state, unsigned char *key);
void Key_update_dec(unsigned char *key);

//To check the internal values
void Display_state_nibble(unsigned char *state);
void Display_state_bit(unsigned char *state);
/***************************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************************/

//Convert a 32-character hex string into a nibble array (leftmost character first)
void Hex_to_nibble(const char *hex, unsigned char *out){
  for(int i = 0; i < 32; i++){
    char c = hex[i];
    out[i] = (c <= '9') ? (c - '0') : ((c | 0x20) - 'a' + 10);
  }
}

int main(void){

  //Known-answer tests (key, plaintext, ciphertext), including asymmetric inputs
  const char *kat[][3] = {
    {"00000000000000000000000000000000", "00000000000000000000000000000000", "c002be5e64c78a72ab9a3439518352aa"},
    {"00000000000000000000000000000000", "00000000000000000000000000000007", "1ba3363734c09a29f67c23bbb2cccc05"},
    {"00000000000000000000000000000000", "70000000000000000000000000000000", "6f7d7746eaf0d97a154079f6bd846438"},
    {"00000000000000000000000000000000", "44444444444444444444444444444444", "7ad3303667b2af6deef434dd110d7fb8"},
    {"ffffffffffffffffffffffffffffffff", "11111111111111111111111111111111", "806f0cf45b94f0370206975fe78ac10f"},
    {"d9bb750877be6eebc339a2258c874fc9", "3325cfadce142135242f8e04752614ab", "2b599d496ee6938cec0bae00c4947b7d"},
    {"76543210032032032032032032032032", "789a789a789a789a789a789a789a789a", "f568e4e628f723ee6d5fbacccf811523"},
    {"23023023023023023023023001234567", "b6e4789ab6e4789ab6e4789ab6e4789a", "77acdeed645e49da25591bf5f32658a2"},
    {"5920effb52bc61e33a98425321e76915", "e6517531abf63f3d7805e126943a081c", "ea5e0713249400a86ec61bb6cdffb364"},
    {"25798b6c2b24cd83281de76786213b86", "57cf5777053928343047ebe8dbfc374f", "62b9d55d799a3e784583e7377229a623"},
  };
  int n = (int)(sizeof(kat) / sizeof(kat[0]));
  int pass = 0;

  for(int t = 0; t < n; t++){
    unsigned char K[32], P[32], C[32], key[32], state[32];
    Hex_to_nibble(kat[t][0], K);
    Hex_to_nibble(kat[t][1], P);
    Hex_to_nibble(kat[t][2], C);

    for(int i = 0; i < 32; i++){ state[i] = P[i]; key[i] = K[i]; }
    Baksheesh_enc(state, key);
    int enc_ok = 1;
    for(int i = 0; i < 32; i++){ if(state[i] != C[i]) enc_ok = 0; }

    printf("Test %d (encryption %s)\n", t + 1, enc_ok ? "PASS" : "FAIL");
    printf("Key       : "); Display_state_nibble(K);
    printf("Plaintext : "); Display_state_nibble(P);
    printf("Ciphertext: "); Display_state_nibble(state);

    for(int i = 0; i < 32; i++){ key[i] = K[i]; }
    Baksheesh_dec(state, key);
    int dec_ok = 1;
    for(int i = 0; i < 32; i++){ if(state[i] != P[i]) dec_ok = 0; }
    printf("Decryption: %s\n\n", dec_ok ? "PASS" : "FAIL");

    if(enc_ok && dec_ok){ pass++; }
  }

  printf("Result: %d/%d passed\n", pass, n);

  return (pass == n) ? 0 : 1;
}

/***************************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************************/

void Baksheesh_enc(unsigned char *state, unsigned char *key){

  //Whitening key
  for(int i = 0; i < 32; i++){
    state[i] ^= key[i];
  }

  //Round function
  for(int r = 0; r < ROUND; r++){

    //SubCells
    for(int i = 0; i < 32; i++){
      state[i] = SBox[state[i]];
    }

    //PermBits
    //Convert nibble-wise variables into bit-wise variables
    unsigned char tmp[128];
    unsigned char buf[128];
    for(int i = 0; i < 32; i++){
      for(int j = 0; j < 4; j++){
        tmp[(i * 4) + j] = (state[i] >> j) & 0x1;
      }
    }
    //Bit permutation
    for(int i = 0; i < 128; i++){
      buf[PermBits[i]] = tmp[i];
    }

    //AddConstants
    buf[TP[0]] ^= RC[r] & 0x1;
    buf[TP[1]] ^= (RC[r] >> 1) & 0x1;
    buf[TP[2]] ^= (RC[r] >> 2) & 0x1;
    buf[TP[3]] ^= (RC[r] >> 3) & 0x1;
    buf[TP[4]] ^= (RC[r] >> 4) & 0x1;
    buf[TP[5]] ^= (RC[r] >> 5) & 0x1;

    //Convert bit-wise variables into nibble-wise variables
    for(int i = 0; i < 32; i++){
      state[i] = buf[(4 * i)] ^ (buf[(4 * i) + 1] << 1) ^ (buf[(4 * i) + 2] << 2) ^ (buf[(4 * i) + 3] << 3);
    }

    //KeyUpdate
    Key_update(key);

    //AddRoundKey
    for(int i = 0; i < 32; i++){
      state[i] ^= key[i];
    }

  }

  //Output nibble ordering (match the test-vector convention)
  for(int i = 0; i < 16; i++){
    unsigned char t = state[i];
    state[i] = state[31 - i];
    state[31 - i] = t;
  }
}

void Baksheesh_dec(unsigned char *state, unsigned char *key){

  //Undo the output nibble ordering before the inverse round function
  for(int i = 0; i < 16; i++){
    unsigned char t = state[i];
    state[i] = state[31 - i];
    state[31 - i] = t;
  }

  //Set key
  for(int i = 0; i < ROUND; i++){
    Key_update(key);
  }

  //Round function
  for(int r = 0; r < ROUND; r++){


    //AddRoundKey
    for(int i = 0; i < 32; i++){
      state[i] ^= key[i];
    }

    Key_update_dec(key);

    //PermBits
    //Convert nibble-wise variables into bit-wise variables
    unsigned char tmp[128];
    unsigned char buf[128];
    for(int i = 0; i < 32; i++){
      for(int j = 0; j < 4; j++){
        tmp[(i * 4) + j] = (state[i] >> j) & 0x1;
      }
    }
    //AddConstants
    tmp[TP[0]] ^= RC[ROUND - 1 - r] & 0x1;
    tmp[TP[1]] ^= (RC[ROUND - 1 - r] >> 1) & 0x1;
    tmp[TP[2]] ^= (RC[ROUND - 1 - r] >> 2) & 0x1;
    tmp[TP[3]] ^= (RC[ROUND - 1 - r] >> 3) & 0x1;
    tmp[TP[4]] ^= (RC[ROUND - 1 - r] >> 4) & 0x1;
    tmp[TP[5]] ^= (RC[ROUND - 1 - r] >> 5) & 0x1;

    //Bit permutation
    for(int i = 0; i < 128; i++){
      buf[PermBitsInv[i]] = tmp[i];
    }
    //Convert bit-wise variables into nibble-wise variables
    for(int i = 0; i < 32; i++){
      state[i] = buf[(4 * i)] ^ (buf[(4 * i) + 1] << 1) ^ (buf[(4 * i) + 2] << 2) ^ (buf[(4 * i) + 3] << 3);
    }

    //SubCells
    for(int i = 0; i < 32; i++){
      state[i] = InvSBox[state[i]];
    }


  }

//  Key_update_dec(key);

  //Whitening key
  for(int i = 0; i < 32; i++){
    state[i] ^= key[i];
  }

}

void Key_update(unsigned char *key){

  //Convert nibble-wise variables into bit-wise variables
  unsigned char tmp[128];
  unsigned char buf[128];

  for(int g = 0; g < 8; g++){
    unsigned int w = ((unsigned int)key[4*g] << 12) | ((unsigned int)key[4*g+1] << 8)
                   | ((unsigned int)key[4*g+2] << 4) |  (unsigned int)key[4*g+3];
    for(int s = 0; s < 16; s++){
      tmp[(16 * g) + s] = (w >> s) & 0x1;
    }
  }

  //One-bit cyclic right rotation of the 128-bit value
  for(int i = 0; i < 127; i++){
    buf[i] = tmp[i + 1];
  }
  buf[127] = tmp[0];

  //Convert bit-wise variables into nibble-wise variables
  for(int g = 0; g < 8; g++){
    unsigned int w = 0;
    for(int s = 0; s < 16; s++){
      w |= ((unsigned int)buf[(16 * g) + s]) << s;
    }
    key[4*g]   = (w >> 12) & 0xF;
    key[4*g+1] = (w >> 8)  & 0xF;
    key[4*g+2] = (w >> 4)  & 0xF;
    key[4*g+3] =  w        & 0xF;
  }

}

void Key_update_dec(unsigned char *key){

  //Convert nibble-wise variables into bit-wise variables
  unsigned char tmp[128];
  unsigned char buf[128];

  for(int g = 0; g < 8; g++){
    unsigned int w = ((unsigned int)key[4*g] << 12) | ((unsigned int)key[4*g+1] << 8)
                   | ((unsigned int)key[4*g+2] << 4) |  (unsigned int)key[4*g+3];
    for(int s = 0; s < 16; s++){
      tmp[(16 * g) + s] = (w >> s) & 0x1;
    }
  }

  //One-bit cyclic left rotation (inverse of the encryption key update)
  for(int i = 0; i < 127; i++){
     buf[i + 1] = tmp[i];
  }
  buf[0] = tmp[127];

  //Convert bit-wise variables into nibble-wise variables
  for(int g = 0; g < 8; g++){
    unsigned int w = 0;
    for(int s = 0; s < 16; s++){
      w |= ((unsigned int)buf[(16 * g) + s]) << s;
    }
    key[4*g]   = (w >> 12) & 0xF;
    key[4*g+1] = (w >> 8)  & 0xF;
    key[4*g+2] = (w >> 4)  & 0xF;
    key[4*g+3] =  w        & 0xF;
  }

}

/***************************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************************/


void Display_state_nibble(unsigned char *state){

  for(int i = 0; i < 32; i++){
    printf("%x", state[i]);
  }
  printf("\n");

}

void Display_state_bit(unsigned char *state){

  printf(" ");
  for(int i = 0; i < 32; i++){
    for(int j = 3; j >= 0; j--){
      printf("%x", ((state[i] >> j) & 0x1));
    }
    printf(" ");
  }
  printf("\n");
}