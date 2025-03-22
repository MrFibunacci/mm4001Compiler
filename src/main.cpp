#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <algorithm>

typedef u_int8_t byte;

enum Reg
{
	A, 		// Akkumulator
	D,		// Datenregister
	PL,		// Pointer-Low
	PH,		// Pointer-High
	FLG,	// Flagregister
	PC,		// ProgramCounter
	SP,		// Stackpointer
};

enum Flags
{
	B = 1 << 3,	// borrow (overflow on subtraction)
	C = 1 << 2,	// carry (overflow on addition)
	G = 1 << 1,	// greater (on cmp)
	E = 1 << 0  // equal (on cmp)
};

enum Opcodes
{
	NOP,
	MOV,
	INA,
	STO,
	LOD,
	ALU,
	JIT,	// Jump if true
	JIF,	// Jump if false
	PUSH,
	POP,
	TAK,
	PRN,
	CLS,
	HLT,
	TRP,
	RES,
};

enum ALU_Opcodes 
{
	ADD,
	SUB,
	INC,
	DEC,
	CMP,
	NOT,
	AND,
	NAND,
	OR,
	NOR,
	XOR,
	XNOR,
	SHL,
	SHR,
	ROL,
	ROR,
};

constexpr std::array<std::string_view, 16> OpcNames = {
	"NOP",
	"MOV",
	"INA",
	"STO",
	"LOD",
	"ALU",
	"JIT",
	"JIF",
	"PUSH",
	"POP",
	"TAK",
	"PRN",
	"CLS",
	"HLT",
	"TRP",
	"RES",
};

constexpr std::array<std::string_view, 16> ALUOpcNames = {
	"ADD",
	"SUB",
	"INC",
	"DEC",
	"CMP",
	"NOT",
	"AND",
	"NAND",
	"OR",
	"NOR",
	"XOR",
	"XNOR",
	"SHL",
	"SHR",
	"ROL",
	"ROR",
};

constexpr std::array<std::string_view, 4> RegNames = {
	"A",
	"D",
	"PL",
	"PH",
};

// split string by delimiter
std::vector<std::string> split(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

std::ifstream openFile(const char* filePath)
{	
	std::ifstream file(filePath);

	if (!file.is_open())
	{
		printf("Failed to open file: %s", filePath);
		exit(1);
	}

	return file;
}

//Reads string number and returns as integer
int readNumber(std::string s)
{
	auto base = 10;
	//check if string starts with "0x" then use hexadecimal
	if(s.rfind("0x", 0) == 0)
		base = 16;

	return std::stoi(s, nullptr, base);
}

bool isValInArray(std::array<std::string_view, 4> arr, std::string _val)
{
	std::string_view *foo = std::find(arr.begin(), arr.end(), _val);

	return (foo != std::end(arr));
}

int getIDOfElement(std::array<std::string_view, 4> arr, std::string _val)
{
	return (std::find(arr.begin(), arr.end(), _val)) - arr.begin();
}

int main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		printf("mm4001-compiler [asm-file] [bin-file]\n");
		exit(2);
	}

	std::ifstream asmFile = openFile(argv[1]);
	std::ofstream binFile (argv[2], std::ofstream::binary);

	std::string line;

	std::vector<char> writeToBinary;

	while (std::getline(asmFile, line)) 
	{
		std::vector<std::string> s = split(line, " ");

		// get the opcode to save in binary file
		int opc = (std::find(OpcNames.begin(), OpcNames.end(), s[0])) - OpcNames.begin();

		switch (opc)
		{
		case Opcodes::NOP:
		case Opcodes::RES:
		case Opcodes::POP:
		case Opcodes::PUSH:
		case Opcodes::HLT:
			// shift opc 4bit left and save in bin file
			writeToBinary.push_back(opc << 4);
			break;

		case Opcodes::MOV: {
			// get arguemts
			std::vector<std::string> regs = split(s[1], ",");

			if (!isValInArray(RegNames, regs[1])) {
				std::cout << "ERROR: Register " << regs[1] << " does not exist." << std::endl;
				exit(1);
			}

			if (!isValInArray(RegNames, regs[0])) {
				std::cout << "ERROR: Register " << regs[0] << " does not exist." << std::endl;
				exit(1);
			}
			
			byte fromReg = getIDOfElement(RegNames, regs[1]);
			byte toReg = getIDOfElement(RegNames, regs[0]);

			writeToBinary.push_back(opc << 4 | fromReg | toReg << 2);
		} break;

		case Opcodes::INA: {
			// cast text number to binary 
			int immediate = readNumber(s[1]);
			// check if argument is valid number
			if (immediate >= 1 << 4) {
				std::cout << "ERROR: immediate value to large in Line: " << line << std::endl;
			} else {
				// combine number and opc and push to variable
				writeToBinary.push_back(opc << 4 | immediate);
			}
		} break;
		
		default:
			std::cout << "ERROR: unkown OPC -> " << OpcNames[opc] << std::endl;
			exit(1);
			break;
		}

		std::cout << "Opcode: " << OpcNames[opc] << std::endl;
		//for(int i = 0; i < s.size(); i++) { printf("%s\n", s[i].c_str()); }
	}

	binFile.write(writeToBinary.data(), sizeof writeToBinary);
	
	asmFile.close();
	binFile.close();
	return 0;
}
