/****************************************************************************
* Fully unrolled ARM assembly implementation.
* I refer to the technique of Fixslicing: A New GIFT Representation.
****************************************************************************/

.syntax unified
.thumb

/******************************************************************************
* Macro to compute the SWAPMOVE technique.
*   - out0-out1     output registers
*   - in0-in1       input registers
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
*   - m0-m1         masks
*   - n0-n1         shift value
*   - tmp           temporary register
******************************************************************************/
.macro nibror   out, in, m0, m1, n0, n1, tmp
    and     \tmp, \m0, \in, lsr \n0
    and     \out, \in, \m1
    orr     \out, \tmp, \out, lsl \n1
.endm

/******************************************************************************
* Macro to compute the SBox (the NOT operation is included in the round keys).
*   - in0-in3       input/output registers
*   - tmp           temporary register
*   - n             ror index value to math fixslicing
******************************************************************************/
.macro sbox     in0, in1, in2, in3, tmp
    and     \tmp, \in2, \in1
    mvn     \tmp, \tmp    
    eor     \in0, \in0, \tmp    
    eor     \in3, \in3, \in0
    mvn     \tmp, \in0
    and     \tmp, \tmp, \in2
    eor     \in1, \in1, \tmp
    and     \tmp, \in1, \in0
    eor     \in2, \in2, \tmp
    eor     \in1, \in1, \in3
    eor     \in0, \in0, \in1
    eor     \in2, \in2, \in0
.endm

/******************************************************************************
* Macro to compute the first round within a quintuple round routine.
*   - in0-in3       input/output registers
*   - const0-const1 round constants
*   - idx           ror index to be used in the sbox (to match fixslicing)
******************************************************************************/
.macro round_0  in0, in1, in2, in3
    sbox    \in0, \in1, \in2, \in3, r8          // sbox layer
    nibror  \in1, \in1, r4, r2, 1, 3, r8        // linear layer
    nibror  \in0, \in0, r2, r4, 3, 1, r8        // linear layer
    orr     r14, r2, r2, lsl #1                 // 0x33333333 for 'nibror'
    nibror  \in3, \in3, r14, r14, 2, 2, r8      // linear layer
    ldr.w   r6, [r1], #4                        // load 1st rkey word
    eor     \in0, \in0, r6                      // add 1st rkey word
    ldr.w   r7, [r1], #4                        // load 2nd rkey word
    eor     \in1, \in1, r7                      // add 2nd rkey word
    ldr.w   r6, [r1], #4                        // load 3st rkey word
    eor     \in2, \in2, r6                      // add 3st rkey word
    ldr.w   r7, [r1], #4                        // load 4nd rkey word
    eor     \in3, \in3, r7                      // add 4nd rkey word
.endm

/******************************************************************************
* Macro to compute the second round within a quintuple round routine.
*   - in0-in3       input/output registers
*   - const0-const1 round constants
******************************************************************************/
.macro round_1  in0, in1, in2, in3
    sbox    \in0, \in1, \in2, \in3, r8          // sbox layer
    mvn     r14, r3, lsl #12                    // r14<-0x0fff0fff for HALF_ROR
    nibror  \in1, \in1, r14, r3,  4,  12, r8    // HALF_ROR(in3, 4)
    nibror  \in0, \in0, r3,  r14, 12,  4, r8    // HALF_ROR(in2, 12)
    rev16   \in3, \in3                          // HALF_ROR(in1, 8)
    ldr.w   r6, [r1], #4                        // load 1st rkey word
    eor     \in0, \in0, r6                      // add 1st rkey word
    ldr.w   r7, [r1], #4                        // load 2nd rkey word
    eor     \in1, \in1, r7                      // add 2nd rkey word
    ldr.w   r6, [r1], #4                        // load 1st rkey word
    eor     \in2, \in2, r6                      // add 1st rkey word
    ldr.w   r7, [r1], #4                        // load 2nd rkey word
    eor     \in3, \in3, r7                      // add 2nd rkey word
   
.endm

/******************************************************************************
* Macro to compute the third round within a quintuple round routine.
*   - in0-in3       input/output registers
*   - const0-const1 round constants
******************************************************************************/
.macro round_2  in0, in1, in2, in3
    sbox    \in0, \in1, \in2, \in3, r8          // sbox layer
    orr     r14, r2, r2, lsl #2                 // r14<-0x55555555 for swpmv
    swpmv   \in3, \in3, \in3, \in3, r14, #1, r8
    eor     r8, \in1, \in1, lsr #1
    and     r8, r8, r14, lsr #16
    eor     \in1, \in1, r8
    eor     \in1, \in1, r8, lsl #1              //SWAPMOVE(r12,r12,0x55550000,1)
    eor     r8, \in0, \in0, lsr #1
    and     r8, r8, r14, lsl #16
    eor     \in0, \in0, r8
    eor     \in0, \in0, r8, lsl #1              //SWAPMOVE(r11,r11,0x00005555,1)
    ldr.w   r6, [r1], #4                        // load 2nd rkey word
    eor     \in0, r6, \in0, ror #16             // add 3nd rkey word
    ldr.w   r7, [r1], #4                        // load 1st rkey word
    eor     \in1, r7, \in1, ror #16             // add 2nd rkey word

    ldr.w   r6, [r1], #4                        // load 1st rkey word
    eor     \in2, \in2, r6                      // add 1st rkey word
    ldr.w   r7, [r1], #4                        // load 2nd rkey word
    eor     \in3, \in3, r7                      // add 1st rkey word


.endm

/******************************************************************************
* Macro to compute the fourth round within a quintuple round routine.
*   - in0-in3       input/output registers
*   - const0-const1 round constants
******************************************************************************/
.macro round_3  in0, in1, in2, in3
    sbox    \in0, \in1, \in2, \in3, r8           // sbox layer
    eor     r14, r3, r3, lsl #8                 // r14<-0x0f0f0f0f for nibror
    nibror  \in3, \in3, r14, r14, #4, #4, r8
    orr     r14, r14, r14, lsl #2               // r14<-0x3f3f3f3f for nibror
    mvn     r8, r14, lsr #6                     // r8 <-0xc0c0c0c0 for nibror
    nibror  \in0, \in0, r14, r8, #2, #6, r6
    nibror  \in1, \in1, r8, r14, #6, #2, r8
    ldr.w   r6, [r1], #4                        // load 1st rkey word
    eor     \in0, \in0, r6                      // add 1st rkey word
    ldr.w   r7, [r1], #4                        // load 2nd rkey word
    eor     \in1, \in1, r7                      // add 2nd rkey word
    ldr.w   r6, [r1], #4                        // load 3st rkey word
    eor     \in2, \in2, r6                      // add 3st rkey word
    ldr.w   r7, [r1], #4                        // load 4nd rkey word
    eor     \in3, \in3, r7                      // add 4nd rkey word
   
.endm

/******************************************************************************
* Macro to compute the fifth round within a quintuple round routine.
*   - in0-in3       input/output registers
*   - const0-const1 round constants
******************************************************************************/
.macro round_4  in0, in1, in2, in3
    sbox    \in0, \in1, \in2, \in3, r8          // sbox layer
    ldr.w   r6, [r1], #4                        // load 1st rkey word
    eor     \in0, r6, \in0                      // add 1st keyword
    ldr.w   r7, [r1], #4                        // load 2nd rkey word
    eor     \in3, r7, \in3, ror #16             // add 2st keyword
    ldr.w   r6, [r1], #4                        // load 3st rkey word
    eor     \in0, r6, \in0, ror #8              // add 3nd keyword
    ldr.w   r7, [r1], #4                        // load 4nd rkey word
    eor     \in1, r7, \in1, ror #24             // add 4st keyword
.endm

/*****************************************************************************
* Subroutine to implement a quintuple round of GIFT-128.
*****************************************************************************/
.align 2
quintuple_round:
    str.w   r14, [sp]
    round_0     r9, r10, r11, r12
    round_1     r10, r12, r9, r11
    round_2     r12, r11, r10, r9
    round_3     r11, r9, r12, r10 
    round_4     r9, r10, r11, r12
    ldr.w   r14, [sp]
    mov         r5, r9
    mov         r9, r10
    mov         r10, r12
    mov         r12, r11
    mov         r11, r5
    bx      lr

/*****************************************************************************
* Fully unrolled implementation of the GIFT-128 block cipher.
* This function simply encrypts a 128-bit block, without any operation mode.
*****************************************************************************/
@ void encrypt_block(u8 *out, const u32* rkey, const u8 *block)
.global encrypt_block
.type   encrypt_block,%function
encrypt_block:
    push {r2-r12,r14}
    ldm         r2, {r4-r7}     // load plaintext blocks in r4-r7
    rev         r4, r4          // endianess to match the fixsliced representation
    rev         r5, r5          // endianess to match the fixsliced representation
    rev         r6, r6          // endianess to match the fixsliced representation
    rev         r7, r7          // endianess to match the fixsliced representation
    // ------------------ PACKING ------------------ 
    uxth        r8, r5
    uxth        r9, r7
    orr         r9, r9, r8, lsl #16             // r9 <- block[6-7] || block[14-15]
    uxth        r8, r5, ror #16
    uxth        r10, r7, ror #16
    orr         r10, r10, r8, lsl #16           // r10<- block[4-5] || block[12-13]
    uxth        r8, r4
    uxth        r11, r6
    orr         r11, r11, r8, lsl #16           // r11<- block[2-3] || block[10-11]
    uxth        r8, r4, ror #16
    uxth        r12, r6, ror #16
    orr         r12, r12, r8, lsl #16           // r12<- block[0-1] || block[8-9]
    movw        r2, #0x0a0a
    movt        r2, #0x0a0a                     // r2 <- 0x0a0a0a0a for SWAPMOVE
    swpmv       r9,  r9,  r9,  r9,  r2, #3, r3
    swpmv       r10, r10, r10, r10, r2, #3, r3
    swpmv       r11, r11, r11, r11, r2, #3, r3
    swpmv       r12, r12, r12, r12, r2, #3, r3
    movw        r2, #0x00cc
    movt        r2, #0x00cc                     // r2 <- 0x00cc00cc for SWAPMOVE
    swpmv       r9,  r9,  r9,  r9,  r2, #6, r3
    swpmv       r10, r10, r10, r10, r2, #6, r3
    swpmv       r11, r11, r11, r11, r2, #6, r3
    swpmv       r12, r12, r12, r12, r2, #6, r3
    movw        r2, #0x000f
    movt        r2, #0x000f                     // r2 <- 0x000f000f for SWAPMOVE
    swpmv       r9,  r10, r9,  r10, r2, #4,  r3
    swpmv       r9,  r11, r9,  r11, r2, #8,  r3
    swpmv       r9,  r12, r9,  r12, r2, #12, r3
    lsl         r2, r2, #4                      // r2 <- 0x00f000f0 for SWAPMOVE
    swpmv       r10, r11, r10, r11, r2, #4,  r3
    swpmv       r10, r12, r10, r12, r2, #8,  r3
    lsl         r2, r2, #4                      // r2 <- 0x0f000f00 for SWAPMOVE
    swpmv       r11, r12, r11, r12, r2, #4,  r3
    // ------------------ GIFTb-CORE ROUTINE ------------------
    mov     r3, r2, lsr #8                  // r3 <- 0x000f000f (for HALF_ROR)
    movw    r2, #0x1111
    movt    r2, #0x1111                     // r2 <- 0x11111111 (for NIBBLE_ROR)
    mvn     r4, r2, lsl #3                  // r4 <- 0x7777777 (for NIBBLE_ROR)

    bl      quintuple_round
    bl      quintuple_round
    bl      quintuple_round
    bl      quintuple_round
    bl      quintuple_round
    bl      quintuple_round
    bl      quintuple_round
    // ------------------ UNPACKING ------------------ 
    ldr.w   r0, [sp, #4]                    //restore 'ctext' address
    movw    r2, #0x0f00
    movt    r2, #0x0f00                     // r2 <- 0x0f000f00 for SWAPMOVE
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
    add.w   sp, #4
    pop     {r0,r2-r12,r14}
    bx      lr
