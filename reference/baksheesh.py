"""
BAKSHEESH Reference Implementation (Canonical Permutation)

This is the canonical BAKSHEESH cipher implementation using the C++-matching
bit permutation. All test vectors are from designR.tex specification.
"""

from __future__ import annotations

from typing import Iterable, List, Sequence

# S-box (GIFT-heritage): 306DB58ECF924A71
LS_sbox = (3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1)
LS_inv_sbox = (1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9)

# Canonical bit-permutation (C++-matching)
PLAYER = (
    96, 65, 34, 3, 64, 33, 2, 99, 32, 1, 98, 67, 0, 97, 66, 35,
    100, 69, 38, 7, 68, 37, 6, 103, 36, 5, 102, 71, 4, 101, 70, 39,
    104, 73, 42, 11, 72, 41, 10, 107, 40, 9, 106, 75, 8, 105, 74, 43,
    108, 77, 46, 15, 76, 45, 14, 111, 44, 13, 110, 79, 12, 109, 78, 47,
    112, 81, 50, 19, 80, 49, 18, 115, 48, 17, 114, 83, 16, 113, 82, 51,
    116, 85, 54, 23, 84, 53, 22, 119, 52, 21, 118, 87, 20, 117, 86, 55,
    120, 89, 58, 27, 88, 57, 26, 123, 56, 25, 122, 91, 24, 121, 90, 59,
    124, 93, 62, 31, 92, 61, 30, 127, 60, 29, 126, 95, 28, 125, 94, 63,
)

INV_PLAYER = (
    12, 9, 6, 3, 28, 25, 22, 19, 44, 41, 38, 35, 60, 57, 54, 51,
    76, 73, 70, 67, 92, 89, 86, 83, 108, 105, 102, 99, 124, 121, 118, 115,
    8, 5, 2, 15, 24, 21, 18, 31, 40, 37, 34, 47, 56, 53, 50, 63,
    72, 69, 66, 79, 88, 85, 82, 95, 104, 101, 98, 111, 120, 117, 114, 127,
    4, 1, 14, 11, 20, 17, 30, 27, 36, 33, 46, 43, 52, 49, 62, 59,
    68, 65, 78, 75, 84, 81, 94, 91, 100, 97, 110, 107, 116, 113, 126, 123,
    0, 13, 10, 7, 16, 29, 26, 23, 32, 45, 42, 39, 48, 61, 58, 55,
    64, 77, 74, 71, 80, 93, 90, 87, 96, 109, 106, 103, 112, 125, 122, 119,
)

# Round constants
RC = (
    2, 33, 16, 9, 36, 19, 40, 53, 26, 13, 38, 51, 56, 61, 62, 31,
    14, 7, 34, 49, 24, 45, 54, 59, 28, 47, 22, 43, 20, 11, 4, 3, 32, 17, 8,
)

# Tap positions for round constant addition
CONST_LOC = (11, 14, 16, 32, 64, 105)

# Number of rounds
N_ROUNDS = 35


def _ensure_length(seq: Sequence[int], expected: int, name: str) -> None:
    """Validate sequence length."""
    if len(seq) != expected:
        raise ValueError(f"{name} must contain {expected} nibbles (got {len(seq)})")


def nibbles_to_bits(x: Sequence[int]) -> List[int]:
    """Convert nibbles to bit array (MSB first per nibble)."""
    bits: List[int] = []
    for nib in x:
        bits.extend(((nib >> shift) & 1) for shift in (3, 2, 1, 0))
    return bits


def bits_to_nibbles(bits: Sequence[int]) -> List[int]:
    """Convert bit array to nibbles."""
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
    """Apply bit permutation."""
    if len(bits) != len(player_map):
        raise ValueError("Bit permutation length mismatch")
    out = [0] * len(bits)
    for src, dst in enumerate(player_map):
        out[dst] = bits[src]
    return out


def pretty_print(x: Iterable[int]) -> str:
    """Convert nibbles to hex string."""
    return ''.join(f"{xx:x}" for xx in x)


def _nibbles_to_words(nibbles: Sequence[int]) -> List[int]:
    """Pack nibbles into 16-bit words."""
    if len(nibbles) % 4:
        raise ValueError("Nibble length must be a multiple of four")
    words: List[int] = []
    for i in range(0, len(nibbles), 4):
        word = 0
        for shift, nib in zip((12, 8, 4, 0), nibbles[i : i + 4]):
            word |= (nib & 0xF) << shift
        words.append(word)
    return words


def _words_to_nibbles(words: Sequence[int]) -> List[int]:
    """Unpack 16-bit words into nibbles."""
    nibbles: List[int] = []
    for word in words:
        for shift in (12, 8, 4, 0):
            nibbles.append((word >> shift) & 0xF)
    return nibbles


def _words_to_bits(words: Sequence[int]) -> List[int]:
    """Convert words to bits (MSB first)."""
    bits: List[int] = []
    for word in words:
        for shift in range(15, -1, -1):
            bits.append((word >> shift) & 1)
    return bits


def _bits_to_words(bits: Sequence[int]) -> List[int]:
    """Convert bits to words (MSB first)."""
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


def _words_to_bits_lsb(words: Sequence[int]) -> List[int]:
    """Convert words to bits (LSB first)."""
    bits: List[int] = []
    for word in words:
        for shift in range(16):
            bits.append((word >> shift) & 1)
    return bits


def _bits_to_words_lsb(bits: Sequence[int]) -> List[int]:
    """Convert bits to words (LSB first)."""
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


def _xor_words(a: Sequence[int], b: Sequence[int]) -> List[int]:
    """XOR two word sequences."""
    return [x ^ y for x, y in zip(a, b)]


def _add_round_constants_bits(bits: List[int], rc: int) -> None:
    """Add round constant to specific bit positions."""
    for i, pos in enumerate(CONST_LOC):
        bits[pos] ^= (rc >> i) & 1


def _apply_sbox_layer(state: Sequence[int]) -> List[int]:
    """Apply S-box to all nibbles."""
    return [LS_sbox[n] for n in state]


def _apply_inv_sbox_layer(state: Sequence[int]) -> List[int]:
    """Apply inverse S-box to all nibbles."""
    return [LS_inv_sbox[n] for n in state]


def _post_process_words(words: Sequence[int]) -> List[int]:
    """Reverse the nibble order of the final state (published ciphertext convention)."""
    out: List[int] = []
    for group in range(8):
        src = words[7 - group]
        word = 0
        for sbox in range(4):
            nib = (src >> (4 * sbox)) & 0xF
            word |= nib << (4 * (3 - sbox))
        out.append(word)
    return out


def _invert_post_process_words(words: Sequence[int]) -> List[int]:
    """Undo the ciphertext nibble-order reversal."""
    out = [0] * 8
    for group in range(8):
        src = words[group]
        word = 0
        for sbox in range(4):
            nib = (src >> (4 * (3 - sbox))) & 0xF
            word |= nib << (4 * sbox)
        out[7 - group] = word
    return out


def _expand_subkeys(key: Sequence[int], rounds: int = N_ROUNDS) -> List[List[int]]:
    """Expand the key schedule for all rounds (rotation in the word-packed layout)."""
    if rounds > len(RC):
        raise ValueError(f"Requested {rounds} rounds but only {len(RC)} round constants")
    current = _nibbles_to_words(key)
    subkeys = [current[:]]
    for _ in range(rounds):
        bits = _words_to_bits_lsb(current)
        bits = bits[1:] + bits[:1]  # One-bit cyclic right rotation of the value; the array is LSB-first, so the array shifts left
        current = _bits_to_words_lsb(bits)
        subkeys.append(current[:])
    return subkeys


def encrypt(key: Sequence[int], pt: Sequence[int], no_of_rounds: int = N_ROUNDS) -> List[int]:
    """
    Encrypt plaintext with BAKSHEESH.
    
    Args:
        key: 32 nibbles (128 bits)
        pt: 32 nibbles (128 bits)
        no_of_rounds: Number of rounds (default: 35)
        
    Returns:
        Ciphertext as 32 nibbles
    """
    _ensure_length(key, 32, "Key")
    _ensure_length(pt, 32, "Plaintext")
    if no_of_rounds > len(RC):
        raise ValueError("Number of rounds exceeds available round constants")

    subkeys = _expand_subkeys(key, no_of_rounds)
    state_words = _nibbles_to_words(pt)

    for rnd in range(no_of_rounds):
        state_words = _xor_words(state_words, subkeys[rnd])
        state_nibbles = _words_to_nibbles(state_words)
        state_nibbles = _apply_sbox_layer(state_nibbles)
        bits = nibbles_to_bits(state_nibbles)
        bits = perm_layer(bits, PLAYER)
        _add_round_constants_bits(bits, RC[rnd])
        state_words = _bits_to_words(bits)

    state_words = _xor_words(state_words, subkeys[no_of_rounds])
    cipher_words = _post_process_words(state_words)
    return _words_to_nibbles(cipher_words)


def decrypt(key: Sequence[int], ct: Sequence[int], no_of_rounds: int = N_ROUNDS) -> List[int]:
    """
    Decrypt ciphertext with BAKSHEESH.
    
    Args:
        key: 32 nibbles (128 bits)
        ct: 32 nibbles (128 bits)
        no_of_rounds: Number of rounds (default: 35)
        
    Returns:
        Plaintext as 32 nibbles
    """
    _ensure_length(key, 32, "Key")
    _ensure_length(ct, 32, "Ciphertext")
    if no_of_rounds > len(RC):
        raise ValueError("Number of rounds exceeds available round constants")

    subkeys = _expand_subkeys(key, no_of_rounds)

    state_words = _nibbles_to_words(ct)
    state_words = _invert_post_process_words(state_words)
    state_words = _xor_words(state_words, subkeys[no_of_rounds])

    for rnd in reversed(range(no_of_rounds)):
        bits = _words_to_bits(state_words)
        _add_round_constants_bits(bits, RC[rnd])
        bits = perm_layer(bits, INV_PLAYER)
        state_nibbles = bits_to_nibbles(bits)
        state_nibbles = _apply_inv_sbox_layer(state_nibbles)
        state_words = _nibbles_to_words(state_nibbles)
        state_words = _xor_words(state_words, subkeys[rnd])

    return _words_to_nibbles(state_words)


def hex_to_nibbles(hex_str: str) -> List[int]:
    """Convert hex string to nibble list."""
    if len(hex_str) != 32:
        raise ValueError("Hex string must be 32 characters (128 bits)")
    nibbles = []
    for char in hex_str:
        nibbles.append(int(char, 16))
    return nibbles


if __name__ == '__main__':
    # Test vectors from designR.tex
    TEST_VECTORS = [
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

    print("=" * 70)
    print("BAKSHEESH Reference Implementation - Test Vectors")
    print("=" * 70)
    
    passed = 0
    failed = 0
    
    for i, tv in enumerate(TEST_VECTORS, 1):
        key_nibbles = hex_to_nibbles(tv['key'])
        pt_nibbles = hex_to_nibbles(tv['plaintext'])
        
        # Encrypt
        ct_nibbles = encrypt(key_nibbles, pt_nibbles)
        ct_hex = pretty_print(ct_nibbles)
        
        # Decrypt
        dec_nibbles = decrypt(key_nibbles, ct_nibbles)
        dec_hex = pretty_print(dec_nibbles)
        
        # Check results
        enc_match = ct_hex == tv['expected']
        dec_match = dec_hex == tv['plaintext']
        
        if enc_match and dec_match:
            status = "PASS"
            passed += 1
        else:
            status = "FAIL"
            failed += 1
        
        print(f"\nTest {i}: {status}")
        print(f"  PT:       {tv['plaintext']}")
        print(f"  Key:      {tv['key']}")
        print(f"  Expected: {tv['expected']}")
        print(f"  Got:      {ct_hex}")
        if not enc_match:
            print("  Encryption mismatch")
        if not dec_match:
            print(f"  Decryption failed: {dec_hex}")
    
    print(f"\n{'=' * 70}")
    print(f"Results: {passed} passed, {failed} failed out of {len(TEST_VECTORS)} tests")
    print(f"{'=' * 70}")
    
    if failed == 0:
        print("\nAll test vectors passed")
    else:
        print(f"\n{failed} test(s) failed")
