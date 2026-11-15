#ifndef fixslice_H_
#define fixslice_H_

#define KEY_SIZE    16
#define BLOCK_SIZE  16
#ifdef __cplusplus
 extern "C" {
#endif
typedef unsigned char u8;
typedef unsigned int u32;

/* Portable C baseline. */
extern void encrypt_block_ref(u8* out_block, const u32* rkey, const u8* in_block);
/* Fully unrolled ARM assembly, optimised for speed. */
extern void encrypt_block_fast(u8* out_block, const u32* rkey, const u8* in_block);
/* Fully unrolled ARM assembly, optimised for code size. */
extern void encrypt_block_compact(u8* out_block, const u32* rkey, const u8* in_block);
/* 1st-order masked ARM assembly. */
extern void encrypt_block_masked(u8* out_block, const u32* rkey, const u8* in_block);

#ifdef __cplusplus
}
#endif
#endif  // fixslice_H_
