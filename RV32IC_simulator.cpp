#include <iostream>
#include <fstream>
#include "stdlib.h"
#include <iomanip>
#include <string>
#include <cmath>
using namespace std;

int regs[32] = { 0 };   //array of registers 
unsigned int pc = 0x0;  //location 0x00000000 in the main memory
bool flag = false;
char memory[8 * 1024] = { 0 };	// only 8KB of memory located at address 0



void emitError(const char *s)
{
	cout << s;
	system("pause");
	exit(0);
}
void printPrefix(unsigned int instA, unsigned int instW) {
	cout << "0x" << hex << std::setfill('0') << std::setw(8) << instA << "\t0x" << std::setw(8) << instW;
}
void instDecExec(unsigned int instWord)
{
	unsigned int rd, rs1, rs2, funct3, funct7, opcode;
	unsigned int I_imm, S_imm, B_imm, U_imm, J_imm;
	unsigned int address;
	regs[0] = 0;
	unsigned int instPC = pc - 4;

	opcode = instWord & 0x0000007F;
	rd = (instWord >> 7) & 0x0000001F;
	funct3 = (instWord >> 12) & 0x00000007;
	rs1 = (instWord >> 15) & 0x0000001F;
	rs2 = (instWord >> 20) & 0x0000001F;
	funct7 = (instWord >> 25) & 0x0000007F;

	// — inst[31] — inst[30:25] inst[24:21] inst[20]
	I_imm = ((instWord >> 20) & 0x7FF) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
	S_imm = ((instWord >> 20) & 0x7E0) | ((instWord >> 7) & 0x1F) | (((instWord >> 31) ? 0xFFFFF800 : 0x0));
	B_imm = ((instWord >> 7) & 0x1E) | ((instWord >> 20) & 0x7E0) | ((instWord << 4) & 0x0800) | (((instWord >> 31) ? 0xFFFFF000 : 0x0));
	U_imm = ((instWord) & 0XFFFFF000);
	J_imm = (instWord & 0x000FF000) | ((instWord >> 9) & 0x0800) | ((instWord >> 20) & 0x07FE) | (((instWord >> 31) ? 0xFFF00000 : 0x0));

	printPrefix(instPC, instWord);

	if (opcode == 0x33) {		// R Instructions
		switch (funct3) {
		case 0:  // ADD or SUB
			if (funct7 == 32)    //SUB
			{
				cout << "\tSUB\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = regs[rs1] - regs[rs2];
			}
			else    //ADD
			{
				cout << "\tADD\tx" << dec<< rd << ", x" << dec <<rs1 << ", x" <<  dec << rs2 << "\n";
				regs[rd] = regs[rs1] + regs[rs2];
			}
			break;

		case 1: //SLL
			cout << "\tSLL\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
			regs[rd] = regs[rs1] << regs[rs2];
			break;

		case 2: //SLT    
			cout << "\tSLT\tx" << dec << rd << ", x" << dec <<(int)rs1 << ", x" << dec <<(int)rs2 << "\n";
			if ((int)regs[rs1] < (int)regs[rs2])
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;

		case 3: //SLTU
			cout << "\tSLTU\tx" <<  dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
			if ((unsigned int)regs[rs1] < (unsigned int)regs[rs2])
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;

		case 4: //XOR
			cout << "\tXOR\tx" << dec << rd << ", x" << dec<< rs1 << ", x" << dec << rs2 << "\n";
			regs[rd] = regs[rs1] ^ regs[rs2];
			break;

		case 5: //SRL or SRA   
			if (funct7 == 32)
			{  //SRA
				cout << "\tSRA\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = regs[rs1] >> regs[rs2];
				unsigned int last_bit = regs[rs1] >> 31;
				if (last_bit == 0)
					last_bit = 0x00000000;
				else
				{
					last_bit = 0xFFFFFFFF;
					last_bit = (last_bit << (32 - regs[rs2]));
				}
				regs[rd] = regs[rd] | last_bit;
			}
			else
			{  //SRL
				cout << "\tSRL\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
				regs[rd] = regs[rs1] >> regs[rs2];
			}
			break;

		case 6: //OR
			cout << "\tOR\tx" << dec << rd << ", x" << dec << rs1 << ", x" << dec << rs2 << "\n";
			regs[rd] = regs[rs1] | regs[rs2];
			break;

		case 7: //AND
			cout << "\tAND\tx" << dec << rd << ", x" << dec <<  rs1 << ", x" << dec << rs2 << "\n";
			regs[rd] = regs[rs1] & regs[rs2];
			break;

		default:
			cout << "\tUnkown I Instruction \n";
		}
	}

	else if (opcode == 0x13)
	{	// I instructions
		switch (funct3) {
			//ADDI
		case 0:	cout << "\tADDI\tx" << dec << (int)rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n"; 
			regs[rd] = regs[rs1] + (int)I_imm;
			break;

			//SLLI
		case 1:	cout << "\tSLLI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] << (int)I_imm;
			break;

			//SLTI
		case 2:	cout << "\tSLTI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			if (regs[rs1] < (int)I_imm)
				regs[rd] = 1;
			else
				regs[rd] = 0;
			break;

			//SLTIU
		case 3:	cout << "\tSLTIU\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			if ((unsigned int)regs[rs1] < I_imm)
				regs[rd] = 0x1;
			else
				regs[rd] = 0x0;
			break;

			//XORI
		case 4:	cout << "\tXORI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] ^ (int)I_imm;
			break;

		case 5:	

			if (funct7 == 0x0)
				//SRLI
			{
				cout << "\tSRLI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				regs[rd] = regs[rs1] >> (int)I_imm;
				break;
			}
			if (funct7 == 0x20)
				//SRAL
			{
				cout << "\tSRAL\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
				regs[rd] = (regs[rs1] >> (int)I_imm) | ((regs[rs1] >> 31) ? (0xFFFFFFFF << (32 - (int)I_imm)) : 0x0);
				break;
			}
			else cout << "\tUnkown I Instruction \n";

			break;

			//ORI
		case 6:	cout << "\tORI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] | (int)I_imm;

			break;

			//ANDI
		case 7:	cout << "\tANDI\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";
			regs[rd] = regs[rs1] & (int)I_imm;

			break;
		default:
			cout << "\tUnkown I Instruction \n";
		}
	}

	else if (opcode == 0x3) //laod instructions 
	{
		if (funct3 == 0)//LB
		{
			cout << "\tLB\tx" << dec << rd << ", " << hex << "0x" << (int)I_imm << "(x" << dec << rs1 << ")" << endl;
			regs[rd] = 0;
			regs[rd] = (unsigned char)memory[regs[rs1] + (int)I_imm] | ((((unsigned char)memory[regs[rs1] + (int)I_imm] >> 7) ? 0xFFFFFF00 : 0x0));

		}
		else if (funct3 == 1)//LH
		{
			cout << "\tLH\tx" << dec << rd << ", " << hex << "0x" << (int)I_imm << "(x" << dec << rs1 << ")" << endl;
			regs[rd] = 0;
			regs[rd] = ((unsigned char)memory[regs[rs1] + (int)I_imm]) | ((unsigned char)memory[regs[rs1] + (int)I_imm + 1] << 8)
				| (((((unsigned char)memory[regs[rs1] + (int)I_imm + 1]) >> 15) ? 0xFFFF0000 : 0x0));
		}
		else if (funct3 == 2)//LW
		{
			cout << "\tLW\tx" << dec << rd << ", " << hex << "0x" << (int)I_imm << "(x" << dec << rs1 << ")" << endl;
			regs[rd] = ((unsigned char)memory[regs[rs1] + (int)I_imm]) | (((unsigned char)memory[regs[rs1] + (int)I_imm + 1]) << 8)
				| (((unsigned char)memory[regs[rs1] + (int)I_imm + 2]) << 16)
				| (((unsigned char)memory[regs[rs1] + (int)I_imm + 3]) << 24);
		}
		else if (funct3 == 4)//LBU
		{
			cout << "\tLBU\tx" << dec << rd << ", " << hex << "0x" << (int)I_imm << "(x" << dec << rs1 << ")" << endl;
			regs[rd] = 0;
			regs[rd] = (unsigned char)memory[regs[rs1] + (int)I_imm];
		}
		else if (funct3 == 5) //LHU
		{
			cout << "\tLHU\tx" << dec << rd << ", " << hex << "0x" << (int)I_imm << "(x" << dec << rs1 << ")" << endl;
			regs[rd] = 0;
			regs[rd] = ((unsigned char)memory[regs[rs1] + (int)I_imm]) | ((unsigned char)memory[regs[rs1] + (int)I_imm + 1] << 8);
		}
	}

	else if (opcode == 0x37)   // U instruction (LUI)
	{
		cout << "\tLUI\tx" << dec << rd << ", " << dec << U_imm << "\n";
		regs[rd] = U_imm;
	}
	else if (opcode == 0x17)   // U instruction (AUIPC)
	{
		cout << "\tAUIPC\tx" << dec << rd << ", " << dec << U_imm << "\n";
		regs[rd] = pc + U_imm;
	}
	else if (opcode == 0x63)   // B instructions 
	{
		switch (funct3)
		{
		case 0:
			cout << "\tBEQ\tx" << dec << rs1 << ", x" << dec << rs2 << ", " << hex << "0x" << (int)B_imm << endl;
			if ((int)regs[rs1] == (int)regs[rs2])
				pc += (int)B_imm - 4;
			break;
		case 1:

			cout << "\tBNE\tx" << dec << rs1 << ", x" << dec << rs2 << ", " << "0x" << (int)B_imm << endl;
			if ((int)regs[rs1] != (int)regs[rs2])
				pc += (int)B_imm - 4;
			break;
		case 4:
			cout << "\tBLT\tx" << dec << (int)rs1 << ", x" << dec << (int)rs2 << ", " << hex << "0x" << (int)B_imm << endl;
			if ((int)regs[rs1] < (int)regs[rs2])
				pc += (int)B_imm - 4;
			break;
		case 5:
			cout << "\tBGE\tx" << dec << (int) rs1 << ", x" << dec << (int)rs2 << ", " << hex << "0x" << (int)B_imm << endl;
			if ((int)regs[rs1] >= (int)regs[rs2])
				pc += (int)B_imm - 4;
			break;
		case 6:
			cout << "\tBLTU\tx" << dec << (int) rs1 << ", x" << dec << (int) rs2 << ", " << hex << "0x" << (int)B_imm << endl;
			if ((unsigned int)regs[rs1] < (unsigned int)regs[rs2])
				pc += (int)B_imm - 4;
			break;
		case 7:
			cout << "\tBGEU\tx" << dec << (int ) rs1 << ", x" << dec << (int) rs2 << ", " << hex << "0x" << (int)B_imm << endl;
			if ((unsigned int)regs[rs1] >= (unsigned int)regs[rs2])
				pc += (int)B_imm - 4;
			break;
		default:
			cout << "\tUnkown B Instruction \n";
		}


	}
	else if (opcode == 0x23) //store instructions 
	{
		switch (funct3) {
		case 0:
			cout << "\tSB\tx" << dec << rs2 << ", " << hex << "0x" << (int)S_imm << "(x" << dec << rs1 << ")" << endl;
			memory[regs[rs1] + S_imm] = regs[rs2];
			break;
		case 1:
			cout << "\tSH\tx" << dec << rs2 << ", " << hex << "0x" << (int)S_imm << "(x" << dec << rs1 << ")" << endl;
			memory[regs[rs1] + S_imm] = regs[rs2];
			memory[regs[rs1] + S_imm + 1] = regs[rs2] >> 8;
			break;
		case 2:
			cout << "\tSW\tx" << dec << rs2 << ", " << hex << "0x" << (int)S_imm << "(x" << dec << rs1 << ")" << endl;
			memory[regs[rs1] + S_imm] = regs[rs2];
			memory[regs[rs1] + S_imm + 1] = regs[rs2] >> 8;
			memory[regs[rs1] + S_imm + 2] = regs[rs2] >> 16;
			memory[regs[rs1] + S_imm + 3] = regs[rs2] >> 24;
			break;
		default:
			cout << " \tUnkown Store Instruction \n";
			break;

		}
	}
	else if (opcode == 0x6F)   // J instruction (JAL) 
	{
		cout << "\tJAL\tx" << dec << rd << ", " << hex << "0x" << (int)J_imm << endl;
		regs[rd] = pc;
		pc += (int)J_imm - 4;
	}
	else if (opcode == 0x67)   // J instruction (JALR) 
	{
		regs[rd] = pc + 4;
		pc = regs[rs1] + (int)I_imm;
		cout << "\tJALR\tx" << dec << rd << ", x" << dec << rs1 << ", " << hex << "0x" << (int)I_imm << "\n";

	}
	else if (opcode == 0x73)
	{
		if (funct3 == 0x0)
		{
			if ((int)I_imm == 0x0)
				cout << "\tECALL\t" << "\n";
			if (regs[17] == 10)
			{
				flag = true;
				cout << "Exit  " << endl;
			}

			else if (regs[17] == 5)
				cin >> regs[10];

			else if (regs[17] == 1)
				cout << dec << regs[10] << "\n";

			else if (regs[17] == 4)
			{
				int nullstring = regs[10];
				while (memory[nullstring] != '\0')
				{
					cout << memory[nullstring];
					nullstring++;
				}
				cout << endl;
			}
			else if (regs[17] == 8)
			{
				string temp;
				int add = regs[10];
				cin >> temp;
				temp += '\0';
				for (int i = 0; i < temp.length(); i++)
				{
					memory[add] = temp[i];
					add++;
				}
			}
		}
		else cout << "\tUnkown ECAll Service \n";
	}
	else
	{
		cout << "\tUnkown Instruction \n";
	}
}

void instDecExecCom(unsigned int instWordC)
{
	unsigned int rd, rd_, rs1, rs1_, rs2, rs2_, funct2, funct3, funct4, funct6, opcode;
	unsigned int CI_imm, CIW_imm, CL_imm, CS_imm, CB_imm, CJ_imm, CI2_imm, lwsp_imm, swsp_imm;
	unsigned int address;
	unsigned int instPC = pc - 2;

	opcode = instWordC & 0x0003;
	rd = (instWordC >> 7) & 0x001F;
	rd_ = (instWordC >> 2) & 0x0003;
	rs1 = (instWordC >> 7) & 0x001F;
	rs1_ = (instWordC >> 7) & 0x0003;
	rs2 = (instWordC >> 2) & 0x001F;
	rs2_ = (instWordC >> 2) & 0x0003;
	funct2 = (instWordC >> 5) & 0x0003;
	funct3 = (instWordC >> 13) & 0x0007;
	funct4 = (instWordC >> 12) & 0x000F;
	funct6 = (instWordC >> 10) & 0x003F;

	lwsp_imm = ((((instWordC >> 4) & 0x0007) | ((instWordC >> 9) & 0x0008) | ((instWordC << 2) & 0x0030)) << 2);

	swsp_imm = ((((instWordC >> 9) & 0x000f) | ((instWordC >> 3) & 0x0030)) << 2);

	CI_imm = ((instWordC >> 2) & 0x001F) | (((instWordC >> 12) & 0x0001) ? 0xFFFFFFE0 : 0x0); //addi
	CI2_imm = ((instWordC >> 2) & 0x001F); //slli, srli,srai
	CJ_imm = ((instWordC >> 2) & 0x000E) | ((instWordC >> 7) & 0x0010) | ((instWordC << 3) & 0x0020) | ((instWordC >> 1) & 0x0040)
		| ((instWordC << 1) & 0x0080) | ((instWordC >> 1) & 0x0300) | ((instWordC << 2) & 0x0400) |
		(((instWordC >> 12) & 0x0001) ? 0xFFFFF800 : 0x0); //J , JAL
	CL_imm = ((instWordC >> 4) & 0X0004) | ((instWordC >> 7) & 0X0038) | ((instWordC << 1) & 0X0040); //LW , SW
	CB_imm = ((instWordC >> 2) & 0x0006) | ((instWordC >> 7) & 0x0018) | ((instWordC << 3) & 0x0020) |
		((instWordC << 1) & 0x00a0) | ((instWordC >> 12) ? 0xFFFFFF00 : 0x0);

	printPrefix(instPC, instWordC);

	if (opcode == 0x0)
	{
		switch (funct3)
		{
		case 2:   //LW
			cout << "\tC.LW\tx" << dec << rd_ << ", " << hex << "0x" << (int)CL_imm << "(x" << dec <<rs1 << ")" << endl;
			regs[rd_] = ((unsigned char)memory[regs[rs1_] + (int)CL_imm]) | (((unsigned char)memory[regs[rs1_] + (int)CL_imm + 1]) << 8);
			break;
		case 6:   //SW
			cout << "\tC.SW\tx" << dec << rs2_ << ", " << hex << "0x" << (int)CL_imm << "(x" << dec << rs1_ << ")" << endl;
			memory[regs[rs1_] + CL_imm] = regs[rs2_];
			memory[regs[rs1_] + CL_imm + 1] = regs[rs2_] >> 8;
			break;
		default:
			cout << "\tUnkown C store Instruction \n";
		}
	}

	else if (opcode == 0x1)
	{
		switch (funct3)
		{
		case 0:
			/*if (regs[rs1] == 0)   //NOP
			{
				cout << "\tC.NOP\n";
				pc = pc + 4;
			}
			else   //ADDI
			{*/
			if (CI_imm != 0)
			{
				cout << "\tC.ADDI\tx" << dec << rd << ", x" << dec << rd << ", " << hex << "0x" << (int)CI_imm << "\n";
				regs[rd] = regs[rd] + CI_imm;
			}
			else
				cout << "\tReserved\n";
			//}
			break;
		case 1:    //JAL
			regs[1] = pc;
			pc += (int)CJ_imm - 2;
			cout << "\tC.JAL\tx1, " << hex << "0x" << CJ_imm << endl;
			break;
		case 2:    //LI , when rd̸=x0; the code points with rd=x0 encode HINTs
			cout << "\tC.LI\tx" << dec << rd << ", " << hex << "0x" << (int)CI_imm << "\n";
			regs[rd] = CI_imm;
			break;
		case 3:	//LUI 
			if (CI_imm != 0 && regs[rd] != 0 && regs[rd] != 2)
			{
				unsigned int lui_imm = CI_imm << 12;
				cout << "\tC.LUI\tx" << dec << rd << ", " << hex << "0x" << (int)lui_imm << "\n";
				regs[rd] = lui_imm;
			}
			else
				cout << "reserved\n";
			break;
		case 4:
		{unsigned int check = (instWordC >> 10) & 0x3;
		if (check == 0)      //SRLI
		{
			cout << "\tC.SRLI\tx" << dec << rd << ", x" << dec << rd << ", " << hex << "0x" << (int)CI2_imm << "\n";
			regs[rd_] = regs[rd_] >> CI2_imm;
		}
		else if (check == 1) //SRAI
		{
			cout << "\tC.SRAI\tx" << dec << rd << ", x" << dec << rd << ", " << hex << "0x" << (int)CI2_imm << "\n";
			regs[rd_] = (int)regs[rd_] >> CI2_imm;
		}
		else if (check == 2) //ANDI
		{
			cout << "\tC.ANDI\tx" << dec << rd << ", x" << dec << rd << ", " << hex << "0x" << (int)CI_imm << "\n";
			regs[rd_] = regs[rd_] & CI_imm;
		}
		else
		{
			if (funct6 == 0x35 && funct2 == 0x0)  //SUB
			{
				cout << "\tC.SUB\tx" << dec << rd << ", x" << dec << rd << ", x" << dec << rs2 << "\n";
				regs[rd_] = regs[rd_] - regs[rs2_];
			}
			if (funct6 == 0x35 && funct2 == 0x1)  //XOR
			{
				cout << "\tC.XOR\tx" << dec << rd << ", x" << dec << rd << ", x" << dec << rs2 << "\n";
				regs[rd_] = regs[rd_] ^ regs[rs2_];
			}
			if (funct6 == 0x35 && funct2 == 0x2)  //OR
			{
				cout << "\tC.OR\tx" << dec << rd << ", x" << dec << rd << ", x" << dec << rs2 << "\n";
				regs[rd_] = regs[rd_] | regs[rs2_];
			}
			if (funct6 == 0x35 && funct2 == 0x3)  //AND
			{
				cout << "\tC.AND\tx" << dec << rd << ", x" << dec << rd << ", x" << dec << rs2 << "\n";
				regs[rd_] = regs[rd_] & regs[rs2_];
			}
		}
		}
		break;
		case 5: // J
			regs[rd] = pc;
			pc += (int)CJ_imm - 2;
			cout << "\tC.J\tx" << dec << rd << ", " << hex << "0x" << CJ_imm  << endl;
			break;
		case 6:  //BEQZ
			if (regs[rs1_] == 0)
			{
				cout << "\tBEQZ\tx" << dec << rs1_ << ", " << hex << "0x" << CB_imm << "\n";
				instPC += CB_imm;
			}
			break;
		case 7:  //BNEZ
			if (regs[rs1_] != 0)
			{
				cout << "\tBNEZ\tx" << dec << rs1_ << ", " << hex << "0x" << CB_imm << "\n";
				instPC += CB_imm;
			}
			break;
		default:
			cout << "\tUnkown C branch Instruction \n";
		}
	}

	else if (opcode == 0x2)
	{
		switch (funct3)
		{
		case 0:   //SLLI
			cout << "\tC.SLLI\tx" << dec << rd << ", x" << dec << rd << ", x" << CI2_imm << "\n";
			regs[rd] = regs[rd] << CI2_imm;
			break;

		case 2:   //LWSP
			cout << "\tC.LWSP\tx" << dec << rd << ", " << hex << "0x" << (int)lwsp_imm << "(x2)" << endl;
			regs[rd] = ((unsigned char)memory[regs[2] + (int)lwsp_imm * 4]) | (((unsigned char)memory[regs[2] + (int)lwsp_imm * 4 + 1]) << 8);
			break;

		case 4:
			switch (funct4)
			{
			case 0x8:
				if (CI2_imm != 0)  //C.MV
				{
					cout << "\tC.MV\tx" << dec << rd << ", x" << dec << rs2 << "\n";
					regs[rd] = regs[rs2];
				}
				else   // C.JR
				{
					cout << "\tC.JR\tx0" << ", 0(" << dec << rs1 << ")" << "\n";
					pc = regs[rs1];
				}
				break;
			case 0x9:
				if (regs[rs2] != 0)  //C.ADD
				{
					cout << "\tC.ADD\tx" << dec << rd << ", x" << dec << rd << ", x" << dec << rs2 << "\n";
					regs[rd] = regs[rd] + regs[rs2];
				}
				else  // C.JALR
				{
					cout << "\tC.JALR\tx1" << ", 0(" << dec << rs1 << ")" << "\n";
					regs[1] = pc + 2;
					pc = regs[rs1];
				}
				break;
			default:
				cout << "\tUnkown C.J Instruction \n";
			}
			break;

		case 6:   //SWSP
			cout << "\tC.SWSP\tx" << dec << rs2 << ", " << hex << "0x" << (int)swsp_imm << "(x2)" << endl;
			memory[regs[2] + swsp_imm * 4] = regs[rs2];
			memory[regs[2] + swsp_imm * 4 + 1] = regs[rs2] >> 8;
			break;
		default:
			cout << "\tUnkown C. sp Instruction \n";
		}

	}
	else
		cout << "\tUnkown C Instruction \n";
}

int main(int argc, char *argv[]) {

	unsigned int instWord = 0, instWordC = 0;
	ifstream inFile;
	ofstream outFile;

	if (argc < 1) emitError("use: rv32i_sim <machine_code_file_name>\n"); //when wil this comment be printed

	inFile.open(argv[1], ios::in | ios::binary | ios::ate);

	if (inFile.is_open())
	{
		int fsize = inFile.tellg();

		inFile.seekg(0, inFile.beg);
		if (!inFile.read((char *)memory, fsize)) emitError("Cannot read from input file\n");

		while (!flag)
		{
			unsigned int check = (unsigned char)memory[pc];

			if ((check & 0x00000003) == 0x3)
			{
				instWord = (unsigned char)memory[pc] |
					(((unsigned char)memory[pc + 1]) << 8) |
					(((unsigned char)memory[pc + 2]) << 16) |
					(((unsigned char)memory[pc + 3]) << 24);
				pc += 4;
				instDecExec(instWord);
			}
			else if ((check & 0x00000003) == 0x0 | (check & 0x00000003) == 0x1 | (check & 0x00000003) == 0x2)
			{
				instWord = (unsigned char)memory[pc] |
					(((unsigned char)memory[pc + 1]) << 8);
				pc += 2;
				instDecExecCom(instWord);
			}
			else
			{
				emitError("Neither 32-bit Nor 16-bit Instruction\n");
			}

		}
		// dump the registers
		for (int i = 0; i < 32; i++)
			cout << "x" << dec << i << ": \t" << "0x" << hex << std::setfill('0') << std::setw(8) << regs[i] << "\n";

	}
	else emitError("Cannot access input file\n");

	system("pause");
	exit(0);
}