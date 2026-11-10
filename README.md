# Baksheesh #
This [repository](https://github.com/anubhab001/baksheesh) contains source-codes related to Baksheesh ([IACR ePrint](https://eprint.iacr.org/2023/750), [CiC](https://cic.iacr.org/p/2/4/31)), a lightweight block cipher.


## Directory Structure ##

```
.
├── hardware-ti-order-3
├── hardware-unprotected
├── optimised-fixslice
├── optimised-software
│   └── fix-slice
└── reference
```

## Notes ##
1. Zorro and Gift heritage generate identical test cases.
2. A minor discrepancy was missed while generating test cases for the cipher, as the plaintext and the key are read left-to-right whereas the ciphertext is written right-to-left. The test cases are kept in this form for historical reasons; the left-to-right ciphertext is equally valid and may be used without reservation (see `reference/README.md`).

## Acknowledgment ##
We thank Yong Liu (University of Chinese Academy of Sciences, Beijing, PR China) for taking interest in our cipher and providing valuable feedback on the specification.

## External Links ##
* Traces (power) for unprotected SBox:
    * [Hardware](
    https://vutbr-my.sharepoint.com/:f:/g/personal/xgerli02_vutbr_cz/Enz_sj83kBZCoWbzeIeYAukBIfBt8SbwJYZCy9PCNPhEbg?e=Re2p1T)
    * [Software](https://vutbr-my.sharepoint.com/:f:/g/personal/xgerli02_vutbr_cz/EpmiMzJtXM5Et_w_spJZM2gBnP_LieJ7sVeTOFugsPr7Mg?e=nJm5g3)
