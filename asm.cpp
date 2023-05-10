/*****************************************************************************
TITLE: assembler
AUTHOR: Aishez singh - 2101CS06
Declaration of Authorship
This cpp file, asm.cpp, is part of the assignment of CS209/210 at the
department of Computer Science and Engg, IIT Patna .
*****************************************************************************/
#include <bits/stdc++.h>
using namespace std;
#define F first
#define S second
#define pa pair<int, int>
#define sz(x) x.size()
#define pb push_back

// Storing the data
struct node
{
    string label; //  name of label to be replaced
    int cnt_prog; //  the offset of the current instruction in input assembly file
    int ln_num;   //  Line number
    int opc;      //  the binary opcode for the current instriuction
};

int program_counter = 0;
int line_number = 0;
int len;
string str, aux;
map<string, pa> instruct_table;
map<int, string> error_table;
map<string, int> label_table;
vector<string> original_file;
vector<string> errors;
vector<node> labelCallValue;
vector<node> labelCallOffset; // store the called label names
vector<string> lf_col1(1000);
vector<string> lf_col2(1000);

/*
The lf_col1 variable stores the left column of the .lst file, which contains the memory addresses of the translated machine code.
The lf_col2 variable stores the right column of the .lst file, which contains the corresponding machine code or the error message if any error occurred during assembly.
lst file is helpful in debugging


The lf_col1 and lf_col2 variables are vectors of strings with a maximum capacity of 1000 elements.
The lf_col1 vector stores the line numbers and the corresponding assembly code for each line,
while the lf_col2 vector stores the machine code generated for each line.
*/

int numberType;
int hex_max_value = 16777215; // the maxnumber used of finding hex of negative numbers
unsigned long long int hex_max_val8bit = 4294967295;
int errCheck[100005];
string current = "@aarya";
string previous = "@aarya";

void init() // initializing the instruction table with opcodes   instruct_table["memonic"] = { op_code , num_of_operand }
{
    // 0 means no value or offset
    // 1 means value or label
    // 2 means offset
    instruct_table["data"] = {-1, 1};
    instruct_table["SET"] = {-2, 1};
    instruct_table["ldc"] = {0, 1};   // load constant
    instruct_table["adc"] = {1, 1};   // add with carry
    instruct_table["ldl"] = {2, 2};   // load address of the label
    instruct_table["stl"] = {3, 2};   // Store to local (store a value to a local variable)
    instruct_table["ldnl"] = {4, 2};  // Load non local
    instruct_table["stnl"] = {5, 2};  // store non local
    instruct_table["add"] = {6, 0};   // add top 2 values of the stack
    instruct_table["sub"] = {7, 0};   // Subtract top 2 values of the stack
    instruct_table["shl"] = {8, 0};   // bitwise shift left
    instruct_table["shr"] = {9, 0};   // bitwise shift right
    instruct_table["adj"] = {10, 1};  // adjust SP ( SP += constant)
    instruct_table["a2sp"] = {11, 0}; // assign the SP with some constant
    instruct_table["sp2a"] = {12, 0}; // sp -> absolute address
    instruct_table["call"] = {13, 2}; // for calling a procedure/function
    instruct_table["return"] = {14, 0};
    instruct_table["brz"] = {15, 2};  // condition branch statement if the acumulator is 0
    instruct_table["brlz"] = {16, 2}; // branch is lesss tahn or equal to 0
    instruct_table["br"] = {17, 2};   // branch unconditionally
    instruct_table["HALT"] = {18, 0};
    error_table[0] = "incorrect file format";
    error_table[1] = "extra on end of line";
    error_table[2] = "bogus label name";
    error_table[3] = "duplicate label definition";
    error_table[4] = "bogus mnemonic";
    error_table[5] = "improper operand";
    return;
}

string removeUneccesarySpaces(string tmp)
{
    istringstream iss(tmp); // create input string stream from input string
    vector<string> tokens;  // create vector to store individual strings
    string token;
    while (iss >> token)
    { // read input string stream into vector of tokens
        tokens.push_back(token);
    }
    ostringstream oss; // create output string stream to build the final string
    for (size_t i = 0; i < tokens.size(); i++)
    {
        oss << tokens[i];
        if (i != tokens.size() - 1)
        {
            oss << " ";
        }
    }
    return oss.str(); // return final string from output string stream
}

string removeComment(string str)
{
    size_t pos = str.find(';'); // find position of first semicolon
    if (pos != string::npos)
    {                   // if semicolon found
        str.erase(pos); // erase from semicolon to end of string
    }
    return str;
}

void error(int tag, int lineNumber)
{
    errCheck[lineNumber] = 1;
    string tmp_err = "Error " + to_string(tag) + " on line number " + to_string(lineNumber + 1);
    errors.push_back(tmp_err);
}

bool isvalid_labelname(const std::string &str)
{
    if (str.empty())
    {
        return false;
    }
    bool valid_start = isalpha(str[0]) != 0 || str[0] == '_';
    for (size_t i = 1; i < str.size(); ++i)
    {
        bool valid_char = isalnum(str[i]) != 0 || str[i] == '_';
        if (!valid_char)
        {
            return false;
        }
    }
    return valid_start;
}

string opcode_to_hex_string(unsigned long long int num, int bits)
{
    string ret(bits, '0'); // initialize ret with bits length and fill with 0
    int idx = bits - 1;    // index to fill the content in ret from right to left
    while (num != 0 && idx >= 0)
    {
        int curr = num % 16;
        if (curr <= 9)
            ret[idx] = (char)(48 + curr); //  char( ascii(0) + curr)
        else
            ret[idx] = (char)(65 + curr - 10); // char( ascii('A') + curr-10 )
        num /= 16;
        idx--;
    }
    return ret;
}

bool isDec(string str)
{
    return str.find_first_not_of("0123456789") == string::npos;
}

bool isOct(string str)
{
    if (str.empty())
        return false; // Empty string is not valid octal
    for (char c : str)
    {
        if (c < '0' || c > '7')
        {
            return false; // Non-octal character
        }
    }
    return true;
}

bool isHex(string str)
{
    if (str.empty())
    {
        return false;
    }
    return all_of(str.begin(), str.end(), [](char c)
                  { return isxdigit(c); });
}

/*
    number type 0 : Decimal
    number type 1 : Octal
    number type 2 : Hex
*/

bool isValidValue(string str)
{
    // Check if the string is empty
    if (str.empty())
        return false;

    // Check for leading sign
    int start = 0;
    if (str[0] == '-' || str[0] == '+')
    {
        // If string contains only sign, it's invalid
        if (str.size() == 1)
            return false;
        start = 1;
    }

    // Check if value is hexadecimal
    if (start + 2 < str.size() && str[start] == '0' && (str[start + 1] == 'x' || str[start + 1] == 'X'))
    {
        for (int i = start + 2; i < str.size(); i++)
        {
            if (!isxdigit(str[i])) // predefined class in c++ to check if str[i] is in hexadecimal character
            {
                return false;
            }
        }
        numberType = 2;
        return true;
    }

    // Check if value is octal
    if (start < str.size() && str[start] == '0')
    {
        for (int i = start + 1; i < str.size(); i++)
        {
            if (str[i] < '0' || str[i] > '7')
            {
                return false;
            }
        }
        numberType = 1;
        return true;
    }

    // Check if value is decimal
    for (int i = start; i < str.size(); i++)
    {
        if (str[i] < '0' || str[i] > '9')
        {
            return false;
        }
    }
    numberType = 0;
    return true;
}

bool checkMnemonic(string str) // function to check if str is valid Mnemonic or not
{
    if (instruct_table.find(str) == instruct_table.end())
        return false;
    return true;
}

int DecOctHexToInt(string str, int type)
{
    int ret = 0, var1 = 1;
    for (char c : str)
    {
        int curr = 0;
        if (c >= '0' && c <= '9')
        {
            curr = c - '0';
        }
        else if (c >= 'A' && c <= 'F')
        {
            curr = c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'f')
        {
            curr = c - 'a' + 10;
        }
        ret = ret * type + curr;
    }
    return ret;
}

string value_to_opcode(string str, int type, int par = 0)
{
    bool ispositive = true;
    int start_idx = 0;
    if (str[0] == '-' || str[0] == '+')
    {
        start_idx++;
        if (str[start_idx] == '0' && str.size() == 2)
        {
            return "000000";
        }
        if (str[0] == '-')
        {
            ispositive = false;
        }
    }
    string val_str = str.substr(start_idx);
    unsigned long long int val = 0;
    if (type == 0)
    {
        val = std::stoi(val_str, nullptr, 10);
    }
    else if (type == 1)
    {
        val = std::stoi(val_str, nullptr, 8);
    }
    else if (type == 2)
    {
        val = std::stoi(val_str, nullptr, 16);
    }
    if (par == 1)
    {
        if (!ispositive)
        {
            val = hex_max_val8bit - val + 1;
        }
        return opcode_to_hex_string(val, 8);
    }
    else
    {
        if (!ispositive)
        {
            val = hex_max_value - val + 1;
        }
        return opcode_to_hex_string(val, 6);
    }
}

void Token_2(vector<string> &str_tok)
{
    string mnemonic = str_tok[0];

    if (!checkMnemonic(mnemonic))
    {
        lf_col2[line_number] = "        ";
        error(4, line_number);
        current = previous;
        return;
    }

    int opcode_mnemonic = instruct_table[mnemonic].F;

    if (opcode_mnemonic == -1) // Memonic = Define constant(DC) , operand = constant value
    {
        string operand = str_tok[1];

        if (!isValidValue(operand))
        {
            error(5, line_number);
            lf_col2[line_number] = "        ";
            current = previous;
            return;
        }

        string set_opc = value_to_opcode(operand, numberType, 1);
        lf_col2[line_number] = set_opc;
    }

    else if (opcode_mnemonic == -2) // mnemonic is "DS" (Define Storage) and the operand is the number of storage units to allocate
    {
        string operand = str_tok[1];

        if (!isValidValue(operand))
        {
            error(5, line_number);
            lf_col2[line_number] = "        ";
            current = previous;
            return;
        }

        string set_opc = value_to_opcode(operand, numberType, 1);
        lf_col2[line_number] = set_opc;

        if (current != previous)
        {
            label_table[current] = DecOctHexToInt(set_opc, 16);
        }
    }
    else
    {
        string suff_opcode = opcode_to_hex_string(opcode_mnemonic, 2);
        int num_of_operands = instruct_table[mnemonic].S;
        string operand = str_tok[1];

        if (num_of_operands == 0)
        {
            error(5, line_number);
            lf_col2[line_number] = "        ";
        }
        else if (num_of_operands == 1)
        {
            if (!isValidValue(operand))
            {
                labelCallValue.pb({operand, program_counter, line_number, opcode_mnemonic});
            }
            else
            {
                string pref_opcode = value_to_opcode(operand, numberType);
                lf_col2[line_number] = pref_opcode + suff_opcode;
            }
        }
        else if (num_of_operands == 2)
        {
            if (!isValidValue(operand))
            {
                labelCallOffset.pb({operand, program_counter, line_number, opcode_mnemonic});
            }
            else
            {
                string pref_opcode = value_to_opcode(operand, numberType);
                lf_col2[line_number] = pref_opcode + suff_opcode;
            }
        }
    }

    current = previous;
}

void check_instruc(string str)
{
    // Split the input string into tokens
    vector<string> str_tok;
    istringstream iss(str);
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(str_tok)); // copy( begning_itr , end_itr , beg_itr_destination)
    int num_tokens = str_tok.size();

    // Check number of tokens and process accordingly
    if (num_tokens == 0)
    {
        lf_col2[line_number] = "        "; // 8 bit space in column 2
    }
    else if (num_tokens == 1)
    {
        program_counter++;
        string mnemonic = str_tok[0];
        if (!checkMnemonic(mnemonic))
        {
            lf_col2[line_number] = "        ";
            error(4, line_number);
        }
        else
        {
            int opcode = instruct_table[mnemonic].F;
            int num_operands = instruct_table[mnemonic].S;
            string suff_opcode = opcode_to_hex_string(opcode, 2);
            if (num_operands == 0)
            {
                lf_col2[line_number] = "000000" + suff_opcode;
            }
            else // because token was 1
            {
                lf_col2[line_number] = "        ";
                error(5, line_number);
            }
        }
        current = previous;
    }
    else if (num_tokens == 2)
    {
        program_counter++;
        Token_2(str_tok);
    }
    else
    {
        program_counter++;
        error(1, line_number);
        lf_col2[line_number] = "        ";
        current = previous;
    }
}

/*
The variables current and previous are used to keep track of the current and previous lines of the assembly code being processed.
This is useful in error handling, as it allows the program to backtrack to the previous line in case an error is encountered on the current line.
For example, when an error is encountered in a line of code, the program sets the current variable to the previous line number,so that it can backtrack
 and re-process the previous line. This is done because the error in the current line may have been caused by a mistake in the previous line.
By re-processing the previous line, the program can check if there are any errors in it that could have caused the error in the current line.
The previous variable is used to store the line number of the previous line of code, which is updated whenever the program processes a new line.
*/

void Pass1(char **argv)
{
    string tmp = argv[1];
    int lnght = tmp.size();
    string fname = tmp.substr(0, lnght - 4);
    string inp_file = fname + ".asm";
    ifstream input(inp_file);
    current = previous;

    while (getline(input, str))
    {
        original_file.push_back(str);
        len = str.size();

        // REmove comments
        for (int i = 0; i < len; i++)
        {
            if (str[i] == ';')
            {
                str = str.substr(0, i);
                len = str.size();
                break;
            }
        }

        //  REmove white  space from line
        int i = 0, j = 0;
        while (i < len)
        {
            while (i < len && str[i] == ' ')
                i++;
            while (i < len && str[i] != ' ')
            {
                str[j] = str[i];
                i++, j++;
            }
            if (i < len && str[i] == ' ')
                str[j++] = ' ';
        }
        if (j > 0 && str[j - 1] == ' ')
            j--;

        str = str.substr(0, j);
        len = str.size();

        if (len == 0) // If line is empty skip it
        {
            lf_col1[line_number] = lf_col2[line_number] = " ";
            line_number++;
            continue;
        }

        // Check if the line contains a label

        int colon_idx = -1;
        for (int i = 0; i < len; i++)
        {
            if (str[i] == ':')
            {
                colon_idx = i;
                break;
            }
        }
        if (colon_idx == -1)
        {
            string pc = opcode_to_hex_string(program_counter, 8);
            lf_col1[line_number] = pc;
            check_instruc(str);
        }

        else
        {
            string pc = opcode_to_hex_string(program_counter, 8);
            lf_col1[line_number] = pc;
            string label = str.substr(0, colon_idx);
            bool valid_label = true;
            if (label.size() == 0)
                valid_label = false;
            if (!isalpha(label[0])) // isalpha predefined function to check if alphabet
                valid_label = false;
            for (int i = 1; i < label.size(); i++)
            {
                if (!isalnum(label[i]) && label[i] != '_')
                {
                    valid_label = false;
                    break;
                }
            }
            if (valid_label == false)
            {
                error(2, line_number);
            }
            else
            {
                if (label_table.find(label) != label_table.end())
                {
                    error(3, line_number);
                }
                else
                {
                    current = label;
                    label_table[label] = program_counter;
                }
            }
            string instruc = str.substr(colon_idx + 1, len - colon_idx - 1);
            check_instruc(instruc);
        }
        line_number++;
    }
}

void writeObjFile(string fname) // Writing into the object file
{
    string obj_file = fname + ".o";
    vector<int> machine_code_int; // object file contents
    for (int i = 0; i < line_number; i++)
    {
        if (lf_col2[i][0] == ' ')
            continue;
        int code = DecOctHexToInt(lf_col2[i], 16);
        machine_code_int.push_back(code);
    }
    ofstream obj_file_stream(obj_file, ios::binary);
    int *code_array = machine_code_int.data();
    int num_codes = machine_code_int.size();
    obj_file_stream.write(reinterpret_cast<const char *>(code_array), num_codes * sizeof(int));
}

// Performs Pass 2 of the assembler
void Pass2(char **arguments) // symbolic address will be replaced with absolute addresss , symbolic opcodes will be replaced by binary opcodes
{
    // Iterate through labelCallOffset
    // Calculating address if the label
    for (auto element : labelCallOffset)
    {
        // Check if the label exists in the label table
        if (label_table.find(element.label) == label_table.end())
        {
            error(2, element.ln_num); // Report error and continue
            continue;
        }
        int address = label_table[element.label] - element.cnt_prog; // program_counter - offset of current instruction
        if (address < 0)                                             // if the current instruction is lower than the actuall label
        {
            address = hex_max_value + address + 1;
        }
        lf_col2[element.ln_num] = opcode_to_hex_string(address, 6) + opcode_to_hex_string(element.opc, 2);
    }

    // Iterate through labelCallValue
    for (auto element : labelCallValue)
    {
        // Check if the label exists in the label table
        if (label_table.find(element.label) == label_table.end())
        {
            error(2, element.ln_num); // Report error and continue
            continue;
        }
        int address = label_table[element.label]; // address stores the program counter
        if (address < 0)
        {
            address = hex_max_value + address + 1;
        }
        lf_col2[element.ln_num] = opcode_to_hex_string(address, 6) + opcode_to_hex_string(element.opc, 2);
    }

    // Prepare to write error log
    string filename = string(arguments[1]);
    int length = filename.size();
    string baseName = filename.substr(0, length - 4);
    string errorFile = baseName + ".log";
    ofstream errorLog;
    errorLog.open(errorFile);

    // Check if there were any errors
    bool errorFlag = false;
    if (errors.size() == 0)
    {
        errorLog << "Compiled without any errors" << endl;
    }
    else
    {
        cout << "Error messages:\n";
    }

    // Write error messages to log file and console
    for (auto message : errors)
    {
        errorLog << "ERROR: " << message << endl;
        cout << "ERROR: " << message << endl;
        errorFlag = true;
    }
    errorLog.close();

    // Write assembled code to list file
    string listFile = baseName + ".lst";
    ofstream listLog;
    listLog.open(listFile);

    for (int i = 0; i < line_number; i++)
    {
        if (errCheck[i])
        {
            continue;
        }
        string output = lf_col1[i] + " " + lf_col2[i] + " " + original_file[i];
        listLog << output << endl;
    }

    listLog.close();

    // If there were no errors, write the object file
    if (!errorFlag)
    {
        writeObjFile(baseName);
    }
}

int main(int argc, char **argv)
{
    // Initialize the opcode map.
    init();

    // Get the input filename and extension.
    std::string input_filename = argv[1];
    std::string extension = input_filename.substr(input_filename.find_last_of('.') + 1);

    // Check if the file extension is supported.
    if (extension != "asm")
    {
        std::cout << "Error: unsupported file format." << std::endl;
        return 0;
    }

    else
    {
        // Pass 1 of the assembler.
        Pass1(argv);

        // Pass 2 of the assembler.
        Pass2(argv);
    }

    return 0;
}
