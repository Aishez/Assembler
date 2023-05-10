/*****************************************************************************
TITLE: Emulator
AUTHOR: Aishez singh - 2101CS06
Declaration of Authorship
This cpp file, emu.cpp, is part of the assignment of CS209/210 at the
department of Computer Science and Engg, IIT Patna .
*****************************************************************************/
#include <bits/stdc++.h>
using namespace std;
#define F first
#define S second
#define pa pair<int, int>
#define sz(x) x.size()
#define pb push_back
#define all(v) v.begin(), v.end()

vector<pair<pair<string, string>, string>> container_instruc;
long long int powr16[15];
int REG_A = 8483536;
int REG_B = 134518156;
int pc = 0;
int sp = 134518166;
int tr = 0;
uint32_t n, size, instruc_cnt;
uint32_t inp_opc[200005];
map<int, int> memory; // memory default value is def
bool stop_exec = false;
vector<string> container_trace;
vector<string> container_read;
vector<string> container_write;
vector<string> container_before;
vector<string> container_after;

void initialixe()
{
	int powr16[15] = {1, 16}; // Initialize the array with the first two values
	for (int i = 2; i < 15; i++)
	{
		powr16[i] = 16 * powr16[i - 1];
	}

	vector<pair<string, string>> instructions = {
		{"ldc      ", "0      "},
		{"adc      ", "1      "},
		{"ldl      ", "2      "},
		{"stl      ", "3      "},
		{"ldnl     ", "4      "},
		{"stnl     ", "5      "},
		{"add      ", "6      "},
		{"sub      ", "7      "},
		{"shl      ", "8      "},
		{"shr      ", "9      "},
		{"adj      ", "10     "},
		{"a2sp     ", "11     "},
		{"sp2a     ", "12     "},
		{"call     ", "13     "},
		{"return   ", "14     "},
		{"brz      ", "15     "},
		{"brlz     ", "16     "},
		{"br       ", "17     "},
		{"HALT     ", "18     "}};

	for (auto &instr : instructions)
	{
		if (instr.first == "SET      ")
		{
			container_instruc.push_back({instr, "value"});
		}
		else if (instr.first == "HALT     ")
		{
			container_instruc.push_back({instr, ""});
		}
		else
		{
			container_instruc.push_back({instr, "offset"});
		}
	}
}


int getHextoDec(char ch)
{
	if (ch >= '0' && ch <= '9')
	{
		return ch - '0';
	}
	else if (ch >= 'A' && ch <= 'F')
	{
		return ch - 'A' + 10;
	}
	else if (ch >= 'a' && ch <= 'f')
	{
		return ch - 'a' + 10;
	}
	else
	{
		// ch is not a valid hexadecimal digit
		// handle the error as appropriate for your use case
	}
}

string opcode_to_hex_string(uint32_t num, int bits)
{
	stringstream ss;
	ss << setfill('0') << setw((bits + 3) / 4) << hex << num;
	string ret = ss.str();
	if (ret.size() < bits)
	{
		ret = string(bits - ret.size(), '0') + ret;
	}
	return ret;
}

int hexToIntInst(string str)
{
	int ret = 0;
	for (char c : str)
	{
		ret <<= 4;
		if (isdigit(c))
		{
			ret += c - '0';
		}
		else
		{
			ret += toupper(c) - 'A' + 10;
		}
	}
	return ret;
}

int hex_to_int_val(const string &hex_str)
{
	const string hex_digits = "0123456789ABCDEF";
	const int num_hex_digits = hex_digits.size();
	const int num_bits_per_hex_digit = 4;

	int decimal_val = 0;
	int num_hex_digits_processed = 0;

	for (auto it = hex_str.rbegin(); it != hex_str.rend(); ++it)
	{
		const char c = toupper(*it);
		const int hex_digit_val = hex_digits.find(c);

		if (hex_digit_val == string::npos)
		{
			// invalid hex character
			return 0;
		}

		const int bits_to_shift = num_bits_per_hex_digit * num_hex_digits_processed;
		decimal_val += hex_digit_val << bits_to_shift;

		++num_hex_digits_processed;
	}

	// Check if the most significant bit is 1
	const int msb_mask = 1 << (num_bits_per_hex_digit * hex_str.size() - 1);
	if (decimal_val & msb_mask)
	{
		// Perform two's complement to get the negative value
		const int num_bits = num_bits_per_hex_digit * hex_str.size();
		decimal_val -= 1 << num_bits;
	}

	return decimal_val;
}

string curr_trace = "", num_hex_str;
// Function to implement all instructions
void Functions(int inst, int val)
{
	// Define local variables
	string curr_read, curr_write;

	// Handle each instruction type
	switch (inst)
	{
	case 0: // ldc function
		curr_trace += "ldc " + num_hex_str;
		REG_B = REG_A;
		REG_A = val;
		break;
	case 1: // adc function
		curr_trace += "adc " + num_hex_str;
		REG_A += val;
		break;
	case 2: // ldl function
		curr_trace += "ldl " + num_hex_str;
		REG_B = REG_A;
		curr_read = "Reading memory[" + opcode_to_hex_string(sp + val, 8) + "]";
		curr_read += " finds ";
		if (memory.find(sp + val) != memory.end())
		{
			REG_A = memory[sp + val];
		}
		else
		{
			REG_A = 0;
		}
		curr_read += opcode_to_hex_string(REG_A, 8);
		container_read.push_back(curr_read);
		break;
	case 3: // stl function
		curr_trace += "stl " + num_hex_str;
		curr_write = "Writing memory[" + opcode_to_hex_string(sp + val, 8) + "] was ";
		if (memory.find(sp + val) == memory.end())
		{
			curr_write += opcode_to_hex_string(0, 8);
		}
		else
		{
			curr_write += opcode_to_hex_string(memory[sp + val], 8);
		}
		memory[sp + val] = REG_A;
		REG_A = REG_B;
		curr_write += " now " + opcode_to_hex_string(memory[sp + val], 8);
		container_write.push_back(curr_write);
		break;
	case 4: // ldnl function
		curr_trace += "ldnl " + num_hex_str;
		curr_read = "Reading memory[" + opcode_to_hex_string(REG_A + val, 8) + "]";
		curr_read += " finds ";
		if (memory.find(REG_A + val) != memory.end())
		{
			REG_A = memory[REG_A + val];
		}
		else
		{
			REG_A = 0;
		}
		curr_read += opcode_to_hex_string(REG_A, 8);
		container_read.push_back(curr_read);
		break;
	case 5: // stnl function
		curr_trace += "stnl " + num_hex_str;
		curr_write = "Writing memory[" + opcode_to_hex_string(REG_A + val, 8) + "] was ";
		if (memory.find(REG_A + val) == memory.end())
		{
			curr_write += opcode_to_hex_string(0, 8);
		}
		else
		{
			curr_write += opcode_to_hex_string(memory[REG_A + val], 8);
		}
		memory[REG_A + val] = REG_B;
		curr_write += " now " + opcode_to_hex_string(memory[REG_A + val], 8);
		container_write.push_back(curr_write);
		break;
	case 6: // add function
		curr_trace += "add ";
		REG_A = REG_B + REG_A;
		break;
	case 7: // sub function
		curr_trace += "sub ";
		REG_A = REG_B - REG_A;
		break;
	case 8: // shl function
		curr_trace += "shl ";
		REG_A = REG_B << REG_A;
		break;
	case 9: // shr function
		curr_trace += "shr ";
		REG_A = REG_B >> REG_A;
		break;
	case 10: // shr function
		curr_trace += "adj ";
		curr_trace += num_hex_str;
		sp = sp + val;
		break;

	case 11:
		curr_trace += "a2sp ";
		sp = REG_A;
		REG_A = REG_B;
		break;
	case 12:
		curr_trace += "sp2a ";
		REG_B = REG_A;
		REG_A = sp;
		break;
	case 13:
		curr_trace += "call ";
		curr_trace += num_hex_str;
		REG_B = REG_A;
		REG_A = pc;
		pc = pc + val;
		break;
	case 14:
		curr_trace += "return ";
		pc = REG_A;
		REG_A = REG_B;
		break;
	case 15:
		curr_trace += "brz ";
		curr_trace += num_hex_str;
		if (REG_A == 0)
			pc = pc + val;
		break;
	case 16:
		curr_trace += "brlz ";
		curr_trace += num_hex_str;
		if (REG_A < 0)
			pc = pc + val;
		break;
	case 17:
		curr_trace += "br ";
		curr_trace += num_hex_str;
		pc = pc + val;
		break;
	case 18:
		curr_trace += "HALT ";
		stop_exec = true;
		break;
	default:
		curr_trace += "SET ";
		curr_trace += num_hex_str;
	}
}

// cstyle string
// The terminating condition of the for loop is the expression *p which checks whether the character pointed to by the pointer p is non-zero.
// Since C-style strings are null-terminated, which means they end with a null character ('\0'),
// the loop will terminate when the null character is reached, which has a value of zero in C++.

void ShowInstructions()
{
	const char *instructions = R"(usage: emu [options] file.o
-trace  show instruction trace
-read   show memory reads
-write  show memory writes
-before show memory dump before execution
-after  show memory dump after execution
-wipe   wipe written flags before execution
-isa    display ISA)";

	for (const char *p = instructions; *p; ++p)
	{
		if (*p == '\n')
			cout << endl;
		else
			cout << *p;
	}
}

void Exec_Before()
{
	std::stringstream ss;
	for (int i = 0; i < tr; i += 4)						// tr = size of the memory array
	{
		ss << opcode_to_hex_string(i, 8) << " ";
		for (int j = 0; j < 4; j++)
		{
			ss << opcode_to_hex_string(memory[i + j], 8) << " ";
		}
		container_before.push_back(ss.str());			// ss.str() = string representation of the object of class string steam
		ss.str("");
	}
}

void Exec_Trace()
{
	while (!stop_exec)
	{
		pc++;
		instruc_cnt++;
		n = inp_opc[pc - 1];
		string mach = opcode_to_hex_string(n, 8);
		string operand = mach.substr(0, 6);
		string instruc = mach.substr(6, 2);
		int num = hex_to_int_val(operand);
		num_hex_str = opcode_to_hex_string(num, 8);

		curr_trace = "";
		curr_trace += "PC=";
		curr_trace += opcode_to_hex_string(pc - 1, 8);
		curr_trace += ", ";
		curr_trace += "SP=";
		curr_trace += opcode_to_hex_string(sp, 8);
		curr_trace += ", ";
		curr_trace += "A=";
		curr_trace += opcode_to_hex_string(REG_A, 8);
		curr_trace += ", ";
		curr_trace += "B=";
		curr_trace += opcode_to_hex_string(REG_B, 8);
		curr_trace += "";
		int inst = hexToIntInst(instruc);
		Functions(inst, num);
		curr_trace += "\n";
		container_trace.push_back(curr_trace);
	}
}

void Exec_After()
{
	string curr_after = "";
	int i = 0;
	while (i < tr)
	{
		if (i % 4 == 0)
		{
			if (!curr_after.empty())
			{
				container_after.push_back(curr_after);
				curr_after.clear();
			}
			curr_after += opcode_to_hex_string(i, 8) + " " + opcode_to_hex_string(memory[i], 8) + " ";
		}
		else
		{
			curr_after += opcode_to_hex_string(memory[i], 8) + " ";
		}
		i++;
	}
	if (!curr_after.empty())
	{
		container_after.push_back(curr_after);
	}
}

// argc = no. of command line arguments
// argv = array of strings that contain the actuall argument

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		ShowInstructions();
		return 0;
	}

	string command = argv[1];
	string filename = argv[2]; // object file name

	if (command != "-trace" && command != "-read" && command != "-write" && command != "-before" && command != "-after" && command != "-isa")
	{
		ShowInstructions();
		return 0;
	}

	initialixe();

	FILE *fp;
	fp = fopen(filename.c_str(), "rb");

	// uint32_t is an unsigned integer data type that is 32 bits wide
	// The "u" in uint32_t stands for "unsigned," meaning that the variable can only hold non-negative values.
	// The "32" indicates the number of bits allocated to the variable.

	//  The first argument &n specifies the memory address of the buffer where the read data is to be stored. Here, it's the memory address of the variable n.
	// The second argument sizeof(uint32_t) specifies the size of each element in bytes that is to be read. uint32_t is an unsigned 32-bit integer data type, so sizeof(uint32_t) is the size of an unsigned 32-bit integer, which is usually 4 bytes.
	// The third argument 1 specifies the number of elements to read, in this case, it's 1.
	// So, the fread() function reads a single 32 - bit unsigned integer from the file pointed by fp and stores it in the variable n.

	uint32_t n;
	while (fread(&n, sizeof(uint32_t), 1, fp) != 0)
	{
		inp_opc[tr++] = n;
		memory[tr - 1] = n;
	}

	fclose(fp);

	int length = filename.size();
	string basename = filename.substr(0, length - 2);
	string output_filename = basename + ".txt";

	ofstream fout;
	fout.open(output_filename);

	if (command == "-trace")
	{
		Exec_Before();
		Exec_Trace();
		Exec_After();

		for (auto it : container_trace)
		{
			cout << it;
			fout << it;
		}
	}
	else if (command == "-read")
	{
		for (auto it : container_read)
		{
			cout << it << endl;
			fout << it << endl;
		}
	}
	else if (command == "-write")
	{
		for (auto it : container_write)
		{
			cout << it << endl;
			fout << it << endl;
		}
	}
	else if (command == "-before")
	{
		fout << "Memory Dump before execution\n\n";
		cout << "Memory Dump before execution\n\n";

		for (auto it : container_before)
		{
			fout << it << endl;
			cout << it << endl;
		}
	}
	else if (command == "-after")
	{
		fout << "Memory Dump after execution\n\n";
		cout << "Memory Dump after execution\n\n";

		for (auto it : container_after)
		{
			fout << it << endl;
			cout << it << endl;
		}
	}
	else if (command == "-isa")
	{
		fout << "Opcode Mnemonic Operand\n";
		cout << "Opcode Mnemonic Operand\n";

		for (auto it : container_instruc)
		{
			fout << it.F.S << it.F.F << it.S << endl;
			cout << it.F.S << it.F.F << it.S << endl;
		}

		fout << endl
			 << to_string(instruc_cnt) << " instructions executed";
		cout << endl
			 << to_string(instruc_cnt) << " instructions executed";
	}

	fout.close();
	return 0;
}
