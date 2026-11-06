"""
BAKSHEESH Encryption Trace Generator

This script prints a complete 35-round encryption trace for one published test
vector: the input state, the round key and the output state of every round,
together with the intermediate values after each layer. All values are printed
in the layout of the reference implementation (see the repository README). The
committed file encryption_trace.txt holds the output of this script.
"""

import baksheesh as bk

KEY_HEX = "25798b6c2b24cd83281de76786213b86"
PT_HEX = "57cf5777053928343047ebe8dbfc374f"


def words_to_hex(words):
    """Render a word state as a 32-character hexadecimal string."""
    return bk.pretty_print(bk._words_to_nibbles(words))


def main():
    key = bk.hex_to_nibbles(KEY_HEX)
    pt = bk.hex_to_nibbles(PT_HEX)
    subkeys = bk._expand_subkeys(key)

    print("BAKSHEESH encryption trace (35 rounds)")
    print()
    print(f"Key      : {KEY_HEX}")
    print(f"Plaintext: {PT_HEX}")
    print()
    print("All values are printed in the layout of the reference implementation:")
    print("the leftmost character is nibble 0 of the state array, exactly as the")
    print("plaintext and the key are read from their hexadecimal strings. Round")
    print("key 0 is the master key (whitening key). Round r applies SubCells,")
    print("PermBits, AddConstants (constant RC[r-1]) and the XOR of round key r.")
    print("The published ciphertext is the final state written in reversed")
    print("nibble order.")
    print()

    state = bk._nibbles_to_words(pt)
    print(f"Initial state           : {words_to_hex(state)}")
    print(f"Round key 00 (whitening): {words_to_hex(subkeys[0])}")
    state = bk._xor_words(state, subkeys[0])
    print(f"State after whitening   : {words_to_hex(state)}")

    for rnd in range(bk.N_ROUNDS):
        print()
        print(f"Round {rnd + 1:02d}")
        print(f"  Input state       : {words_to_hex(state)}")
        nibbles = bk._apply_sbox_layer(bk._words_to_nibbles(state))
        print(f"  After SubCells    : {bk.pretty_print(nibbles)}")
        bits = bk.perm_layer(bk.nibbles_to_bits(nibbles), bk.PLAYER)
        print(f"  After PermBits    : {words_to_hex(bk._bits_to_words(bits))}")
        bk._add_round_constants_bits(bits, bk.RC[rnd])
        state = bk._bits_to_words(bits)
        print(f"  After AddConstants: {words_to_hex(state)}  (constant {bk.RC[rnd]})")
        print(f"  Round key {rnd + 1:02d}      : {words_to_hex(subkeys[rnd + 1])}")
        state = bk._xor_words(state, subkeys[rnd + 1])
        print(f"  Output state      : {words_to_hex(state)}")

    final_hex = words_to_hex(state)
    ct_hex = bk.pretty_print(bk.encrypt(key, pt))
    print()
    print(f"Final state             : {final_hex}")
    print(f"Ciphertext (reversed)   : {ct_hex}")
    if ct_hex != final_hex[::-1]:
        raise RuntimeError("Trace does not match the reference encryption")


if __name__ == "__main__":
    main()
