/****************************************************************************
* 1st order masked ARM assembly implementation of the GIFT-128 block cipher.
* See 'Fixslicing: A New GIFT Representation' paper available at
* https://eprint.iacr.org/2020/412 for more details.
*
* @author   Alexandre Adomnicai, Nanyang Technological University,
*           alexandre.adomnicai@ntu.edu.sg
*
* @date     July 2021
****************************************************************************/

.syntax unified
.thumb

.type rconst,%object
rconst:
.word 0x10000008, 0x80018000, 0x54000002, 0x01010181
.word 0x8000001f, 0x10888880, 0x6001e000, 0x51500002
.word 0x03030180, 0x8000002f, 0x10088880, 0x60016000
.word 0x41500002, 0x03030080, 0x80000027, 0x10008880
.word 0x4001e000, 0x11500002, 0x03020180, 0x8000002b
.word 0x10080880, 0x60014000, 0x01400002, 0x02020080
.word 0x80000021, 0x10000080, 0x0001c000, 0x51000002
.word 0x03010180, 0x8000002e, 0x10088800, 0x60012000
.word 0x40500002, 0x01030080, 0x80000006, 0x10008808
.word 0xc001a000, 0x14500002, 0x01020181, 0x8000001a

/******************************************************************************
* Macro to compute the SWAPMOVE technique.
*   - out0,out1     output registers
*   - in0,in1       input registers
*   - m             mask
*   - n             shift value
*   - tmp           temporary register
******************************************************************************/
.macro swpmv    out0, out1, in0, in1, m, n, tmp
    eor     \tmp, \in1, \in0, lsr \n
    and     \tmp, \m
    eor     \out1, \in1, \tmp
    eor     \out0, \in0, \tmp, lsl \n
.endm

/******************************************************************************
* Macro to compute a nibble-wise rotation to the right.
*   - out           output register
*   - in            input register
*   - m0,m1         masks
*   - n0,n1         shift value
*   - tmp           temporary register
******************************************************************************/
.macro nibror   out, in, m0, m1, n0, n1, tmp
    and     \tmp, \m0, \in, lsr \n0
    and     \out, \in, \m1
    orr     \out, \tmp, \out, lsl \n1
.endm

/******************************************************************************
* 1st-order secure AND between two masked values. Technique from the paper
* 'Optimal First-Order Boolean Masking for Embedded IoT Devices' available at
* https://orbilu.uni.lu/bitstream/10993/37740/1/Optimal_Masking.pdf.
*   - z1,z2         output shares
*   - x1,x2         1st input shares
*   - y1,y2         2nd input shares
*   - tmp           temporary register
******************************************************************************/
.macro secand   z1, z2, x1, x2, y1, y2, tmp
    orn     \tmp, \x1, \y2
    and     \z1, \x1, \y1
    eor     \z1, \tmp, \z1
    orn     \tmp, \x2, \y2
    and     \z2, \x2, \y1
    eor     \z2, \z2, \tmp
.endm

/******************************************************************************
* 1st-order secure OR between two masked values. Technique from the paper
* 'Optimal First-Order Boolean Masking for Embedded IoT Devices' available at
* https://orbilu.uni.lu/bitstream/10993/37740/1/Optimal_Masking.pdf.
*   - z1,z2         output shares
*   - x1,x2         1st input shares
*   - y1,y2         2nd input shares
*   - tmp           temporary register
******************************************************************************/
.macro secor    z1, z2, x1, x2, y1, y2, tmp
    orr     \tmp, \x1, \y2
    and     \z1, \x1, \y1
    eor     \z1, \tmp, \z1
    and     \tmp, \x2, \y2
    orr     \z2, \x2, \y1
    eor     \z2, \z2, \tmp
.endm

/******************************************************************************
* 1st-order secure XOR between two masked values.
*   - z1,z2         output shares
*   - x1,x2         1st input shares
*   - y1,y2         2nd input shares
******************************************************************************/
.macro secxor   z1, z2, x1, x2, y1, y2
    eor     \z1, \x1, \y1
    eor     \z2, \x2, \y2
.endm

/******************************************************************************
* 1st-order masked S-box. Registers r10,r3 always refer to state[1] while 
* r11,r4 always refer to state[2].
*   - in0           1st input register (i.e. state[0])
*   - in3           4th input register (i.e. state[3])
******************************************************************************/
.macro sbox     in0, in0_m, in3, in3_m
    secand  r8, r7, \in0, \in0_m, r11, r4, r6
    secxor  r10, r3, r10, r3, r8, r7
    secand  r8, r7, r10, r3, \in3, \in3_m, r6
    secxor  \in0, \in0_m, \in0, \in0_m, r8, r7
    secor   r8, r7, \in0, \in0_m, r10, r3, r6
    secxor  r11, r4, r11, r4, r8, r7
    secxor  \in3, \in3_m, \in3, \in3_m, r11, r4
    secxor  r10, r3, r10, r3, \in3, \in3_m
    secand  r8, r7, \in0, \in0_m, r10, r3, r6
    secxor  r11, r4, r11, r4, r8, r7
    mvn     \in3, \in3
.endm

/******************************************************************************
* 1st-order masked linear layer for rounds i s.t. i % 5 = 0.
******************************************************************************/
.macro llayer0
    mvn     r6, r14, lsl #3                 // r6<- 0x77777777 for nibror
    nibror  r12, r12, r6, r14, 1, 3, r8     // nibror(r12,1)
    nibror  r5, r5, r6, r14, 1, 3, r8       // mask correction
    nibror  r11, r11, r14, r6, 3, 1, r8     // nibror(r11,3)
    nibror  r4, r4, r14, r6, 3, 1, r8       // mask correction
    orr     r6, r14, r14, lsl #1            // r6 <- 0x33333333 for nibror
    nibror  r10, r10, r6, r6, 2, 2, r8      // nibror(r10, 2)
    nibror  r3, r3, r6, r6, 2, 2, r8        // mask correction
.endm

/******************************************************************************
* 1st-order masked linear layer for rounds i s.t. i % 5 = 1.
******************************************************************************/
.macro llayer1
    movw    r6, #0x000f
    movt    r6, #0x000f                     // r6 <- 0x000f000f for halfror
    mvn     r7, r6, lsl #12                 // r7 <- 0x0fff0fff for halfror
    nibror  r9, r9, r7, r6,  4,  12, r8     // halfror(r9,4)
    nibror  r2, r2, r7, r6,  4,  12, r8     // mask correction
    nibror  r11, r11, r6, r7,  12,  4, r8   // halfror(r11,12)
    nibror  r4, r4, r6, r7,  12,  4, r8     // mask correction
    rev16   r10, r10                        // halfror(r10,8)
    rev16   r3, r3                          // mask correction
.endm

/******************************************************************************
* 1st-order masked linear layer for rounds i s.t. i % 5 = 2.
******************************************************************************/
.macro llayer2
    movw    r6, #0x5555
    movt    r6, #0x5555                     // r6 <- 0x55555555 for swpmv
    swpmv   r10, r10, r10, r10, r6, #1, r8  // swpmv(r10, r10, 0x55..55, 1)
    swpmv   r3, r3, r3, r3, r6, #1, r8      // mask correction
    eor     r8, r12, r12, lsr #1
    and     r8, r8, r6, lsr #16
    eor     r12, r12, r8
    eor     r12, r12, r8, lsl #1            // swpmv(r12, r12, 0x55550000, 1)
    eor     r8, r5, r5, lsr #1
    and     r8, r8, r6, lsr #16
    eor     r5, r5, r8
    eor     r5, r5, r8, lsl #1              // mask correction
    eor     r8, r11, r11, lsr #1
    and     r8, r8, r6, lsl #16
    eor     r11, r11, r8
    eor     r11, r11, r8, lsl #1            // swpmv(r11, r11, 0x00005555, 1)
    eor     r8, r4, r4, lsr #1
    and     r8, r8, r6, lsl #16
    eor     r4, r4, r8
    eor     r4, r4, r8, lsl #1              // mask correction
.endm

/******************************************************************************
* 1st-order masked linear layer for rounds i s.t. i % 5 = 3.
******************************************************************************/
.macro llayer3
    movw    r6, #0x0f0f
    movt    r6, #0x0f0f                     // r6 <- 0x0f0f0f0f for byteror
    nibror  r10, r10, r6, r6, #4, #4, r8    // byteror(r10,4)
    nibror  r3, r3, r6, r6, #4, #4, r8      // mask correction
    orr     r6, r6, r6, lsl #2              // r6 <- 0x3f3f3f3f for byteror
    mvn     r8, r6
    and     r7, r8, r11, lsl #6
    and     r11, r6, r11, lsr #2
    orr     r11, r11, r7                    // byteror(r11,2)
    and     r7, r8, r4, lsl #6
    and     r4, r6, r4, lsr #2
    orr     r4, r4, r7                      // mask correction
    mvn     r8, r6, lsr #6                  // r8 <- 0xc0c0c0c0 for byteror
    nibror  r9, r9, r8, r6, #6, #2, r7      // byteror(r9, 6)
    nibror  r2, r2, r8, r6, #6, #2, r7      // mask correction
.endm

/******************************************************************************
* 1st-order masked add round key.
******************************************************************************/
.macro ark  in0, ror_idx0, ror_idx1
    ldr.w   r6, [r1], #4                    // load 1st rkey word
    ldr.w   r7, [r1], #4                    // load 2nd rkey word                 
    eor     r10, r6, r10, ror \ror_idx0     // add 1st rkey word
    eor     r11, r7, r11, ror \ror_idx1     // add 2nd rkey word
    ldr.w   r6, [r1], #4                    
    ldr.w   r7, [r1], #4                    
    eor     r12, r6, r12
    eor     r9, r7, r9
    ldr.w   r6, [r1, #312]                  // load 1st rkey_mask
    ldr.w   r7, [r1, #316]                  // load 2nd rkey_mask
    ldr.w   r14, [r0], #4                   // load rconst
    eor     r3, r6, r3, ror \ror_idx0       // mask correction
    eor     r4, r7, r4, ror \ror_idx1       // mask correction
    eor     \in0, \in0, r14                 // add rconst
    ldr.w   r6, [r1, #320]                  // load 1st rkey_mask
    ldr.w   r7, [r1, #324]                  // load 2nd rkey_mask
    eor     r2, r6, r3
    eor     r5, r7, r4
.endm

/******************************************************************************
* 1st-order masked quintuple round.
******************************************************************************/
quintuple_round_masked:
    str.w   r14, [sp]
    movw r14, #0x1111
    movt r14, #0x1111                       // r14<- 0x11111111
    sbox    r9, r2, r12, r5                 // 1st round
    llayer0
    ark     r9, #0, #0
    sbox    r12, r5, r9, r2                 // 2nd round
    llayer1
    ark     r12, #0, #0
    sbox    r9, r2, r12, r5                 // 3rd round
    llayer2
    ark     r9, #0, #16
    ror     r5, #16
    ror     r12, #16
    sbox    r12, r5, r9, r2                 // 4th round
    llayer3 
    ark     r12, #0, #0
    sbox    r9, r2, r12, r5                 // 5th round
    ark     r9, #16, #8
    ldr.w   r14, [sp]
    eor     r9, r9, r12, ror #24
    eor     r12, r9, r12, ror #24
    eor     r9, r9, r12                     // swap r9 with r12 >>> 24
    eor     r2, r2, r5, ror #24
    eor     r5, r2, r5, ror #24
    eor     r2, r2, r5                      // swap r2 with r5 >>> 24
    bx      lr

/*****************************************************************************
* 1st order masked implementation of the GIFT-128 block cipher. This function
* simply encrypts a 128-bit block, without any operation mode.
*****************************************************************************/
@ void gift128_encrypt_block(u8 *out, const u32* rkey, const u8 *block)
.global gift128_encrypt_block
.type   gift128_encrypt_block,%function
gift128_encrypt_block:
    push    {r0-r12,r14}
    // ------------------ PACKING ------------------
    ldm     r2, {r4-r7}     // load plaintext blocks
    rev     r4, r4          // endianess to match the fixsliced representation
    rev     r5, r5          // endianess to match the fixsliced representation
    rev     r6, r6          // endianess to match the fixsliced representation
    rev     r7, r7          // endianess to match the fixsliced representation
    uxth    r8, r5
    uxth    r9, r7
    orr     r9, r9, r8, lsl #16             // r9 <- block[6-7] || block[14-15]
    uxth    r8, r5, ror #16
    uxth    r10, r7, ror #16
    orr     r10, r10, r8, lsl #16           // r10<- block[4-5] || block[12-13]
    uxth    r8, r4
    uxth    r11, r6
    orr     r11, r11, r8, lsl #16           // r11<- block[2-3] || block[10-11]
    uxth    r8, r4, ror #16
    uxth    r12, r6, ror #16
    orr     r12, r12, r8, lsl #16           // r12<- block[0-1] || block[8-9]
    movw    r2, #0x0a0a
    movt    r2, #0x0a0a                     // r2 <- 0x0a0a0a0a for SWAPMOVE
    swpmv   r9,  r9,  r9,  r9,  r2, #3, r3
    swpmv   r10, r10, r10, r10, r2, #3, r3
    swpmv   r11, r11, r11, r11, r2, #3, r3
    swpmv   r12, r12, r12, r12, r2, #3, r3
    movw    r2, #0x00cc
    movt    r2, #0x00cc                     // r2 <- 0x00cc00cc for SWAPMOVE
    swpmv   r9,  r9,  r9,  r9,  r2, #6, r3
    swpmv   r10, r10, r10, r10, r2, #6, r3
    swpmv   r11, r11, r11, r11, r2, #6, r3
    swpmv   r12, r12, r12, r12, r2, #6, r3
    movw    r2, #0x000f
    movt    r2, #0x000f                     // r2 <- 0x000f000f for SWAPMOVE
    swpmv   r9,  r10, r9,  r10, r2, #4,  r3
    swpmv   r9,  r11, r9,  r11, r2, #8,  r3
    swpmv   r6,  r12, r9,  r12, r2, #12, r3
    lsl     r2, r2, #4                      // r2 <- 0x00f000f0 for SWAPMOVE
    swpmv   r10, r11, r10, r11, r2, #4,  r3
    swpmv   r7, r12, r10, r12, r2, #8,  r3
    lsl     r2, r2, #4                      // r2 <- 0x0f000f00 for SWAPMOVE
    swpmv   r8, r14, r11, r12, r2, #4,  r3
    // ------------------ MASKING ------------------
    eor     r9, r2, r6                     // apply masks to the internal state
    eor     r10, r3, r7                    // apply masks to the internal state
    eor     r11, r4, r8                    // apply masks to the internal state
    eor     r12, r5, r14                   // apply masks to the internal state
    // ------------------ GIFTb-CORE ROUTINE ------------------
    adr     r0, rconst
    sub.w   sp, #4
    ark     r9, #0, #0
    bl      quintuple_round_masked
    bl      quintuple_round_masked
    bl      quintuple_round_masked
    bl      quintuple_round_masked
    bl      quintuple_round_masked
    bl      quintuple_round_masked
    bl      quintuple_round_masked
    bl      quintuple_round_masked
    add.w   sp, #4
    ldr.w   r0, [sp]                        // restore 'ctext' address
    // ------------------ UNMASKING ------------------
    mov     r6, r9
    mov     r9, #0                          // clear r9 to avoid HD leakages
    mov     r7, r10
    mov     r10, #0                         // clear r10 to avoid HD leakages
    mov     r8, r11
    mov     r11, #0                         // clear r11 to avoid HD leakages
    mov     r14, r12
    mov     r12, #0                         // clear r12 to avoid HD leakages
    eor     r9, r6, r2                      // unmask the internal state
    eor     r10, r7, r3                     // unmask the internal state
    eor     r11, r8, r4                     // unmask the internal state
    eor     r12, r14, r5                    // unmask the internal state
    // ------------------ UNPACKING ------------------ 
    movw    r2, #0x0f00
    movt    r2, #0x0f00
    swpmv   r11, r12, r11, r12, r2, #4,  r3
    lsr     r2, r2, #4                      // r2 <- 0x00f000f0 for SWAPMOVE
    swpmv   r10, r12, r10, r12, r2, #8,  r3
    swpmv   r10, r11, r10, r11, r2, #4,  r3
    lsr     r2, r2, #4                      // r2 <- 0x000f000f for SWAPMOVE
    swpmv   r9,  r12, r9,  r12, r2, #12, r3
    swpmv   r9,  r11, r9,  r11, r2, #8,  r3
    swpmv   r9,  r10, r9,  r10, r2, #4,  r3
    movw    r2, #0x00cc
    movt    r2, #0x00cc                     // r2 <- 0x00cc00cc for SWAPMOVE
    swpmv   r9,  r9,  r9,  r9,  r2, #6, r3
    swpmv   r10, r10, r10, r10, r2, #6, r3
    swpmv   r11, r11, r11, r11, r2, #6, r3
    swpmv   r12, r12, r12, r12, r2, #6, r3
    movw    r2, #0x0a0a
    movt    r2, #0x0a0a                     // r2 <- 0x0a0a0a0a for SWAPMOVE
    swpmv   r9,  r9,  r9,  r9,  r2, #3, r3
    swpmv   r10, r10, r10, r10, r2, #3, r3
    swpmv   r11, r11, r11, r11, r2, #3, r3
    swpmv   r12, r12, r12, r12, r2, #3, r3
    lsr     r4, r12, #16
    lsr     r5, r11, #16
    orr     r4, r5, r4, lsl #16             // r4 <- out[0]
    lsr     r5, r10, #16
    lsr     r6, r9, #16
    orr     r5, r6, r5, lsl #16             // r5 <- out[1]
    lsl     r6, r12, #16
    lsl     r7, r11, #16
    orr     r6, r6, r7, lsr #16             // r6 <- out[2]
    lsl     r7, r10, #16
    lsl     r8, r9, #16
    orr     r7, r7, r8, lsr #16             // r7 <- out[3]
    rev     r4, r4
    rev     r5, r5
    rev     r6, r6
    rev     r7, r7
    stm     r0, {r4-r7}
    pop     {r0-r12,r14}
    bx      lr
