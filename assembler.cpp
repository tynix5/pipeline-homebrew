
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <bitset>
#include <math.h>

#define SUCCESS				1
#define FAIL				-1


#define NUM_REGS			8

/* Instruction Types */
#define NOP					0
#define IMMEDIATE			1
#define BRANCH				2
#define REGISTER			3
#define RAM					4
#define STACK				5
#define JUMP				6
#define DIRECTIVE			7

#define BYTE_ADDRESSABLE	1
//#define WORD_ADDRESSABLE	1

#ifdef BYTE_ADDRESSABLE
	#define WORD_SIZE_BYTES		1
#elif WORD_ADDRESSABLE
	#define WORD_SIZE_BYTES	2
#endif

/* Labels, instructions, and registers are not case-sensitive */
/* Branch instructions either have a label or number */

using namespace std;

enum opcodes {

	nop = 0,
	mvi,
	addi,
	subi,
	andi,
	ori,
	cmpi,
	bra,	//
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

string valid_registers[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"};

vector <string> label_names;			// names of labels
vector <int> label_addresses;			// addresses of labels
vector <string> tokens;					// tokenizer

/* Receives string and returns corresponding opcode for instruction */
int string_to_opcode(string str);
/* Receives string and returns decoded register # */
int string_to_register(string str);
/* Takes in binary, hex, or decimal string and returns integer representation */
int string_to_imm(string str);
/* Takes in decimal, binary, hex number in string form and returns raw number */
bool is_valid_immediate(string str, int max_size);
/* Takes in a string, checks if it is a valid label name */
bool is_valid_label(string label);
/* Takes in a label (valid or invalid) and returns its absolute address or -1 if invalid */
int get_label_address(string label);
/* Receives instruction and returns its type (NOP, IMMEDIATE, BRANCH, REGISTER) */
int get_instruction_type(int opcode);

/* Opens program file and reads each line, checking for syntax and valid label names, registers, instructions, and immediate values */
int parse_file(ifstream& prog_file);
/* Receives one line from program line and checks for syntax, updating the program counter for every line; adds each token to a vector */
int parse_instruction(char * line, int line_num, int& pc);
/* Check to see if there is a missing operand in instruction */
bool parse_instruction_early_exit(int instruction_type, int iteration, int line_num);
/* Checks to see if label has a valid name and notes its name and location in memory using the program counter */
int parse_label(string label, int line_num, int& pc);
/* Checks to see if org or dcx directive, otherwise, exit */
int parse_directive(string dir, int line_num, int& pc);
/* Break the parsed file into separate tokens on each line */
int tokenize_file(fstream &parser_file, fstream &token_file);
/* Convert each token to binary and write to new bin file */
int assemble_file(ofstream &bin);
/* Add token to token vector */
void add_token(string tok);


int main() {

	ofstream bin;		
	ifstream prog;

	bin.open("machine_code.bin", ios::out | ios::trunc | ios::binary);			// open new write file, replace old file with new file
	prog.open("fibonacci.txt", ios::in);			// open program file for read-only mode

	
	if (!bin.is_open()) {
		cout << "\nUnable to open write file";
		return FAIL;
	}

	if (!prog.is_open()) {
		cout << "\nUnable to open program file";
		return FAIL;
	}


	if (parse_file(prog) == FAIL)
		goto exit;
	if (assemble_file(bin) == FAIL)
		goto exit;

	cout << "success";


exit:

	bin.close();
	prog.close();

	return 0;
}


int string_to_opcode(string str) {

	if (!str.compare("nop"))
		return opcodes::nop;
	else if (!str.compare("mvi"))
		return opcodes::mvi;
	else if (!str.compare("addi"))
		return opcodes::addi;
	else if (!str.compare("subi"))
		return opcodes::subi;
	else if (!str.compare("andi"))
		return opcodes::andi;
	else if (!str.compare("ori"))
		return opcodes::ori;
	else if (!str.compare("cmpi"))
		return opcodes::cmpi;
	else if (!str.compare("bra"))
		return opcodes::bra;
	else if (!str.compare("bne"))
		return opcodes::bne;
	else if (!str.compare("beq"))
		return opcodes::beq;
	else if (!str.compare("bhs"))
		return opcodes::bhs;
	else if (!str.compare("blo"))
		return opcodes::blo;
	else if (!str.compare("bge"))
		return opcodes::bge;
	else if (!str.compare("blt"))
		return opcodes::blt;
	else if (!str.compare("bvs"))
		return opcodes::bvs;
	else if (!str.compare("bvc"))
		return opcodes::bvc;
	else if (!str.compare("mvr"))
		return opcodes::mvr;
	else if (!str.compare("addr"))
		return opcodes::addr;
	else if (!str.compare("subr"))
		return opcodes::subr;
	else if (!str.compare("andr"))
		return opcodes::andr;
	else if (!str.compare("orr"))
		return opcodes::orr;
	else if (!str.compare("notr"))
		return opcodes::notr;
	else if (!str.compare("cmp"))
		return opcodes::cmp;
	else if (!str.compare("ldr"))
		return opcodes::ldr;
	else if (!str.compare("ldrb"))
		return opcodes::ldrb;
	else if (!str.compare("str"))
		return opcodes::str;
	else if (!str.compare("strb"))
		return opcodes::strb;
	else if (!str.compare("push"))
		return opcodes::push;
	else if (!str.compare("pop"))
		return opcodes::pop;
	else if (!str.compare("call"))
		return opcodes::call;
	else if (!str.compare("ret"))
		return opcodes::ret;
	else if (!str.compare("jmp"))
		return opcodes::jmp;
	else	
		return -1;
}


int string_to_register(string str) {

	for (int i = 0; i < NUM_REGS; i++)
		if (!str.compare(valid_registers[i]))		// if there is a match...
			return stoi(str.substr(1), nullptr, 10);		// remove the 'r' and return string as integer
		
	return -1;			// return -1 if invalid register
}


int string_to_imm(string imm) {

	int immediate;

	if (imm.find("0b") != string::npos && imm.find("0b") == 0)		
		immediate = stoi(imm.substr(2), nullptr, 2);		// binary
	else if (imm.find("0x") != string::npos)
		immediate = stoi(imm.substr(2), nullptr, 16);		// hex
	else 
		immediate = stoi(imm, nullptr, 10);		// decimal

	return immediate;
}


bool is_valid_immediate(string imm, int max_size) {

	int immediate;

	if (imm.find("0b") != string::npos && imm.find("0b") == 0) {		// binary number...

		string bin_substring = imm.substr(2);		// remove "0b" from string

		if (bin_substring.find_first_not_of("01") != string::npos)		// contains numbers other than 0 and 1... fail
			return false;
		
		immediate = stoi(bin_substring, nullptr, 2);		// get bin # (base 2) in decimal
		
	}
	else if (imm.find("0x") != string::npos && imm.find("0x") == 0) {			// hex number...

		string hex_substring = imm.substr(2);			// remove "0x" from string

		if (hex_substring.find_first_not_of("0123456789abcdef") != string::npos)		// contains non hex characters... fail
			return false; 
		

		immediate = stoi(hex_substring, nullptr, 16);		// get hex # (base 16) in decimal
	}
	else if (imm.find_first_not_of("0123456789-") != string::npos)			// not hex, not bin, and there is a non-number character or blank space... fail
		return false;
	else			// decimal number...
		immediate = stoi(imm, nullptr, 10);		// get # representation of string
		

	if (immediate > ((int) pow(2, max_size) - 1) || immediate < (-1 * (int) pow(2, max_size - 1)))			// if # does not fit into max_size bits (signed and unsigned)
		return false;
	

	return true;
}


bool is_valid_label(string label) {

	int first_char = label.find_first_of("abcdefghijklmopqrstuvwxyz_");
	int first_num = label.find_first_of("012345679");

	if (first_char != string::npos && first_num != string::npos) {		// if the operand has both letters and number
		if (first_num < first_char)			// if label starts with a number
			return false;
	}
	else if (first_char == string::npos && first_num != string::npos)			// the operand only has numbers...	fail (no explicit jumps allowed)
		return false;
	else if (first_char == string::npos && first_num == string::npos)			// if branch operand is not memory address or label, error...
		return false;
	else if (string_to_opcode(label) != -1 || string_to_register(label) != -1)		// reserved word
		return false;
	

	return true;			// if all test cases pass, valid label
}


int get_label_address(string label) {

	for (int i = 0; i != label_names.size(); i++)		// search through list of label names...
		if (!label_names.at(i).compare(label))		// if there is a match
			return label_addresses.at(i);
	

	// if label not found...
	cout << "\nLine []: Error... Label not found\n";
	return -1;
	
}


int get_instruction_type(int opcode) {

	if (opcode == NOP)
		return NOP;
	if (opcode >= opcodes::mvi && opcode <= opcodes::cmpi)
		return IMMEDIATE;
	if (opcode >= opcodes::bra && opcode <= opcodes::bvc)
		return BRANCH;
	if (opcode >= opcodes::mvr && opcode <= opcodes::cmp)
		return REGISTER;
	if (opcode >= opcodes::ldr && opcode <= opcodes::strb)
		return RAM;
	if (opcode >= opcodes::push && opcode <= opcodes::ret)
		return STACK;
	if (opcode == opcodes::jmp)
		return JUMP;
	else 
		return -1;
}


int parse_file(ifstream& prog_file) {

	if (!prog_file.is_open())
		return FAIL;


	string line;
	int line_count = 0;
	int pc = 0;

	while (getline(prog_file, line)) {

		line_count++;

		if (line.find(';') != -1)				// delete comments
			line.erase(line.find(';'));
		
		if (line.empty() || line.find_first_not_of(" \t\n") == string::npos)				// empty line, ignore
			continue;


		int found = line.find_first_not_of(" \t");			// ignore whitespace


		if (found != string::npos) {			// if the line has text...


			line = line.substr(found);			// remove leading whitespace...

			if (line.find_last_not_of(" \t\n") != string::npos)
				line = line.substr(0, line.find_last_not_of(" \t\n") + 1);			// remove trailing whitespace from line

			transform(line.begin(), line.end(), line.begin(), ::tolower);				// convert to all lowercase for comparison


			string cpy = line;		// create copy of line

			if (line.at(0) == '.') {				// if first non-whitespace character is a '.', it is a label

				cpy = cpy.substr(1);		// remove '.'

				if ((parse_label(cpy, line_count, pc)) == FAIL)
					return FAIL;

				continue;			// don't write label to parser file

			} else if (line.at(0) == '#') {			// if first character is '#', it is a directive
				
				if (parse_directive(cpy, line_count, pc) == FAIL)
					return FAIL;	

			} else {				// else, it is an instruction...

				if (parse_instruction(cpy.data(), line_count, pc) == FAIL)		// this method modifies the string
					return FAIL;

			}

		}

	}

	return SUCCESS;
}


int parse_label(string label, int line_num, int& pc) {

	
	int first_char = label.find_first_of("abcdefghijklmopqrstuvwxyz_");
	int first_num = label.find_first_of("012345679");

	bool invalid = false;

	if (first_char == string::npos)		// if no characters in string...
		invalid = true;

	if (first_char != string::npos && first_num != string::npos)		// if the operand has both letters and numbers
		if (first_num < first_char)			// if label starts with a number
			invalid = true;

	if (label.find_first_not_of("abcdefghijklmnopqrstuvwxyz_0123456789") != string::npos)		// if label name contains special characters
		invalid = true;

	if (string_to_opcode(label) != -1 || string_to_register(label) != -1)		// if label has name of instruction or register name (reserved name/keyword)
		invalid = true;
		
	if (invalid) {

		cout << "\nLine " << line_num << ": Error... Invalid label name [" << label << "]" << endl;
		return FAIL;
	}

	// if label name is valid
	label_names.push_back(label);			// save label name
	label_addresses.push_back(pc);				// save label address

	return SUCCESS;
}


int parse_instruction(char * line, int line_num, int& pc) {

	char * token = strtok(line, " ");			// get instruction

	int operand_count = 0;			// 0 = instruction, 1 = register/immediate, 2 = register/immediate, 3+ = illegal
	int opcode;

	int instruction_type = 0;		// will be updated on first iteration of loop

	while (token != NULL) {

		if (operand_count == 0) {			// if instruction is expected...
			
			opcode = string_to_opcode(token);			// get instruction opcode
			instruction_type = get_instruction_type(opcode);		// get type of instruction

			/* Increment program counter based on instruction type (16 bit program counter, 16 bit words) */			
			if (instruction_type == NOP)
				pc += 2;								// 1 byte for opcode, 1 empty byte (1 word)
			else if (instruction_type == IMMEDIATE)
				pc += 2;								// 1 byte for opcode + register, 1 byte for immediate (1 word)
			else if (instruction_type == BRANCH) 
				pc += 2;								// 1 byte for opcode, 1 byte for relative displacement (1 word)
			else if (instruction_type == REGISTER)
				pc += 2;								// 1 byte for opcode + register, 1 byte for register (1 word)
			else if (instruction_type == RAM)			
				pc += 4;								// 1 byte for opcode, 1 for RAM address (1 word)
			else if (instruction_type == JUMP)
				pc += 4;								// 1 byte opcode, 1 empty byte, 2 bytes absolute address
			else if (opcode == -1) {			// if not an instruction...

				cout << "\nLine " << line_num << ": Error... Invalid instruction" << endl;
				return FAIL;
			} 
			else {

				cout << "\nThis tests for unsupported instructions...";
				return FAIL;
			}

			
			add_token(token);
			token = strtok(NULL, ",");		// get next operand (if no "," found, gives the rest of line)
		}


		if (operand_count == 1) {

			string token_string(token);

			if (instruction_type == NOP) {			// nop should have nothing after (if it did, it would have returned after end of first loop)

				cout << "\nLine " << line_num << ": Error... Unexpected [] after nop" << endl;
				return FAIL;

			} else if (instruction_type == IMMEDIATE || instruction_type == REGISTER || instruction_type == RAM) {		// immediate, register, load instructions both take registers for first operand
					
				if (string_to_register(token) == -1) {		// if register doesn't match r0-r3

					cout << "\nLine " << line_num << ": Error... Invalid register" << endl;
					return FAIL;
				}
				
			} else if (instruction_type == BRANCH || instruction_type == JUMP) {		// branch should just have an address after in the form of a label...

				if (!is_valid_label(token_string)) {

					cout << "\nLine " << line_num << ": Error... Expected label after branch instruction" << endl;
					return FAIL;
				}
			}

			add_token(token);
			token = strtok(NULL, " ");			// next operand should have a space delimiter
		}

		else if (operand_count == 2) {

			string token_string(token);

			 if (instruction_type == IMMEDIATE || instruction_type == RAM) {		// immediate and RAM instructions should have a number for second operand

				if (!is_valid_immediate(token_string, 8)) {			// if immediate is not a valid number... fail
					cout << "\nLine " << line_num << ": Error... Expected # (0b..., 0x..., dec)" << endl;
					return FAIL;
				}
				
			} else if (instruction_type == BRANCH || instruction_type == JUMP) {			// there should not be a second operand for branch instructions

				cout << "\nLine " << line_num << ": Error... Unexpected [] after branch" << endl;
				return FAIL;

			} else if (instruction_type == REGISTER) {		// register instructions expect another register for second operand

				if (string_to_register(token) == -1) {		// if register doesn't match r0-r3

					cout << "\nLine " << line_num << ": Error... Invalid register" << endl;
					return FAIL;
				}

			}

			add_token(token);
			token = strtok(NULL, " ");			// get next token, should be NULL if instruction has correct syntax
		}

		else if (operand_count == 3) {				// this will only be reached by instructions with incorrect syntax

			cout << "\nLine " << line_num << ": Error... Unexpected [] after last operand" << endl;
			return FAIL;
			
		}

		operand_count++;
		
	}

	/* Chceck to see if instructions have missing operands */
	if (parse_instruction_early_exit(instruction_type, operand_count, line_num))
		return FAIL;

	return SUCCESS;

}


bool parse_instruction_early_exit(int instruction_type, int iteration, int line_num) {

	if (instruction_type == IMMEDIATE || instruction_type == REGISTER || instruction_type == RAM) {
		if (iteration == 2) {

			cout << "\nLine " << line_num << ": Error... Missing intermediate value" << endl;
			return true;
		}
	}

	if (instruction_type == BRANCH || instruction_type == JUMP) {
		if (iteration == 1) {

			cout << "\nLine " << line_num << ": Error... Missing label/branch address" << endl;
			return true;
		}
	}

	return false;
}


int parse_directive(string dir, int line_num, int&pc) {

	char * line = dir.data();			// get C string
	char * token = strtok(line, " ");		// break up line into individual tokens separated by spaces

	int itr = 0;

	while (token != NULL) {

		string token_string(token);

		if (itr == 0) {			// should be expecting a "#org" or "#dcw" directive

			if (token_string.compare("#org")) {			// for now, if not "#org", fail

				cout << "\nLine " << line_num << ": Error... Expected directive" << endl;
				return FAIL;
			}

			token = strtok(NULL, " ");			// get next address
		}
		else if (itr == 1) {			// check for valid immediate address

			if (!is_valid_immediate(token_string, 16)) {

				cout << "\nLine " << line_num << ": Error... Expected 16 bit address" << endl;
				return FAIL;
			}

			int new_pc = string_to_imm(token_string);			// get memory address of directive
			int range = (new_pc - pc) / 2;			// get # of 16 bit words between current PC and new PC

			if (range < 0) {			// overwriting memory...

				cout << "\nLine " << line_num << ": Error... Org directive overwriting memory" << endl;
				return FAIL;
			}

			for (int i = 0; i < range; i++)
				tokens.push_back("nop");

			pc = new_pc;		// update program counter

			token = strtok(NULL, " ");			// should be NULL after this
		}
		else {		// #org should not have more arguments

			cout << "\nLine " << line_num << ": Error... Unexpected [] after address" << endl;
			return FAIL;
		}

		itr++;
	}

	if (itr == 1) {		// early exit

		cout << "\nLine " << line_num << ": Error... Expected address" << endl;
		return FAIL;
	}
	return SUCCESS;
}


void add_token(string tok) {

	tokens.push_back(tok);
}


int assemble_file(ofstream &bin) {

	
	string line;
	int pc = 0;
	
	auto token_num = tokens.begin();

	while (token_num != tokens.end()) {

		int opcode = string_to_opcode(*(token_num++));
		int instruction_type = get_instruction_type(opcode);
		__int8 high_byte = (__int8) opcode << 3;			// 5 bits for opcode, 3 bits for register

		if (instruction_type == NOP) {
			
			pc += 2;

			bin << high_byte;
			bin << (__int8) 0;
		}
		else if (instruction_type == IMMEDIATE) {

			pc += 2;

			__int8 r_dest = (__int8) string_to_register(*(token_num++));		// get destination register

			bin << (__int8)  (high_byte | r_dest);		// combine register and opcode into final opcode
			bin << (__int8) string_to_imm(*(token_num++));		// write to file

		}
		else if (instruction_type == BRANCH) {

			pc += 2;

			bin << high_byte;		// write branch instruction

			int label_address = get_label_address(*(token_num++));		// get label address

			if (label_address == -1) {

				cout << "\nLine []: Error... Invalid reference to label [" << line << "]" << endl;
				return FAIL;
			}
			
			int offset = (label_address - pc) / WORD_SIZE_BYTES;		// get relative displacement of branch, then divide by # of individually accessible bytes
			
			if (offset > 127 || offset < -128) {

				cout << "\nError... Offset out of range" << endl;
				return FAIL;
			}

			bin << (__int8) offset;			// write relative displacement byte to file

		}
		else if (instruction_type == REGISTER) {

			pc += 2;

			__int8 r_dest = string_to_register(*(token_num++));			// first register
			__int8 r_src = string_to_register(*(token_num++));				// second register

			bin << (__int8) (high_byte | r_dest);			// write first byte of opcode to file
			bin << r_src;			// write source operand to next byte in memory
		}
		else if (instruction_type == RAM) {

			pc += 4;

			__int8 r_ds = string_to_register(*(token_num++));			// register to load memory into

			bin << (__int8) (high_byte | r_ds);
			bin << (__int8) 0;

			string addr = *(token_num++);

			if (is_valid_immediate(addr, 16)) {			// if immediate address...

				int immediate = string_to_imm(addr);		// get immediate from file

				if (opcode == opcodes::str || opcode == opcodes::ldr) {

					if (immediate % 2 == 1)	{		// don't allow misaligned writes when using str and ldr

						cout << "\nError... Misaligned writes and reads not allowed" << endl;
						return FAIL;
					}
				}
				

				bin << (__int8) (immediate >> 8);			// high byte of address
				bin << (__int8) immediate;				// low byte of address
			} 
			
		}
		else if (instruction_type == JUMP) {

			pc += 4;

			bin << high_byte;		// write jump instruction
			bin << (__int8) 0;			// write empty byte

			int label_address = get_label_address(*(token_num++));		// get absolute address of label

			if (label_address == -1) {			// if label not found...

				cout << "\nLine []: Error... Invalid reference to label [" << line << "]" << endl;
				return FAIL;
			}

			bin << (__int8) (label_address >> 8);			// get upper byte of memory address
			bin << (__int8) (label_address);		// get low byte of memory address
			
		}
		else {

			cout << "\nHow did you even get here?" << endl;
			return FAIL;
		}

	}
		

	return SUCCESS;
}
