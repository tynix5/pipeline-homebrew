# pipeline-homebrew
16 bit pipelined computer with custom ISA

Instruction Set

    Opcode (5 bits) - Instruction(bytes)        Syntax                     Description
    -----------------------------------------------------------------------------------
        -   00000   - nop(2)                   nop                         no operation
        -   00001   - mvi(2)                   mvi rd, #                   rd = #
        -   00010   - addi(2)                  addi rd, #                  rd = rd + #
        -   00011   - subi(2)                  subi rd, #                  rd = rd - #
        -   00100   - andi(2)                  andi rd, #                  rd = rd & #
        -   00101   - ori(2)                   ori rd, #                   rd = rd | #
        -   00110   - cmpi(2)                  cmpi rd, #                  Set flags for rd - #
        -   00111   - bra(2)                   bra <label>                 relative branch always
        -   01000   - bne(2)                   bne <label>                 relative branch not equal (Z = 0)
        -   01001   - beq(2)                   beq <label>                 relative branch if equal (Z = 1)
        -   01010   - bhs(2)                   bhs <label>                 relative branch if higher or same (unsigned; C = 1)
        -   01011   - blo(2)                   blo <label>                 relative branch if lower (unsigned; C = 0)
        -   01100   - bge(2)                   bge <label>                 relative branch if greater than or equal (signed; C = N)
        -   01101   - blt(2)                   blt <label>                 relative branch if less than (signed; C != N)
        -   01110   - bvs(2)                   bvs <label>                 relative branch if V set (V = 1)
        -   01111   - bvc(2)                   bvc <label>                 relative branch if V clear (V = 0)
        -   10000   - mvr(2)                   mvr rd, rs                  rd = rs
        -   10001   - addr(2)                  addr rd, rs                 rd = rd + rs
        -   10010   - subr(2)                  subr rd, rs                 rd = rd - rs
        -   10011   - andr(2)                  andr rd, rs                 rd = rd & rs
        -   10100   - orr(2)                   orr rd, rs                  rd = rd | rs
        -   10101   - notr(2)                  notr rd, rs                 rd = ~rs
        -   10110   - cmp(2)                   cmp rd, rs                  Set flags for rd - rs
        -   10111   - ldr(4)                   ldr rd, <address>           load word into rd from <address> in RAM (no misaligned reads)
        -   11000   - ldrb(4)                  ldrb rd, <address>          load byte into rd from <address> in RAM
        -   11001   - str(4)                   str rd, <address>           str word from rd into <address> in RAM (no misaligned writes)
        -   11010   - strb(4)                  strb rd, <address>          store byte from rd into <address> in RAM
        -   11011   - push(2)                  push rd                     push rd onto stack
        -   11100   - pop(2)                   pop rd                      pop data from top of stack into rd
        -   11101   - call(4)                  call <label>                push PC onto stack and move label address into PC
        -   11110   - ret(2)                   ret                         pop stack into PC
        -   11111   - jmp(4)                   jmp <label>                 absolute branch always

    Syntax
        - Assembler is not case-sensitive
        - Big endian

    Labels and Branches
        - Labels have syntax ".label"
        - Label names must start with a character, can use uppercase and lowercase letters (assembler, however, is not case sensitive --> see below), numbers, but must not contain special characters or be a keyword (opcodes, registers, etc.)
            -labelone and labelOne are equivalent labels
        - Relative branches must be between -128 and 127 bytes from label
        - Absolute branches (jmp) and function call (call) use absolute addressing, allowing for entire coverage over program memory
        -Ex:
                ...
                beq done    ; if Z = 1, exit
                bra somewhere   ; if Z = 0, do something else

            .done
                bra done    ; do nothing

    Registers
        - 8 16-bit registers
        - Naming convention as follows: r0, r1, r2, r3, r4, r5, r6, r7 (or R0, R1, R2, R3, R4, R5, R6, R7)

    Operands
        - rd register is embedded into last 3 bits of MSB
        - Opcode becomes most significant 5 bits, rd becomes least significant 3 bits
        -Ex:
                mvi r2, 15
                0x82 0x0f
        - rs register is embedded into LSB of instruction
        -Ex:
                mvr r2, r3
                0x82 0x03

    Architecture
        - 3 stage pipeline (fetch, decode/execute, writeback)
        - 16 bit words and register width, byte addressable memory

    Stack
        - Stack pointer is 8 bits wide, allowing for 128 ints to be pushed onto stack
        - Stack does not allow for 1 byte to be pushed onto stack
        - Stack pointer starts at 0 and incremements (by 2) with every push
        - Stack resides in RAM from 0x0000 to 0x0100
