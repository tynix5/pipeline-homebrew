#include <iostream>
#include <fstream>

using namespace std;


/* uROM1 */
#define OS      0 << 0
#define IMS     1 << 3
#define ALUI    1 << 4
#define STALL   1 << 5
#define LDI     1 << 6
#define PCS     1 << 7
#define DECSP   1 << 8


/* uROM2 */
#define WEN     1 << 0
#define J       1 << 1
#define BRS     1 << 2
#define RW      1 << 3
#define RR      1 << 4
#define RBYTE   1 << 5
#define FLUSH   1 << 6
#define INCSP   1 << 7
#define SPS     1 << 8
#define RET_C   1 << 9
#define CALL_C  1 << 10


/* ALU operations */
#define ADD     0 << 0
#define AND     1 << 0
#define OR      2 << 0
#define B_ID    3 << 0
#define NOT     4 << 0
#define SUB     5 << 0


#define N       1 << 11
#define Z       1 << 10
#define C       1 << 9
#define V       1 << 8


enum opcodes {

	nop = 0,
	mvi,
	addi,
	subi,
	andi,
	ori,
	cmpi,
	bra,	
	bne,
	beq,
    bhs,
	blo,
	bge,
	blt,
	bvs,
	bvc,
	mvr,
	addr,
	subr,
	andr,
	orr,
	notr,
	cmp,
	ldr,
	ldrb,
	str,
	strb,
	push,
	pop,
	call,
	ret,
	jmp
};


unsigned long op_ctrl[32][2] = {    {0, 0},                                     // nop
                                    {B_ID | IMS | ALUI, WEN},                   // mvi  
                                    {ADD | IMS | ALUI, WEN},                    // addi
                                    {SUB | IMS | ALUI, WEN},                    // subi
                                    {AND | IMS | ALUI, WEN},                    // andi
                                    {OR | IMS | ALUI, WEN},                     // ori
                                    {SUB | IMS | ALUI, 0},                      // cmpi
                                    {ADD | IMS | ALUI | STALL | PCS, J| BRS}, // bra
                                    {ADD | IMS | ALUI | PCS, BRS},
                                    {ADD | IMS | ALUI | PCS, BRS},
                                    {ADD | IMS | ALUI | PCS, BRS},
                                    {ADD | IMS | ALUI | PCS, BRS},
                                    {ADD | IMS | ALUI | PCS, BRS},
                                    {ADD | IMS | ALUI | PCS, BRS},
                                    {ADD | IMS | ALUI | PCS, BRS},
                                    {ADD | IMS | ALUI | PCS, BRS},              // bvc
                                    {B_ID | ALUI, WEN},                         // mvr 
                                    {ADD | ALUI, WEN},                          // addr
                                    {SUB | ALUI, WEN},                          // subr
                                    {AND | ALUI, WEN},                          // andr
                                    {OR | ALUI, WEN},                           // orr
                                    {NOT | ALUI, WEN},                          // notr
                                    {SUB | ALUI, 0},                            // cmp
                                    {LDI | STALL, WEN | RR},                                  // ldr
                                    {LDI | STALL, WEN | RR | RBYTE},                          // ldrb
                                    {LDI | STALL, RW},                                  // str
                                    {LDI | STALL, RW | RBYTE},                          // strb
                                    {0, RW | INCSP | SPS},             // push
                                    {DECSP, RR | WEN | SPS},             // pop
                                    {LDI | STALL, J | RW | INCSP | SPS | CALL_C | FLUSH},             // call
                                    {DECSP, RR | J | FLUSH | SPS | RET_C},             // ret
                                    {STALL | LDI, J}                           // jmp
                                    };   


int main() {

    ofstream dx_rom, dx_rom2, wb_rom, wb_rom2;

    dx_rom.open("dx_rom.bin", ios::binary | ios::out | ios::trunc);      // open output files for roms, overwrite files
    dx_rom2.open("dx_rom2.bin", ios::binary | ios::out | ios::trunc);
    wb_rom.open("wb_rom.bin", ios::binary | ios::out | ios::trunc);
    wb_rom2.open("wb_rom2.bin", ios::binary | ios::out | ios::trunc);

    for (int i = 0; i < 256; i++) {

        dx_rom << (unsigned char) op_ctrl[i >> 3][0];
        dx_rom2 << (unsigned char) (op_ctrl[i >> 3][0] >> 8);
    }

    for (int i = 0; i < 4096; i++) {

        int flags = i >> 8;
        int opcode = ((unsigned char) i) >> 3;

        int n_flag = !!(i & N);
        int z_flag = !!(i & Z);
        int c_flag = !!(i & C);
        int v_flag = !!(i & V);

        unsigned long new_ctrl = op_ctrl[((unsigned char) i) >> 3][1];

        if (opcode >= opcodes::bra && opcode <= opcodes::bvc) {


            if (z_flag) {

                if (opcode == opcodes::beq || opcode == opcodes::bge)
                    new_ctrl |= J| FLUSH;

            } else {

                if (opcode == opcodes::bne)
                    new_ctrl |= J| FLUSH;
            }


            if (c_flag) {

                if (opcode == opcodes::bhs)
                    new_ctrl |= J| FLUSH;

            } else {

                if (opcode == opcodes::blo)
                    new_ctrl |= J| FLUSH;
            }

            if (v_flag) {

                if (opcode == opcodes::bvs)
                    new_ctrl |= J| FLUSH;
            } else {

                if (opcode == opcodes::bvc)
                    new_ctrl |= J| FLUSH;
            }

            if (n_flag == v_flag) {

                if (opcode == opcodes::bge)
                    new_ctrl |= J| FLUSH;
            } else {

                if (opcode == opcodes::blt)
                    new_ctrl |= J| FLUSH;
            }
        }
        

        wb_rom << (unsigned char) new_ctrl;
        wb_rom2 << (unsigned char) (new_ctrl >> 8);
    }
    

    dx_rom.close();
    dx_rom2.close();
    wb_rom.close();
    wb_rom2.close();
}