"""
BAKSHEESH Zorro with 128×128 Binary T-matrix

Architecture:
1. Apply 3-bit S-box to all 32 nibbles (lower 3 bits of each)
2. Combine with x3 bits using 128×128 binary T-matrix (linear transformation)

"""

from typing import List, Sequence

# 3-bit S-box (nonlinear, secret)
SBOX_3BIT = [1, 0, 3, 6, 5, 2, 4, 7]

# 128×128 Binary T-matrix (linear transformation over GF(2))
# This matrix combines outputs from 32× 3-bit S-boxes (96 bits) + 32× x3 bits (32 bits) → 128 output bits
# The matrix is block-diagonal with 32 copies of the 4×4 sub-matrix
TMATRIX_4x4 = [
    [1, 1, 0, 0],  # y3 = x3 ⊕ z2
    [1, 0, 1, 0],  # y2 = x3 ⊕ z1
    [1, 0, 0, 1],  # y1 = x3 ⊕ z0
    [1, 0, 1, 1],  # y0 = x3 ⊕ z1 ⊕ z0
]

# Bit-permutation (Canonical BAKSHEESH Zorro permutation)
player = (
    96, 65, 34, 3, 64, 33, 2, 99, 32, 1, 98, 67, 0, 97, 66, 35,
    100, 69, 38, 7, 68, 37, 6, 103, 36, 5, 102, 71, 4, 101, 70, 39,
    104, 73, 42, 11, 72, 41, 10, 107, 40, 9, 106, 75, 8, 105, 74, 43,
    108, 77, 46, 15, 76, 45, 14, 111, 44, 13, 110, 79, 12, 109, 78, 47,
    112, 81, 50, 19, 80, 49, 18, 115, 48, 17, 114, 83, 16, 113, 82, 51,
    116, 85, 54, 23, 84, 53, 22, 119, 52, 21, 118, 87, 20, 117, 86, 55,
    120, 89, 58, 27, 88, 57, 26, 123, 56, 25, 122, 91, 24, 121, 90, 59,
    124, 93, 62, 31, 92, 61, 30, 127, 60, 29, 126, 95, 28, 125, 94, 63,
)

# Round constants
RC = (
    2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31,
    14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8,
)
const_loc = (11, 14, 16, 32, 64, 105)

N_rounds = 35


def apply_sbox_layer(state_nibbles: Sequence[int]) -> List[int]:
    """
    Apply 3-bit S-box layer to all 32 nibbles, then combine with T-matrix.
    
    This implementation applies the 128×128 T-matrix to the entire state:
    - Extract x3 bits from all 32 nibbles (32 bits)
    - Apply 3-bit S-box to lower 3 bits of all 32 nibbles (96 bits output)
    - Combine with 128×128 T-matrix: [x3_0, z2_0, z1_0, z0_0, x3_1, z2_1, ..., z0_31]
    """
    output_nibbles = []
    
    for nibble in state_nibbles:
        # Extract bits
        x0 = (nibble >> 0) & 1
        x1 = (nibble >> 1) & 1
        x2 = (nibble >> 2) & 1
        x3 = (nibble >> 3) & 1
        
        # Stage 1: Apply 3-bit S-box to lower 3 bits
        inp_3bit = (x2 << 2) | (x1 << 1) | x0
        s3_out = SBOX_3BIT[inp_3bit]
        
        z0 = (s3_out >> 0) & 1
        z1 = (s3_out >> 1) & 1
        z2 = (s3_out >> 2) & 1
        
        # Stage 2: Apply 4×4 T-matrix block
        # Input: [x3, z2, z1, z0]
        # Output: [y3, y2, y1, y0]
        input_vec = [x3, z2, z1, z0]
        output_bits = []
        for row in TMATRIX_4x4:
            bit = sum(row[i] * input_vec[i] for i in range(4)) % 2
            output_bits.append(bit)
        
        # Convert to nibble
        output_nibble = (output_bits[0] << 3) | (output_bits[1] << 2) | \
                       (output_bits[2] << 1) | output_bits[3]
        output_nibbles.append(output_nibble)
    
    return output_nibbles


def apply_sbox_layer_explicit_128x128(state_nibbles: Sequence[int]) -> List[int]:
    """
    Alternative implementation showing explicit 128×128 matrix structure.
    
    The 128×128 T-matrix is block-diagonal:
    T = diag(T_4x4, T_4x4, ..., T_4x4)  [32 blocks]
    
    Input vector layout (128 bits):
      [x3_0, z2_0, z1_0, z0_0, x3_1, z2_1, z1_1, z0_1, ..., x3_31, z2_31, z1_31, z0_31]
    
    Output vector layout (128 bits):
      [y3_0, y2_0, y1_0, y0_0, y3_1, y2_1, y1_1, y0_1, ..., y3_31, y2_31, y1_31, y0_31]
    
    """
    # Build 128-bit input vector
    input_bits = []
    for nibble in state_nibbles:
        x0 = (nibble >> 0) & 1
        x1 = (nibble >> 1) & 1
        x2 = (nibble >> 2) & 1
        x3 = (nibble >> 3) & 1
        
        # Apply 3-bit S-box
        inp_3bit = (x2 << 2) | (x1 << 1) | x0
        s3_out = SBOX_3BIT[inp_3bit]
        
        z0 = (s3_out >> 0) & 1
        z1 = (s3_out >> 1) & 1
        z2 = (s3_out >> 2) & 1
        
        # Add to input vector: [x3, z2, z1, z0] for this nibble
        input_bits.extend([x3, z2, z1, z0])
    
    # Apply 128×128 T-matrix (block-diagonal)
    output_bits = [0] * 128
    for block_idx in range(32):
        # Process 4 bits at a time (one nibble)
        base = block_idx * 4
        input_vec = input_bits[base:base+4]
        
        # Apply 4×4 matrix to this block
        for out_bit in range(4):
            bit_sum = sum(TMATRIX_4x4[out_bit][i] * input_vec[i] for i in range(4))
            output_bits[base + out_bit] = bit_sum % 2
    
    # Convert output bits back to nibbles
    output_nibbles = []
    for i in range(32):
        base = i * 4
        nibble = (output_bits[base] << 3) | (output_bits[base+1] << 2) | \
                 (output_bits[base+2] << 1) | output_bits[base+3]
        output_nibbles.append(nibble)
    
    return output_nibbles


def nibbles_to_bits(x: Sequence[int]) -> List[int]:
    bits: List[int] = []
    for nib in x:
        bits.extend(((nib >> shift) & 1) for shift in (3, 2, 1, 0))
    return bits


def bits_to_nibbles(bits: Sequence[int]) -> List[int]:
    if len(bits) % 4:
        raise ValueError("Bit length must be a multiple of four")
    out: List[int] = []
    for i in range(0, len(bits), 4):
        nib = 0
        for bit in bits[i : i + 4]:
            nib = (nib << 1) | bit
        out.append(nib)
    return out


def perm_layer(bits: Sequence[int], player_map: Sequence[int]) -> List[int]:
    if len(bits) != len(player_map):
        raise ValueError("Bit permutation length mismatch")
    out = [0] * len(bits)
    for src, dst in enumerate(player_map):
        out[dst] = bits[src]
    return out


def nibbles_to_words(nibbles: Sequence[int]) -> List[int]:
    if len(nibbles) % 4:
        raise ValueError("Nibble length must be a multiple of four")
    words: List[int] = []
    for i in range(0, len(nibbles), 4):
        word = 0
        for shift, nib in zip((12, 8, 4, 0), nibbles[i : i + 4]):
            word |= (nib & 0xF) << shift
        words.append(word)
    return words


def words_to_nibbles(words: Sequence[int]) -> List[int]:
    nibbles: List[int] = []
    for word in words:
        for shift in (12, 8, 4, 0):
            nibbles.append((word >> shift) & 0xF)
    return nibbles


def words_to_bits(words: Sequence[int]) -> List[int]:
    bits: List[int] = []
    for word in words:
        for shift in range(15, -1, -1):
            bits.append((word >> shift) & 1)
    return bits


def words_to_bits_lsb(words: Sequence[int]) -> List[int]:
    bits: List[int] = []
    for word in words:
        for shift in range(16):
            bits.append((word >> shift) & 1)
    return bits


def bits_to_words(bits: Sequence[int]) -> List[int]:
    if len(bits) != 128:
        raise ValueError("Word conversion expects exactly 128 bits")
    words: List[int] = []
    for group in range(8):
        base = 16 * group
        word = 0
        for offset in range(16):
            word = (word << 1) | bits[base + offset]
        words.append(word)
    return words


def bits_to_words_lsb(bits: Sequence[int]) -> List[int]:
    if len(bits) != 128:
        raise ValueError("Word conversion expects exactly 128 bits")
    words: List[int] = []
    for group in range(8):
        base = 16 * group
        word = 0
        for offset in range(16):
            word |= bits[base + offset] << offset
        words.append(word)
    return words


def xor_words(a: Sequence[int], b: Sequence[int]) -> List[int]:
    return [x ^ y for x, y in zip(a, b)]


def add_round_constants_bits(bits: List[int], rc: int) -> None:
    for i, pos in enumerate(const_loc):
        bits[pos] ^= (rc >> i) & 1


def post_process_words(words: Sequence[int]) -> List[int]:
    out: List[int] = []
    for group in range(8):
        src = words[7 - group]
        word = 0
        for sbox in range(4):
            nib = (src >> (4 * sbox)) & 0xF
            word |= nib << (4 * (3 - sbox))
        out.append(word)
    return out


def expand_subkeys(key: Sequence[int], rounds: int = N_rounds) -> List[List[int]]:
    if rounds > len(RC):
        raise ValueError(f"Requested {rounds} rounds but only {len(RC)} round constants")
    current = nibbles_to_words(key)
    subkeys = [current[:]]
    for _ in range(rounds):
        bits = words_to_bits_lsb(current)
        bits = bits[1:] + bits[:1]  # Rotate left by 1
        current = bits_to_words_lsb(bits)
        subkeys.append(current[:])
    return subkeys


def encrypt(key: Sequence[int], pt: Sequence[int], no_of_rounds: int = N_rounds, 
            use_explicit: bool = False) -> List[int]:
    """
    Encrypt plaintext with BAKSHEESH Zorro using 128×128 T-matrix.
    
    Args:
        key: 32 nibbles (128-bit key)
        pt: 32 nibbles (128-bit plaintext)
        no_of_rounds: Number of rounds (default 35)
        use_explicit: If True, use explicit 128×128 matrix formulation
    
    Returns:
        32 nibbles (128-bit ciphertext)
    """
    if len(key) != 32:
        raise ValueError("Key must be 32 nibbles")
    if len(pt) != 32:
        raise ValueError("Plaintext must be 32 nibbles")
    if no_of_rounds > len(RC):
        raise ValueError("Number of rounds exceeds available round constants")

    subkeys = expand_subkeys(key, no_of_rounds)
    state_words = nibbles_to_words(pt)

    sbox_fn = apply_sbox_layer_explicit_128x128 if use_explicit else apply_sbox_layer

    for rnd in range(no_of_rounds):
        state_words = xor_words(state_words, subkeys[rnd])
        state_nibbles = words_to_nibbles(state_words)
        state_nibbles = sbox_fn(state_nibbles)
        bits = nibbles_to_bits(state_nibbles)
        bits = perm_layer(bits, player)
        add_round_constants_bits(bits, RC[rnd])
        state_words = bits_to_words(bits)

    state_words = xor_words(state_words, subkeys[no_of_rounds])
    cipher_words = post_process_words(state_words)
    return words_to_nibbles(cipher_words)


def pretty_print(x: Sequence[int]) -> str:
    return ''.join(f"{xx:x}" for xx in x)


def hex_to_nibbles(hex_str: str) -> List[int]:
    """Convert hex string to nibble list (character by character)."""
    if len(hex_str) != 32:
        raise ValueError("Hex string must be 32 characters (128 bits)")
    return [int(char, 16) for char in hex_str]


if __name__ == '__main__':
    print("=" * 70)
    
    # Test vectors from designR.tex
    test_vectors = [
        {
            'plaintext': '00000000000000000000000000000000',
            'key': '00000000000000000000000000000000',
            'expected': 'c002be5e64c78a72ab9a3439518352aa'
        },
        {
            'plaintext': '00000000000000000000000000000007',
            'key': '00000000000000000000000000000000',
            'expected': '1ba3363734c09a29f67c23bbb2cccc05'
        },
        {
            'plaintext': '70000000000000000000000000000000',
            'key': '00000000000000000000000000000000',
            'expected': '6f7d7746eaf0d97a154079f6bd846438'
        },
        {
            'plaintext': '44444444444444444444444444444444',
            'key': '00000000000000000000000000000000',
            'expected': '7ad3303667b2af6deef434dd110d7fb8'
        },
        {
            'plaintext': '11111111111111111111111111111111',
            'key': 'ffffffffffffffffffffffffffffffff',
            'expected': '806f0cf45b94f0370206975fe78ac10f'
        },
        {
            'plaintext': '789a789a789a789a789a789a789a789a',
            'key': '76543210032032032032032032032032',
            'expected': 'f568e4e628f723ee6d5fbacccf811523'
        },
        {
            'plaintext': 'b6e4789ab6e4789ab6e4789ab6e4789a',
            'key': '23023023023023023023023001234567',
            'expected': '77acdeed645e49da25591bf5f32658a2'
        },
        {
            'plaintext': 'e6517531abf63f3d7805e126943a081c',
            'key': '5920effb52bc61e33a98425321e76915',
            'expected': 'ea5e0713249400a86ec61bb6cdffb364'
        },
        {
            'plaintext': '3325cfadce142135242f8e04752614ab',
            'key': 'd9bb750877be6eebc339a2258c874fc9',
            'expected': '2b599d496ee6938cec0bae00c4947b7d'
        },
        {
            'plaintext': '57cf5777053928343047ebe8dbfc374f',
            'key': '25798b6c2b24cd83281de76786213b86',
            'expected': '62b9d55d799a3e784583e7377229a623'
        },
    ]
    
    # Test both implementations
    for impl_name, use_explicit in [("Block-wise", False), ("Explicit 128×128", True)]:
        print("=" * 70)
        print(f"Testing: {impl_name} Implementation")
        print("=" * 70)
        
        passed = 0
        for i, tv in enumerate(test_vectors):
            pt_nibbles = hex_to_nibbles(tv['plaintext'])
            key_nibbles = hex_to_nibbles(tv['key'])
            
            ct_nibbles = encrypt(key_nibbles, pt_nibbles, use_explicit=use_explicit)
            result_hex = pretty_print(ct_nibbles)
            
            match = result_hex == tv['expected']
            if match:
                passed += 1
            
            if i == 0 or not match:  # Show first test and any failures
                status = "✅ PASS" if match else "❌ FAIL"
                print(f"\nTest {i + 1}: {status}")
                print(f"  Expected: {tv['expected']}")
                print(f"  Got:      {result_hex}")
        
        print(f"\nResult: {passed}/{len(test_vectors)} tests passed")
        
        if passed == len(test_vectors):
            print("✅ All tests passed!")
        
        print()
