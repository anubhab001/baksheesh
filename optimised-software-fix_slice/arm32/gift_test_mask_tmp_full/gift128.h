#ifndef GIFT128_H_
#define GIFT128_H_

#define KEY_SIZE    16
#define BLOCK_SIZE  16
#ifdef __cplusplus
 extern "C" {
#endif
typedef unsigned char u8;
typedef unsigned int u32;

extern void gift128_keyschedule(const u8* key, u32* rkey);
extern void gift128_encrypt_block(u8* out_block, const u32* rkey, const u8* in_block);
extern void giftb128_encrypt_block(u8* out_block, const u32* rkey, const u8* in_block);
#ifdef __cplusplus
}
#endif
#endif  // GIFT128_H_