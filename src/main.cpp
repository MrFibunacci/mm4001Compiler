#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>

typedef uint8_t byte;

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

constexpr std::array<std::string_view, 4> FlagNames = {
	"B",
	"C",
	"G",
	"E"
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
std::vector<std::string> split(const std::string& s, const std::string& delimiter) {
	size_t pos_start = 0, pos_end;
	const size_t delim_len = delimiter.length();
	std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        std::string token = s.substr(pos_start, pos_end - pos_start);
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
int readNumber(const std::string &s)
{
	auto base = 10;
	//check if string starts with "0x" then use hexadecimal
	if(s.rfind("0x", 0) == 0)
		base = 16;

	return std::stoi(s, nullptr, base);
}

bool isValInArray(std::array<std::string_view, 4> arr, const std::string &_val)
{
	const std::string_view *foo = std::find(arr.begin(), arr.end(), _val);

	return (foo != std::end(arr));
}

int getIDOfElement(const std::array<std::string_view, 4> &arr, const std::string &_val)
{
	return (std::find(arr.begin(), arr.end(), _val)) - arr.begin();
}

template <typename Iter>
size_t index_of(Iter first, Iter last, const typename std::iterator_traits<Iter>::value_type& x)
{
	size_t i = 0;
	while (first != last && *first != x)
		++first, ++i;
	return i;
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
			// get arguments
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
			// check if argument is valid number
			if (int immediate = readNumber(s[1]); immediate >= 1 << 4) {
				std::cout << "ERROR: immediate value to large in Line: " << line << std::endl;
			} else {
				// combine number and opc and push to variable
				writeToBinary.push_back(opc << 4 | immediate);
			}
		} break;

		case STO: {
			byte autoInc;
			if (s[2] == "AutoInc") {
				autoInc = 1;
			} else if (s[2] == "AutoDec") {
				autoInc = 2;
			} else {
				autoInc = 0;
			}

			writeToBinary.push_back(opc << 4 | autoInc << 2 | getIDOfElement(RegNames, s[1]));
		} break;

		case LOD: {
			byte autoInc;
			if (s[2] == "AutoInc") {
				autoInc = 1;
			} else if (s[2] == "AutoDec") {
				autoInc = 2;
			} else if (s.size() == 2) {
				autoInc = 0;
			} else {
				std::cout << "ERROR: Unknown operand in Line: " << line << std::endl;
			}

			writeToBinary.push_back(opc << 4 | getIDOfElement(RegNames, s[1]) << 2 | autoInc);
		} break;

		case ALU: {
			size_t aluInst = index_of(ALUOpcNames.begin(), ALUOpcNames.end(), s[1]);

			writeToBinary.push_back(opc << 4 | aluInst);
		} break;

		case JIF:
		case JIT: {
			byte flagNibble = 0;

			if (s[1] == "B")		// borrow (overflow on subtraction)
				flagNibble = 1 << 3;
			else if (s[1] == "C")	// carry (overflow on addition)
				flagNibble = 1 << 2;
			else if (s[1] == "G")	// greater (on cmp)
				flagNibble =  1 << 1;
			else if (s[1] == "E")	// equal (on cmp)
				flagNibble = 1 << 0;
			else
				std::cout << "ERROR: Unknown Flag in Line: " << line << std::endl;

			writeToBinary.push_back(opc << 4 | flagNibble);
		} break;
		
		default:
			std::cout << "ERROR: unknown OPC -> " << OpcNames[opc] << std::endl;
			exit(1);
		}

		std::cout << "Opcode: " << OpcNames[opc] << std::endl;
		//for(int i = 0; i < s.size(); i++) { printf("%s\n", s[i].c_str()); }
	}

	binFile.write(writeToBinary.data(), writeToBinary.size());
	
	asmFile.close();
	binFile.close();
	return 0;
}
