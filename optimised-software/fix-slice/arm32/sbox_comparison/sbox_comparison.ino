#pragma GCC optimize ("-O3")
#include <stdio.h>
#include <string.h>

#include "sbox.h"

void unpack_print(uint32_t output[4]) {
    unsigned int data2[32] = {0};

    for (int i = 0; i < 32; i++) {
        for (int n = 0; n < 4; n++) {
            data2[i] |= ((output[n] >> i) & 1) << n;
        }
    }

    for (int i = 0; i < 32; i++) {
        Serial.print(data2[i], HEX);
        Serial.print(" ");
        
    }
    Serial.print("\n");
}

void setup() {
Serial.begin(9600);
    unsigned int data[32] = {
        0, 1, 2, 3, 4, 5, 6, 7, 
        8, 9, 10, 11, 12, 13, 14, 15, 
        0, 1, 2, 3, 4, 5, 6, 7, 
        8, 9, 10, 11, 12, 13, 14, 15,
    };

    //pack
    uint32_t input[4] = {0}; 
    for (int i = 0; i < 32; i++) {
        for (int n = 0; n < 4; n++) {
            input[n] |= ((data[i] >> n) & 1) << i;
        }
    }

    
    uint32_t output[4] = {0x00,0x00,0x00,0x00};
    
    //3 0 6 D B 5 8 E C F 9 2 4 A 7 1 3 0 6 D B 5 8 E C F 9 2 4 A 7 1 
    test_baksheesh(input, output);
    unpack_print(output);
    
    //1 A 4 C 6 F 3 9 2 D B 7 5 0 8 E 1 A 4 C 6 F 3 9 2 D B 7 5 0 8 E
    test_gift(input, output);
    unpack_print(output);

    //E 4 B 2 3 8 0 9 1 A 7 F 6 C 5 D E 4 B 2 3 8 0 9 1 A 7 F 6 C 5 D  
    test_piccolo(input, output);
    unpack_print(output);

    //C 5 6 B 9 0 A D 3 E F 8 4 7 1 2 C 5 6 B 9 0 A D 3 E F 8 4 7 1 2 
    test_present(input, output);
    unpack_print(output);
    
    //2 D 3 9 7 B A 6 E 0 F 4 8 5 1 C 2 D 3 9 7 B A 6 E 0 F 4 8 5 1 C 
    test_pyjamask128(input, output);
    
    unpack_print(output);
    
    
}
void loop() {}
