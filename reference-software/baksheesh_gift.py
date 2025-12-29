"""
BAKSHEESH Reference Implementation

Usage:
- encrypt(key, pt, use_gift_permutation=True)   # GIFT-matching bit-permutation
- encrypt(key, pt, use_gift_permutation=False)  # Canonical bit-permutation
"""

from __future__ import annotations

from typing import Iterable, List, Sequence

# SBox
LS_sbox = (3, 0, 6, 13, 11, 5, 8, 14, 12, 15, 9, 2, 4, 10, 7, 1)
LS_inv_sbox = (1, 15, 11, 0, 12, 5, 2, 14, 6, 10, 13, 4, 8, 3, 7, 9)

# Bit-permutation (GIFT-matching version)
player_gift_based = (
    0, 33, 66, 99, 96, 1, 34, 67, 64, 97, 2, 35, 32, 65, 98, 3,
    4, 37, 70, 103, 100, 5, 38, 71, 68, 101, 6, 39, 36, 69, 102, 7,
    8, 41, 74, 107, 104, 9, 42, 75, 72, 105, 10, 43, 40, 73, 106, 11,
    12, 45, 78, 111, 108, 13, 46, 79, 76, 109, 14, 47, 44, 77, 110, 15,
    16, 49, 82, 115, 112, 17, 50, 83, 80, 113, 18, 51, 48, 81, 114, 19,
    20, 53, 86, 119, 116, 21, 54, 87, 84, 117, 22, 55, 52, 85, 118, 23,
    24, 57, 90, 123, 120, 25, 58, 91, 88, 121, 26, 59, 56, 89, 122, 27,
    28, 61, 94, 127, 124, 29, 62, 95, 92, 125, 30, 63, 60, 93, 126, 31,
)
inv_player_gift_based = (
    0, 5, 10, 15, 16, 21, 26, 31, 32, 37, 42, 47, 48, 53, 58, 63,
    64, 69, 74, 79, 80, 85, 90, 95, 96, 101, 106, 111, 112, 117, 122, 127,
    12, 1, 6, 11, 28, 17, 22, 27, 44, 33, 38, 43, 60, 49, 54, 59,
    76, 65, 70, 75, 92, 81, 86, 91, 108, 97, 102, 107, 124, 113, 118, 123,
    8, 13, 2, 7, 24, 29, 18, 23, 40, 45, 34, 39, 56, 61, 50, 55,
    72, 77, 66, 71, 88, 93, 82, 87, 104, 109, 98, 103, 120, 125, 114, 119,
    4, 9, 14, 3, 20, 25, 30, 19, 36, 41, 46, 35, 52, 57, 62, 51,
    68, 73, 78, 67, 84, 89, 94, 83, 100, 105, 110, 99, 116, 121, 126, 115,
)

# Actual bit-permutation (canonical ordering)
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
inv_player = (
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
const_loc = (11, 14, 16, 32, 64, 105)

# Round number
N_rounds = 35


def _ensure_length(seq: Sequence[int], expected: int, name: str) -> None:
    if len(seq) != expected:
        raise ValueError(f"{name} must contain {expected} nibbles (got {len(seq)})")


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


def add_key(pt: Sequence[int], key: Sequence[int]) -> List[int]:
    return [p ^ k for p, k in zip(pt, key)]


def pretty_print(x: Iterable[int]) -> str:
    return ''.join(f"{xx:x}" for xx in x)


def key_update(key: Sequence[int], step: int = 1) -> List[int]:
    """Rotate the bit-sliced key by ``step`` positions (left for positive values)."""
    words = _nibbles_to_words(key)
    bits = _words_to_bits_lsb(words)
    if not bits:
        return []
    shift = step % len(bits)
    rotated = bits[shift:] + bits[:shift]
    rotated_words = _bits_to_words_lsb(rotated)
    return _words_to_nibbles(rotated_words)


def _nibbles_to_words(nibbles: Sequence[int]) -> List[int]:
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
    nibbles: List[int] = []
    for word in words:
        for shift in (12, 8, 4, 0):
            nibbles.append((word >> shift) & 0xF)
    return nibbles


def _words_to_bits(words: Sequence[int]) -> List[int]:
    bits: List[int] = []
    for word in words:
        for shift in range(15, -1, -1):
            bits.append((word >> shift) & 1)
    return bits


def _bits_to_words(bits: Sequence[int]) -> List[int]:
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
    bits: List[int] = []
    for word in words:
        for shift in range(16):
            bits.append((word >> shift) & 1)
    return bits


def _bits_to_words_lsb(bits: Sequence[int]) -> List[int]:
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
    return [x ^ y for x, y in zip(a, b)]


def _add_round_constants_bits(bits: List[int], rc: int) -> None:
    for i, pos in enumerate(const_loc):
        bits[pos] ^= (rc >> i) & 1


def _apply_sbox_layer(state: Sequence[int]) -> List[int]:
    return [LS_sbox[n] for n in state]


def _apply_inv_sbox_layer(state: Sequence[int]) -> List[int]:
    return [LS_inv_sbox[n] for n in state]


def _post_process_words(words: Sequence[int]) -> List[int]:
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
    out = [0] * 8
    for group in range(8):
        src = words[group]
        word = 0
        for sbox in range(4):
            nib = (src >> (4 * (3 - sbox))) & 0xF
            word |= nib << (4 * sbox)
        out[7 - group] = word
    return out


def _expand_subkeys(key: Sequence[int], rounds: int = N_rounds) -> List[List[int]]:
    if rounds > len(RC):
        raise ValueError(f"Requested {rounds} rounds but only {len(RC)} round constants are defined")
    current = _nibbles_to_words(key)
    subkeys = [current[:]]
    for _ in range(rounds):
        bits = _words_to_bits_lsb(current)
        bits = bits[1:] + bits[:1]
        current = _bits_to_words_lsb(bits)
        subkeys.append(current[:])
    return subkeys


def encrypt(
    key: Sequence[int],
    pt: Sequence[int],
    no_of_rounds: int = N_rounds,
    *,
    use_gift_permutation: bool = False,
) -> List[int]:
    _ensure_length(key, 32, "Key")
    _ensure_length(pt, 32, "Plaintext")
    if no_of_rounds > len(RC):
        raise ValueError("Number of rounds exceeds available round constants")

    subkeys = _expand_subkeys(key, no_of_rounds)
    state_words = _nibbles_to_words(pt)
    player_map = player_gift_based if use_gift_permutation else player

    for rnd in range(no_of_rounds):
        state_words = _xor_words(state_words, subkeys[rnd])
        state_nibbles = _words_to_nibbles(state_words)
        state_nibbles = _apply_sbox_layer(state_nibbles)
        bits = nibbles_to_bits(state_nibbles)
        bits = perm_layer(bits, player_map)
        _add_round_constants_bits(bits, RC[rnd])
        state_words = _bits_to_words(bits)

    state_words = _xor_words(state_words, subkeys[no_of_rounds])
    cipher_words = _post_process_words(state_words)
    return _words_to_nibbles(cipher_words)


def decrypt(
    key: Sequence[int],
    ct: Sequence[int],
    no_of_rounds: int = N_rounds,
    *,
    use_gift_permutation: bool = False,
) -> List[int]:
    _ensure_length(key, 32, "Key")
    _ensure_length(ct, 32, "Ciphertext")
    if no_of_rounds > len(RC):
        raise ValueError("Number of rounds exceeds available round constants")

    subkeys = _expand_subkeys(key, no_of_rounds)
    inv_player_map = inv_player_gift_based if use_gift_permutation else inv_player

    state_words = _nibbles_to_words(ct)
    state_words = _invert_post_process_words(state_words)
    state_words = _xor_words(state_words, subkeys[no_of_rounds])

    for rnd in reversed(range(no_of_rounds)):
        bits = _words_to_bits(state_words)
        _add_round_constants_bits(bits, RC[rnd])
        bits = perm_layer(bits, inv_player_map)
        state_nibbles = bits_to_nibbles(bits)
        state_nibbles = _apply_inv_sbox_layer(state_nibbles)
        state_words = _nibbles_to_words(state_nibbles)
        state_words = _xor_words(state_words, subkeys[rnd])

    return _words_to_nibbles(state_words)


if __name__ == '__main__':
    key = [0] * 32
    pt = [0] * 32

    print("\n--- Canonical permutation ---")
    ct_alt = encrypt(key=key[:], pt=pt[:], use_gift_permutation=False)
    print('CT ', pretty_print(ct_alt))
    dec_alt = decrypt(key=key[:], ct=ct_alt[:], use_gift_permutation=False)
    print('PT ', pretty_print(dec_alt))
    assert dec_alt == pt, 'Canonical permutation failed encryption + decryption'

    print("\n--- GIFT-matching permutation ---")
    ct_gift = encrypt(key=key[:], pt=pt[:], use_gift_permutation=True)
    print('CT ', pretty_print(ct_gift))
    dec_gift = decrypt(key=key[:], ct=ct_gift[:], use_gift_permutation=True)
    print('PT ', pretty_print(dec_gift))
    assert dec_gift == pt, 'GIFT permutation failed encryption + decryption'

    print("\n✓ Reference implementation matches.")

