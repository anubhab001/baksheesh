# BAKSHEESH-GIFT Optimized Fixslice Implementation

This directory contains an optimized fix-slice implementation of the BAKSHEESH
block cipher in its GIFT-heritage interpretation, following the fix-slicing
technique introduced for GIFT.

## Contents

- `baksheesh_gift_fixslice.h`, `baksheesh_gift_fixslice.c`: the GIFT-heritage
  fix-slice implementation (35 rounds, BAKSHEESH 4-bit S-box, full 128-bit round
  key, GIFT-128 bit permutation) together with its key schedule.
- `correctness_test.c`: known-answer tests against the specification vectors,
  including asymmetric plaintexts so a byte-order mismatch cannot pass unnoticed.
- `comprehensive_benchmark.c`: a throughput benchmark for the GIFT-heritage
  implementation.
- `baksheesh_3bit_sbox_analysis.sage`: Sage analysis of the 3-bit S-box used in
  the ZORRO-heritage description of the cipher.
- `Makefile`: build and test targets.

## Building and testing

```
make all        # build the correctness test and the benchmark
make test       # run the known-answer tests
make benchmark  # run the throughput benchmark
```

## Notes

The round key is XORed in full (128 bits), as the BAKSHEESH specification
requires.

Only the GIFT-heritage interpretation is provided here. The GIFT and ZORRO
heritages describe the same cipher in two ways, but the ZORRO heritage combines
a 3-bit S-box with a 4x4 binary T-matrix, and a fix-slice pattern for that
T-matrix is not yet available. A direct implementation stays in the nibble
domain and is therefore slower, so an optimized ZORRO fix-slice is left as future
work and is not shipped here.

## License

See the LICENSE file in the parent directory.
