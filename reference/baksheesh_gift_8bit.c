
#include <stdio.h>
#include <stdint.h>

#define ROUND 35

//4-bit SBox and its inverse
const unsigned char SBox[16] = {3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1};
const unsigned char InvSBox[16] = {1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9};

//8-bit SBox and its inverse (the 4-bit SBox applied to each nibble of a byte)
unsigned char SBox8[256];
unsigned char InvSBox8[256];

//Bit permutation
const unsigned char PermBits[128] = {0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3, 4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7, 8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11, 12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15, 16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19, 20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23, 24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27, 28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31};
const unsigned char PermBitsInv[128] = {0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63, 64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127, 12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59, 76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123, 8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55, 72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119, 4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51, 68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115};

//Round constants
const unsigned char RC[ROUND] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};
//Tap positions for round constants
const unsigned char TP[6] = {8, 13, 19, 35, 67, 106};

void Baksheesh_enc(uint8_t *state, uint8_t *key);
void Key_update(uint8_t *key);
void Baksheesh_dec(uint8_t *state, uint8_t *key);
void Key_update_dec(uint8_t *key);

void Init_sbox8(void);
void Hex_to_byte(const char *hex, uint8_t *out);
void Display_state_byte(uint8_t *state);

/***************************************************************************************************************************************************************************************************************************************************/

//The state is 16 bytes; byte b holds nibbles 2b (high) and 2b+1 (low)

int main(void){

  Init_sbox8();

  //Known-answer tests (key, plaintext, ciphertext), including asymmetric inputs
  const char *kat[][3] = {
    {"00000000000000000000000000000000", "00000000000000000000000000000000", "c002be5e64c78a72ab9a3439518352aa"},
    {"00000000000000000000000000000000", "00000000000000000000000000000007", "1ba3363734c09a29f67c23bbb2cccc05"},
    {"00000000000000000000000000000000", "70000000000000000000000000000000", "6f7d7746eaf0d97a154079f6bd846438"},
    {"00000000000000000000000000000000", "44444444444444444444444444444444", "7ad3303667b2af6deef434dd110d7fb8"},
    {"ffffffffffffffffffffffffffffffff", "11111111111111111111111111111111", "806f0cf45b94f0370206975fe78ac10f"},
    {"d9bb750877be6eebc339a2258c874fc9", "3325cfadce142135242f8e04752614ab", "2b599d496ee6938cec0bae00c4947b7d"},
  };
  int n = (int)(sizeof(kat) / sizeof(kat[0]));
  int pass = 0;

  for(int t = 0; t < n; t++){
    uint8_t K[16], P[16], C[16], key[16], state[16];
    Hex_to_byte(kat[t][0], K);
    Hex_to_byte(kat[t][1], P);
    Hex_to_byte(kat[t][2], C);

    for(int i = 0; i < 16; i++){ state[i] = P[i]; key[i] = K[i]; }
    Baksheesh_enc(state, key);
    int enc_ok = 1;
    for(int i = 0; i < 16; i++){ if(state[i] != C[i]) enc_ok = 0; }

    printf("Test %d (encryption %s)\n", t + 1, enc_ok ? "PASS" : "FAIL");
    printf("Key       : "); Display_state_byte(K);
    printf("Plaintext : "); Display_state_byte(P);
    printf("Ciphertext: "); Display_state_byte(state);

    for(int i = 0; i < 16; i++){ key[i] = K[i]; }
    Baksheesh_dec(state, key);
    int dec_ok = 1;
    for(int i = 0; i < 16; i++){ if(state[i] != P[i]) dec_ok = 0; }
    printf("Decryption: %s\n\n", dec_ok ? "PASS" : "FAIL");

    if(enc_ok && dec_ok){ pass++; }
  }

  printf("Result: %d/%d passed\n", pass, n);

  return (pass == n) ? 0 : 1;
}

/***************************************************************************************************************************************************************************************************************************************************/

//Swap the two nibbles of a byte
static uint8_t Swap_nibble(uint8_t x){
  return (uint8_t)((x >> 4) | ((x & 0xF) << 4));
}

//Reverse the 32-nibble ordering held in the 16 bytes
static void Reverse_state(uint8_t *state){
  uint8_t t[16];
  for(int b = 0; b < 16; b++){
    t[b] = Swap_nibble(state[15 - b]);
  }
  for(int b = 0; b < 16; b++){
    state[b] = t[b];
  }
}

//Extract the 128 bits from the byte state (nibble 2b -> tmp[8b..8b+3], nibble 2b+1 -> tmp[8b+4..8b+7])
static void State_to_bits(const uint8_t *state, uint8_t *tmp){
  for(int b = 0; b < 16; b++){
    uint8_t nhi = state[b] >> 4;
    uint8_t nlo = state[b] & 0xF;
    for(int j = 0; j < 4; j++){
      tmp[(8 * b) + j] = (nhi >> j) & 0x1;
      tmp[(8 * b) + 4 + j] = (nlo >> j) & 0x1;
    }
  }
}

//Pack the 128 bits back into the byte state
static void Bits_to_state(const uint8_t *buf, uint8_t *state){
  for(int b = 0; b < 16; b++){
    uint8_t nhi = buf[8 * b] ^ (buf[(8 * b) + 1] << 1) ^ (buf[(8 * b) + 2] << 2) ^ (buf[(8 * b) + 3] << 3);
    uint8_t nlo = buf[(8 * b) + 4] ^ (buf[(8 * b) + 5] << 1) ^ (buf[(8 * b) + 6] << 2) ^ (buf[(8 * b) + 7] << 3);
    state[b] = (uint8_t)((nhi << 4) | nlo);
  }
}

void Baksheesh_enc(uint8_t *state, uint8_t *key){

  //Whitening key
  for(int i = 0; i < 16; i++){
    state[i] ^= key[i];
  }

  //Round function
  for(int r = 0; r < ROUND; r++){

    //SubCells
    for(int i = 0; i < 16; i++){
      state[i] = SBox8[state[i]];
    }

    //PermBits
    uint8_t tmp[128];
    uint8_t buf[128];
    State_to_bits(state, tmp);
    for(int i = 0; i < 128; i++){
      buf[PermBits[i]] = tmp[i];
    }

    //AddConstants
    for(int t = 0; t < 6; t++){
      buf[TP[t]] ^= (RC[r] >> t) & 0x1;
    }

    Bits_to_state(buf, state);

    //KeyUpdate
    Key_update(key);

    //AddRoundKey
    for(int i = 0; i < 16; i++){
      state[i] ^= key[i];
    }

  }

  //Output nibble ordering (match the test-vector convention)
  Reverse_state(state);
}

void Baksheesh_dec(uint8_t *state, uint8_t *key){

  //Undo the output nibble ordering before the inverse round function
  Reverse_state(state);

  //Set key
  for(int i = 0; i < ROUND; i++){
    Key_update(key);
  }

  //Round function
  for(int r = 0; r < ROUND; r++){

    //AddRoundKey
    for(int i = 0; i < 16; i++){
      state[i] ^= key[i];
    }

    Key_update_dec(key);

    //PermBits
    uint8_t tmp[128];
    uint8_t buf[128];
    State_to_bits(state, tmp);

    //AddConstants
    for(int t = 0; t < 6; t++){
      tmp[TP[t]] ^= (RC[ROUND - 1 - r] >> t) & 0x1;
    }

    for(int i = 0; i < 128; i++){
      buf[PermBitsInv[i]] = tmp[i];
    }
    Bits_to_state(buf, state);

    //SubCells
    for(int i = 0; i < 16; i++){
      state[i] = InvSBox8[state[i]];
    }

  }

  //Whitening key
  for(int i = 0; i < 16; i++){
    state[i] ^= key[i];
  }

}

void Key_update(uint8_t *key){

  uint8_t tmp[128];
  uint8_t buf[128];

  //Pack every two bytes into a 16-bit word, then read it least significant bit first
  for(int g = 0; g < 8; g++){
    unsigned int w = ((unsigned int)key[2 * g] << 8) | (unsigned int)key[(2 * g) + 1];
    for(int s = 0; s < 16; s++){
      tmp[(16 * g) + s] = (w >> s) & 0x1;
    }
  }

  //One-bit cyclic right rotation of the 128-bit value
  for(int i = 0; i < 127; i++){
    buf[i] = tmp[i + 1];
  }
  buf[127] = tmp[0];

  for(int g = 0; g < 8; g++){
    unsigned int w = 0;
    for(int s = 0; s < 16; s++){
      w |= ((unsigned int)buf[(16 * g) + s]) << s;
    }
    key[2 * g] = (w >> 8) & 0xFF;
    key[(2 * g) + 1] = w & 0xFF;
  }

}

void Key_update_dec(uint8_t *key){

  uint8_t tmp[128];
  uint8_t buf[128];

  for(int g = 0; g < 8; g++){
    unsigned int w = ((unsigned int)key[2 * g] << 8) | (unsigned int)key[(2 * g) + 1];
    for(int s = 0; s < 16; s++){
      tmp[(16 * g) + s] = (w >> s) & 0x1;
    }
  }

  //One-bit cyclic left rotation (inverse of the encryption key update)
  for(int i = 0; i < 127; i++){
    buf[i + 1] = tmp[i];
  }
  buf[0] = tmp[127];

  for(int g = 0; g < 8; g++){
    unsigned int w = 0;
    for(int s = 0; s < 16; s++){
      w |= ((unsigned int)buf[(16 * g) + s]) << s;
    }
    key[2 * g] = (w >> 8) & 0xFF;
    key[(2 * g) + 1] = w & 0xFF;
  }

}

/***************************************************************************************************************************************************************************************************************************************************/

//Build the 8-bit SBox tables from the 4-bit SBox
void Init_sbox8(void){
  for(int x = 0; x < 256; x++){
    SBox8[x] = (uint8_t)((SBox[x >> 4] << 4) | SBox[x & 0xF]);
    InvSBox8[x] = (uint8_t)((InvSBox[x >> 4] << 4) | InvSBox[x & 0xF]);
  }
}

//Convert a 32-character hex string into 16 bytes (leftmost character first, high nibble of byte 0)
void Hex_to_byte(const char *hex, uint8_t *out){
  for(int b = 0; b < 16; b++){
    char hi = hex[2 * b];
    char lo = hex[(2 * b) + 1];
    uint8_t nhi = (hi <= '9') ? (hi - '0') : ((hi | 0x20) - 'a' + 10);
    uint8_t nlo = (lo <= '9') ? (lo - '0') : ((lo | 0x20) - 'a' + 10);
    out[b] = (uint8_t)((nhi << 4) | nlo);
  }
}

void Display_state_byte(uint8_t *state){
  for(int b = 0; b < 16; b++){
    printf("%02x", state[b]);
  }
  printf("\n");
}
