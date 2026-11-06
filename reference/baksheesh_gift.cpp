/*
BAKSHEESH-GIFT: Reference implementation with 4-bit SBox
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

int Sbox[16] = {0x3, 0x0, 0x6, 0xD, 0xB, 0x5, 0x8, 0xE, 0xC, 0xF, 0x9, 0x2, 0x4, 0xA, 0x7, 0x1};
int SboxInv[16] = {0x1, 0xF, 0xB, 0x0, 0xC, 0x5, 0x2, 0xE, 0x6, 0xA, 0xD, 0x4, 0x8, 0x3, 0x7, 0x9};

int RoundConstant[35] = {2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31, 14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8};

int BitPermutation[128] = {96, 65, 34, 3, 64, 33, 2, 99, 32, 1, 98, 67, 0, 97, 66, 35, 100, 69, 38, 7, 68, 37, 6, 103, 36, 5, 102, 71, 4, 101, 70, 39, 104, 73, 42, 11, 72, 41, 10, 107, 40, 9, 106, 75, 8, 105, 74, 43, 108, 77, 46, 15, 76, 45, 14, 111, 44, 13, 110, 79, 12, 109, 78, 47, 112, 81, 50, 19, 80, 49, 18, 115, 48, 17, 114, 83, 16, 113, 82, 51, 116, 85, 54, 23, 84, 53, 22, 119, 52, 21, 118, 87, 20, 117, 86, 55, 120, 89, 58, 27, 88, 57, 26, 123, 56, 25, 122, 91, 24, 121, 90, 59, 124, 93, 62, 31, 92, 61, 30, 127, 60, 29, 126, 95, 28, 125, 94, 63};

void KeySchedule(int Key[8], int SubKey[36][8])
{
    for (int group = 0; group < 8; group++)
    {
        SubKey[0][group] = Key[group];
    }
    for (int round = 1; round < 36; round++)
    {
        // Convert words to bits (LSB-first ordering)
        int bits[128];
        for (int word = 0; word < 8; word++)
        {
            for (int bit = 0; bit < 16; bit++)
            {
                bits[word * 16 + bit] = (SubKey[round - 1][word] >> bit) & 1;
            }
        }
        
        // One-bit cyclic right rotation of the value; the array is LSB-first, so the array shifts left
        int buf[128];
        for (int i = 0; i < 127; i++)
        {
            buf[i] = bits[i + 1];
        }
        buf[127] = bits[0];
        
        // Convert bits back to words (LSB-first ordering)
        for (int word = 0; word < 8; word++)
        {
            SubKey[round][word] = 0;
            for (int bit = 0; bit < 16; bit++)
            {
                SubKey[round][word] |= (buf[word * 16 + bit] << bit);
            }
        }
    }
}

void BAKSHEESH_Enc(int Plaintext[8], int SubKey[36][8], int Ciphertext[8])
{
    int X[8];
    for (int group = 0; group < 8; group++)
    {
        X[group] = Plaintext[group];
    }
    for (int round = 0; round < 35; round++)
    {
        for (int group = 0; group < 8; group++)
        {
            X[group] ^= SubKey[round][group];
        }
        int Y[32];
        for (int group = 0; group < 8; group++)
        {
            for (int sbox = 0; sbox < 4; sbox++)
            {
                Y[4 * group + sbox] = Sbox[(X[group] >> (4 * (3 - sbox))) & (0xf)];
            }
        }
        
        int InState[128];
        for (int sbox = 0; sbox < 32; sbox++)
        {
            for (int bit = 0; bit < 4; bit++)
            {
                InState[4 * sbox + bit] = ((Y[sbox] >> (3 - bit)) & (0x1));
            }
        }
        
        int OutState[128];
        for (int bit = 0; bit < 128; bit++)
        {
            OutState[BitPermutation[bit]] = InState[bit];
        }
        
        for (int sbox = 0; sbox < 32; sbox++)
        {
            Y[sbox] = 0;
            for (int bit = 0; bit < 4; bit++)
            {
                Y[sbox] ^= (OutState[4 * sbox + bit] << (3 - bit));
            }
        }
        
        OutState[11] ^= ((RoundConstant[round] >> 0) & (0x1));
        OutState[14] ^= ((RoundConstant[round] >> 1) & (0x1));
        OutState[16] ^= ((RoundConstant[round] >> 2) & (0x1));
        OutState[32] ^= ((RoundConstant[round] >> 3) & (0x1));
        OutState[64] ^= ((RoundConstant[round] >> 4) & (0x1));
        OutState[105] ^= ((RoundConstant[round] >> 5) & (0x1));
        
        for (int group = 0; group < 8; group++)
        {
            X[group] = 0;
            for (int bit = 0; bit < 16; bit++)
            {
                X[group] ^= (OutState[16 * group + bit] << (15 - bit));
            }
        }
    }
    
    for (int group = 0; group < 8; group++)
    {
        X[group] ^= SubKey[35][group];
    }
    
    for (int group = 0; group < 8; group++)
    {
        Ciphertext[group] = 0;
        for (int sbox = 0; sbox < 4; sbox++)
        {
            Ciphertext[group] ^= (((X[7 - group] >> (4 * sbox)) & (0xf)) << (4 * (3 - sbox)));
        }
    }
}

// Decryption using 4-bit S-box
void BAKSHEESH_Dec(int Ciphertext[8], int SubKey[36][8], int Plaintext[8]) {
    int X[8];
    
    // Reverse final bit reversal (exact inverse of encryption's post-process)
    for (int group = 0; group < 8; group++) {
        X[7 - group] = 0;
        for (int sbox = 0; sbox < 4; sbox++) {
            X[7 - group] ^= (((Ciphertext[group] >> (4 * (3 - sbox))) & (0xf)) << (4 * sbox));
        }
    }
    
    // Remove final round key
    for (int group = 0; group < 8; group++) {
        X[group] ^= SubKey[35][group];
    }
    
    // Decrypt 35 rounds in reverse
    for (int round = 34; round >= 0; round--) {
        // Convert to bits
        int OutState[128];
        for (int group = 0; group < 8; group++) {
            for (int bit = 0; bit < 16; bit++) {
                OutState[16 * group + bit] = (X[group] >> (15 - bit)) & (0x1);
            }
        }
        
        // Remove round constants
        OutState[11] ^= ((RoundConstant[round] >> 0) & (0x1));
        OutState[14] ^= ((RoundConstant[round] >> 1) & (0x1));
        OutState[16] ^= ((RoundConstant[round] >> 2) & (0x1));
        OutState[32] ^= ((RoundConstant[round] >> 3) & (0x1));
        OutState[64] ^= ((RoundConstant[round] >> 4) & (0x1));
        OutState[105] ^= ((RoundConstant[round] >> 5) & (0x1));
        
        // Apply inverse bit permutation (same as forward)
        int InState[128];
        for (int bit = 0; bit < 128; bit++) {
            InState[bit] = OutState[BitPermutation[bit]];
        }
        
        // Convert to nibbles
        int Y[32];
        for (int sbox = 0; sbox < 32; sbox++) {
            Y[sbox] = 0;
            for (int bit = 0; bit < 4; bit++) {
                Y[sbox] ^= (InState[4 * sbox + bit] << (3 - bit));
            }
        }
        
        // Apply inverse S-box
        for (int sbox = 0; sbox < 32; sbox++) {
            Y[sbox] = SboxInv[Y[sbox]];
        }
        
        // Reconstruct state
        for (int group = 0; group < 8; group++) {
            X[group] = 0;
            for (int sbox = 0; sbox < 4; sbox++) {
                X[group] ^= (Y[4 * group + sbox] << (4 * (3 - sbox)));
            }
        }
        
        // Remove round key
        for (int group = 0; group < 8; group++) {
            X[group] ^= SubKey[round][group];
        }
    }
    
    // Copy result
    for (int group = 0; group < 8; group++) {
        Plaintext[group] = X[group];
    }
}

// Helper function to parse hex string to nibble array
void hex_to_nibbles(const char* hex_str, int nibbles[32]) {
    int len = strlen(hex_str);
    for (int i = 0; i < 32 && i < len; i++) {
        char c = hex_str[i];
        if (c >= '0' && c <= '9') nibbles[i] = c - '0';
        else if (c >= 'a' && c <= 'f') nibbles[i] = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') nibbles[i] = c - 'A' + 10;
        else nibbles[i] = 0;
    }
}

// Helper function to convert nibbles to 16-bit groups
// nibbles[0..3] -> groups[0] with nibbles[0] as MSB
void nibbles_to_groups(int nibbles[32], int groups[8]) {
    for (int g = 0; g < 8; g++) {
        groups[g] = (nibbles[g * 4] << 12) | (nibbles[g * 4 + 1] << 8) | 
                    (nibbles[g * 4 + 2] << 4) | nibbles[g * 4 + 3];
    }
}

// Helper function to convert 16-bit groups to nibbles
void groups_to_nibbles(int groups[8], int nibbles[32]) {
    for (int g = 0; g < 8; g++) {
        for (int n = 0; n < 4; n++) {
            nibbles[g * 4 + n] = (groups[g] >> (4 * (3 - n))) & 0xF;
        }
    }
}

// Test vectors from designR.tex (baksheesh8.py)
struct TestVector {
    const char* key;
    const char* plaintext;
    const char* ciphertext;
};

TestVector test_vectors[10] = {
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

int main()
{
    cout << "BAKSHEESH-GIFT Test Vector Verification" << endl;
    cout << "========================================" << endl;
    
    int passed = 0;
    int failed = 0;
    
    for (int t = 0; t < 10; t++) {
        // Parse hex strings to nibbles
        int key_nibbles[32], pt_nibbles[32], expected_ct_nibbles[32];
        hex_to_nibbles(test_vectors[t].key, key_nibbles);
        hex_to_nibbles(test_vectors[t].plaintext, pt_nibbles);
        hex_to_nibbles(test_vectors[t].ciphertext, expected_ct_nibbles);
        
        // Convert to 16-bit groups
        int Key[8], Plaintext[8], ExpectedCiphertext[8];
        nibbles_to_groups(key_nibbles, Key);
        nibbles_to_groups(pt_nibbles, Plaintext);
        nibbles_to_groups(expected_ct_nibbles, ExpectedCiphertext);
        
        // Encrypt
        int SubKey[36][8];
        int Ciphertext[8];
        KeySchedule(Key, SubKey);
        BAKSHEESH_Enc(Plaintext, SubKey, Ciphertext);
        
        // Verify encryption
        bool enc_match = true;
        for (int i = 0; i < 8; i++) {
            if (Ciphertext[i] != ExpectedCiphertext[i]) {
                enc_match = false;
                break;
            }
        }
        
        // Test decryption
        int Decrypted[8];
        BAKSHEESH_Dec(Ciphertext, SubKey, Decrypted);
        
        bool dec_match = true;
        for (int i = 0; i < 8; i++) {
            if (Decrypted[i] != Plaintext[i]) {
                dec_match = false;
                break;
            }
        }
        
        if (enc_match && dec_match) {
            cout << "Test " << (t + 1) << ": PASSED (Enc + Dec)" << endl;
            passed++;
        } else {
            cout << "Test " << (t + 1) << ": ";
            if (enc_match) {
                cout << "PASSED Encryption";
            } else {
                cout << "FAILED Encryption";
            }
            if (!dec_match) {
                cout << " | FAILED Decryption";
            }
            cout << endl;
            
            if (!enc_match) {
                cout << "  Key:      " << test_vectors[t].key << endl;
                cout << "  PT:       " << test_vectors[t].plaintext << endl;
                cout << "  Expected: " << test_vectors[t].ciphertext << endl;
                cout << "  Got:      ";
                for (int i = 0; i < 8; i++) {
                    printf("%04x", Ciphertext[i]);
                }
                cout << endl;
            }
            if (!dec_match) {
                cout << "  Decrypted: ";
                for (int i = 0; i < 8; i++) {
                    printf("%04x ", Decrypted[i]);
                }
                cout << endl;
                cout << "  Expected:  ";
                for (int i = 0; i < 8; i++) {
                    printf("%04x ", Plaintext[i]);
                }
                cout << endl;
            }
            failed++;
        }
    }
    
    cout << endl << "Summary: " << passed << " passed, " << failed << " failed" << endl;
    
    if (failed == 0) {
        cout << "ALL TESTS PASSED - Baksheesh implementation verified" << endl;
    }
    
    return (failed == 0) ? 0 : 1;
}
