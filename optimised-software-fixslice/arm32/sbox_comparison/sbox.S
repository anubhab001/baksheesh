.syntax unified
.thumb 

.macro sbox_baksheesh     in0, in1, in2, in3, tmp
    orr     \tmp, \in1, \in0
    eor     \in2, \in2, \tmp
    orn     \tmp, \in0, \in2
    eor     \in1, \in1, \tmp
    and     \tmp, \in1, \in2
    eor     \in0, \in0 , \tmp
    eor     \in1, \in1, \in3
    eor     \in2, \in2, \in1
    eor     \in3, \in3, \in0
    eor     \in0, \in0, \in2
    mvn     \in1, \in1
.endm

.macro sbox_gift     in0, in1, in2, in3, tmp
    and     \tmp, \in2, \in0
    eor     \in1, \in1, \tmp
    and     \tmp, \in1, \in3
    eor     \in0, \tmp, \in0
    orr     \tmp, \in0, \in1
    eor     \in2, \tmp, \in2
    eor     \in3, \in3, \in2
    eor     \in1, \in1, \in3
    and     \tmp, \in0, \in1
    eor     \in2, \in2, \tmp
    mvn     \in3, \in3
.endm


.macro sbox_present       in0, in1, in2, in3, temp1, temp2, temp3, temp4, temp5
    eor     \temp1, \in1, \in2
    and     \temp2, \in2, \temp1
    eor     \temp3, \in3, \temp2
    eor     \temp5, \in0, \temp3
    and     \temp2, \temp1, \temp3
    eor     \temp1, \temp1, \temp5
    eor     \temp2, \temp2, \in2
    orr     \temp4, \in0, \temp2
    eor     \in1, \temp1, \temp4
    mvn     \in0, \in0
    eor     \temp2, \temp2, \in0
    eor     \in3, \in1, \temp2
    orr     \temp2, \temp2, \temp1
    eor     \in2, \temp3, \temp2
    mov     \in0, \temp5
.endm


.macro sbox_piccolo in0, in1, in2, in3, tmp
    mov     \tmp, \in1            
    orr     \in1, \in1, \in2     
    mvn     \in3, \in3          

    eor     \in0, \in0, \in2      
    eor     \in1, \in1, \in3     
    orr     \in3, \in3, \in2     

    eor     \in0, \in0, \in3    
    mov     \in3, \in1          

    orr     \in3, \in3, \in0     

    eor     \in3, \in3, \tmp     
    orr     \tmp, \tmp, \in0      

    eor     \in2, \in2, \tmp      
    mvn     \in3, \in3           
.endm


.macro sbox_pyjamask128 in3, in2, in1, in0, tmp
    eor     \in0, \in0, \in3      

    and     \tmp, \in0, \in1       
    eor     \in3, \in3, \tmp       

    and     \tmp, \in1, \in2        
    eor     \in0, \in0, \tmp        

    and     \tmp, \in2, \in3        
    eor     \in1, \in1, \tmp        

    and     \tmp, \in0, \in3       
    eor     \in2, \in2, \tmp       

    eor     \in2, \in2, \in1      

    eor     \in1, \in1, \in0      

    mvn     \in3, \in3            
.endm


.align 2
@ void void test_baksheesh(uint32_t* in,  uint32_t* out);
.global test_baksheesh
.type   test_baksheesh,%function
test_baksheesh:
    push {r2-r12,r14}
    ldm         r0, {r4-r7}   
    sbox_baksheesh   r4, r5, r6, r7, r8

    str r6, [r1], #4
    str r4, [r1], #4
    str r7, [r1], #4
    str r5, [r1], #4
    pop         {r2-r12,r14}
    bx          lr


.align 2
@ void void test_gift(uint32_t* in,  uint32_t* out);
.global test_gift
.type   test_gift,%function
test_gift:
    push {r2-r12,r14}
    ldm         r0, {r4-r7}     
    sbox_gift        r4, r5, r6, r7, r8

    str r7, [r1], #4
    str r5, [r1], #4
    str r6, [r1], #4
    str r4, [r1], #4
    pop         {r2-r12,r14}
    bx          lr

.align 2
@ void void test_present(uint32_t* in,  uint32_t* out);
.global test_present
.type   test_present,%function
test_present:
    push {r2-r12,r14}
    ldm         r0, {r4-r7}
    sbox_present       r4, r5, r6, r7, r8, r9, r10, r11, r12

    stm         r1, {r4-r7}
    pop         {r2-r12,r14}
    bx          lr

.align 2
@ void void test_piccolo(uint32_t* in,  uint32_t* out);
.global test_piccolo
.type   test_piccolo,%function
test_piccolo:
    push {r2-r12,r14}
    ldm         r0, {r4-r7}    
    sbox_piccolo      r4, r5, r6, r7, r8
    
    str r7, [r1], #4
    str r6, [r1], #4
    str r5, [r1], #4
    str r4, [r1], #4
    pop         {r2-r12,r14}
    bx          lr


.align 2
@ void void test_pyjamask128(uint32_t* in,  uint32_t* out);
.global test_pyjamask128
.type   test_pyjamask128,%function
test_pyjamask128:
    push {r2-r12,r14}
    ldm         r0, {r4-r7}    
    sbox_pyjamask128     r4, r5, r6, r7, r8

    str r5, [r1], #4
    str r4, [r1], #4
    str r6, [r1], #4
    str r7, [r1], #4
    pop         {r2-r12,r14}
    bx          lr
