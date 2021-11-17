#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

void print_board();
void read_file();
void decode_instruction(string current_instruction);
void populate_data(int num_int_unit, int num_mult, int num_divider, int num_adder);
vector<string> INSTRUCTION_STRINGS;

int MEMORY[19] = {45, 12, 0, 0, 10, 135, 254, 127, 18, 4, 55, 8, 2, 98, 13, 5, 233, 158, 167};
float FP_REG_ARRAY[32];
int INT_REG_ARRAY[32];

// //vars for unit cycles
int ADDER_CYCLE = 2;
int MULT_CYCLE = 10;
int DIV_CYCLE = 40;
int INT_U_CYCLE = 1;

// This struct contains information pertaining to each instruction line that gets read
struct instr {
    string full_instruction;
    string type;
    string destination;
    string s1;
    string s2;
    int issue;
    int read_oper;
    int exec_complete;
    int write_result;
};

// This vector holds the full list of instructions
vector<instr> INSTRUCTIONS;

int main()
{
    // Initialize all registers to 0
    for(int i = 0; i < 32; i++){
        FP_REG_ARRAY[i] = 0;
        INT_REG_ARRAY[i] = 0;
    }

    // //initialize FU number variables
    int num_adder = 0;
    int num_mult = 0;
    int num_divider = 0;
    int num_int_unit = 0;

    // //Get functional unit data from user

    cout << "How many integer units (load, store, integer add/sub) do you want to use? : ";
    cin >> num_int_unit;
    
    cout << "How many adder units do you want to use? : ";
    cin >> num_adder;

    cout << "How many multiplier units do you want to use? : ";
    cin >> num_mult;

    cout << "How many divider units do you want to use? : ";
    cin >> num_divider;

    //Reads INSTRUCTION_STRINGS from .txt file
    read_file();

    //Decodes all INSTRUCTION_STRINGS and makes necessary changes to registers
    for(int i = 0; i < INSTRUCTION_STRINGS.size(); i++) {
        decode_instruction(INSTRUCTION_STRINGS.at(i));
    }

    populate_data(num_int_unit, num_mult, num_divider, num_adder);
    print_board();

    // Prints out register contents
    cout << endl << endl << "====== Register Contents" << endl << "Register #" << setw(25) << "FP Registers:" << setw(20) << "Int Registers:" << endl;
    for(int i = 0; i < 32; i++) {
        cout << setw(10) << i << ": " << setw(23) << FP_REG_ARRAY[i] << setw(20) << INT_REG_ARRAY[i] << endl;

    }

    return 0;
}

void decode_instruction(string current_instruction) {
    // This function parses through each instruction line and updates registers/memory address accordingly. 

    instr new_instruction;
    new_instruction.full_instruction = current_instruction;

    string instruction = current_instruction.substr(0, current_instruction.find(" "));

    int dest_register = 0;
    int s_register = 0;
    int t_register = 0;
    current_instruction = current_instruction.substr(current_instruction.find(" ") + 1, current_instruction.size());

    // These if/else statements determine what type of instruction is currently being parsed through, dictating the pattern of the remaining parsing.
    if(instruction == "L.D") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        int offset = stoi(current_instruction.substr(0, current_instruction.find("(")));
        current_instruction = current_instruction.substr(current_instruction.find("(") + 1, current_instruction.size());
        int address = stoi(current_instruction.substr(0, current_instruction.find(")")));
        FP_REG_ARRAY[dest_register] = MEMORY[address + offset];
        s_register = offset;
        t_register = address;
        new_instruction.type = "load/store";
    } else if(instruction == "S.D") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        int offset = stoi(current_instruction.substr(0, current_instruction.find("(")));
        current_instruction = current_instruction.substr(current_instruction.find("(") + 1, current_instruction.size());
        int address = stoi(current_instruction.substr(0, current_instruction.find(")")));
        MEMORY[address + offset] = FP_REG_ARRAY[dest_register];
        s_register = offset;
        t_register = address;
        new_instruction.type = "load/store";
    } else if(instruction == "LI") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        int imm = stoi(current_instruction);
        INT_REG_ARRAY[dest_register] = imm;
        t_register = imm;
        new_instruction.type = "load/store";
    } else if(instruction == "LW") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        int offset = stoi(current_instruction.substr(0, current_instruction.find("(")));
        current_instruction = current_instruction.substr(current_instruction.find("(") + 1, current_instruction.size());
        int address = stoi(current_instruction.substr(0, current_instruction.find(")")));
        INT_REG_ARRAY[dest_register] = MEMORY[address + offset];
        s_register = offset;
        t_register = address;
        new_instruction.type = "load/store";
    } else if(instruction == "SW") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        int offset = stoi(current_instruction.substr(0, current_instruction.find("(")));
        current_instruction = current_instruction.substr(current_instruction.find("(") + 1, current_instruction.size());
        int address = stoi(current_instruction.substr(0, current_instruction.find(")")));
        MEMORY[address + offset] = INT_REG_ARRAY[dest_register];
        s_register = offset;
        t_register = address;
        new_instruction.type = "load/store";
    } else if(instruction == "ADD") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        s_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 3, current_instruction.size());
        t_register = stoi(current_instruction);
        INT_REG_ARRAY[dest_register] = INT_REG_ARRAY[s_register] + INT_REG_ARRAY[t_register];
        new_instruction.type = "adder";
    } else if(instruction == "ADD.D") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        s_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 3, current_instruction.size());
        t_register = stoi(current_instruction);
        FP_REG_ARRAY[dest_register] = FP_REG_ARRAY[s_register] + FP_REG_ARRAY[t_register];
        new_instruction.type = "adder";
    } else if(instruction == "ADDI") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(" ") + 1, current_instruction.size());
        s_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(" ") + 1, current_instruction.size());
        int imm = stoi(current_instruction);
        t_register = imm;
        INT_REG_ARRAY[dest_register] = INT_REG_ARRAY[s_register] + imm;
        new_instruction.type = "adder";
    } else if(instruction == "SUB") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        s_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 3, current_instruction.size());
        t_register = stoi(current_instruction);
        INT_REG_ARRAY[dest_register] = INT_REG_ARRAY[s_register] - INT_REG_ARRAY[t_register];
        new_instruction.type = "adder";
    } else if(instruction == "SUB.D") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 2, current_instruction.size());
        s_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(",") + 3, current_instruction.size());
        t_register = stoi(current_instruction);
        FP_REG_ARRAY[dest_register] = FP_REG_ARRAY[s_register] - FP_REG_ARRAY[t_register];
        new_instruction.type = "adder";
    } else if(instruction == "MUL.D") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(" ") + 1, current_instruction.size());
        s_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(" ") + 2, current_instruction.size());
        t_register = stoi(current_instruction);
        FP_REG_ARRAY[dest_register] = FP_REG_ARRAY[s_register] * FP_REG_ARRAY[t_register];
        new_instruction.type = "mult";
    } else if(instruction == "DIV.D") {
        dest_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(" ") + 1, current_instruction.size());
        s_register = stoi(current_instruction.substr(1, current_instruction.find(",") - 1));
        current_instruction = current_instruction.substr(current_instruction.find(" ") + 2, current_instruction.size());
        t_register = stoi(current_instruction);
        FP_REG_ARRAY[dest_register] = FP_REG_ARRAY[s_register] / FP_REG_ARRAY[t_register];
        new_instruction.type = "divider";
    }
    new_instruction.destination = to_string(dest_register);
    new_instruction.s1 = to_string(s_register);
    new_instruction.s2 = to_string(t_register);
    new_instruction.issue = 0;
    new_instruction.read_oper = 0;
    new_instruction.exec_complete = 0;
    new_instruction.write_result = 0;
    INSTRUCTIONS.push_back(new_instruction);
    return;
}

void read_file() {
    //reads input_file and returns vector of instruction lines

    fstream input_file;
	input_file.open("scoreboardInput.txt");
    string instruction_line;
    int index = 0;

    while (!input_file.eof()) {
        getline(input_file, instruction_line);
        if(instruction_line != "") {
            INSTRUCTION_STRINGS.push_back(instruction_line);
        }
        index += 1;
    }
    input_file.close();
    return;
}

void populate_data(int num_int_unit, int num_mult, int num_divider, int num_adder) {
    // This function does the math to determine what cycle each instruction's steps are completed on and updates each instrument structs members

    int num_avail_int = num_int_unit;
    int num_avail_mult = num_mult;
    int num_avail_div = num_divider;
    int num_avail_add = num_adder;
    int next_avail_int = 0;
    int next_avail_add = 0;
    int next_avail_mult = 0;
    int next_avail_div = 0;
    int cycle = 1;
    bool wait_flag = false;


    // This outer loop populates instruction structs with correct Issue, Read Operands, Exec complete, and Write Result cycles
    for(int i = 0; i < INSTRUCTIONS.size(); i++) {
        num_avail_int = num_int_unit;
        num_avail_mult = num_mult;
        num_avail_div = num_divider;
        num_avail_add = num_adder;
        wait_flag = false;
        if(cycle > 1) {
            INSTRUCTIONS.at(i).issue = INSTRUCTIONS.at(i - 1).read_oper;
        } else {
            INSTRUCTIONS.at(i).issue = 1;
        }
        // This inner loop checks to see if the destination of the current instruction is being used by any of the previous instructions, 
        // and if so, if it's safe to write to the destination yet. If not, it updates the 'next_available_unit' value
        for(int j = 0; j < i; j++) {
            if(INSTRUCTIONS.at(j).destination == INSTRUCTIONS.at(i).s1 || INSTRUCTIONS.at(j).destination == INSTRUCTIONS.at(i).s2) {
                if(INSTRUCTIONS.at(i).issue < INSTRUCTIONS.at(j).write_result) {
                    INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(j).write_result + 1;
                    wait_flag = true;
                }
            } else if (wait_flag == false){
                INSTRUCTIONS.at(i).read_oper = cycle + 1;
            }
            if(INSTRUCTIONS.at(j).type == "load/store") {
                if(cycle <= INSTRUCTIONS.at(j).write_result) {
                    num_avail_int -= 1;
                    next_avail_int = INSTRUCTIONS.at(j).write_result + 1;
                }
            } else if(INSTRUCTIONS.at(j).type == "adder") {
                if(cycle <= INSTRUCTIONS.at(j).write_result) {
                    num_avail_add -= 1;
                    next_avail_add = INSTRUCTIONS.at(j).write_result + 1;
                }
            } else if(INSTRUCTIONS.at(j).type == "mult") {
                if(cycle <= INSTRUCTIONS.at(j).write_result) {
                    num_avail_mult -= 1;
                    next_avail_mult = INSTRUCTIONS.at(j).write_result + 1;
                }
            } else if(INSTRUCTIONS.at(j).type == "divider") {
                if(cycle <= INSTRUCTIONS.at(j).write_result) {
                    num_avail_div -= 1;
                    next_avail_div = INSTRUCTIONS.at(j).write_result + 1;
                }
            }
        }

        // This level of if/else statements determines what type of instruction is currently being examined, 
        // and uses the corresponding cycle time for the execution stage
        if(INSTRUCTIONS.at(i).type == "load/store") {
            // This level of if/else statements determines if there is an available functional unit to run the current instruction line. 
            // If not, the read_oper step receives the 'next_available_unit' cycle value
            if(num_avail_int > 0) {
                INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + INT_U_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            } else {
                if(next_avail_int > INSTRUCTIONS.at(i - 1).read_oper) {
                    INSTRUCTIONS.at(i).issue = next_avail_int;
                }
                INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + INT_U_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            }
        } else if(INSTRUCTIONS.at(i).type == "adder") {
            if(num_avail_add > 0) {
                if(wait_flag == false) {
                    INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                }
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + ADDER_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            }  else {
                if(next_avail_add > INSTRUCTIONS.at(i - 1).read_oper) {
                    INSTRUCTIONS.at(i).issue = next_avail_add;
                }
                INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + ADDER_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            }
        } else if(INSTRUCTIONS.at(i).type == "mult") {
            if(num_avail_mult > 0) {
                if(wait_flag == false) {
                    INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                }
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + MULT_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            }  else {
                if(next_avail_mult > INSTRUCTIONS.at(i - 1).read_oper) {
                    INSTRUCTIONS.at(i).issue = next_avail_mult;
                }
                INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + MULT_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            }
        } else if(INSTRUCTIONS.at(i).type == "divider") {
            if(num_avail_div > 0) {
                if(wait_flag == false) {
                    INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                }
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + DIV_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            }  else {
                if(next_avail_div > INSTRUCTIONS.at(i - 1).read_oper) {
                    INSTRUCTIONS.at(i).issue = next_avail_div;
                }
                INSTRUCTIONS.at(i).read_oper = INSTRUCTIONS.at(i).issue + 1;
                INSTRUCTIONS.at(i).exec_complete = INSTRUCTIONS.at(i).read_oper + DIV_CYCLE;
                INSTRUCTIONS.at(i).write_result = INSTRUCTIONS.at(i).exec_complete + 1;
            }
        }
        cycle += 1;
    }
    
    return;
}


void print_board()
{
    // Prints header lines
    cout << endl << "====== " << "Instruction Status" << setw(37) << "Read" << setw(18) << "Execution" << setw(10) << "Write" << endl;
    cout << setw(18) << "Instruction" << setw(15) << "Destination" << setw(5) << "S1" << setw(5) << "S2" << setw(10) << "Issue" << setw(13)
    << "operands" << setw(13) << "complete" << setw(12) << "result" << endl;

    // Prints instructions w/ step-cycle values
    for(int i = 0; i < INSTRUCTIONS.size(); i++) {
        cout << setw(18) << INSTRUCTIONS.at(i).full_instruction << setw(15) << INSTRUCTIONS.at(i).destination << setw(5) << INSTRUCTIONS.at(i).s1 << setw(5) << INSTRUCTIONS.at(i).s2
        << setw(10) << INSTRUCTIONS.at(i).issue << setw(13) << INSTRUCTIONS.at(i).read_oper << setw(13) << INSTRUCTIONS.at(i).exec_complete << setw(12) << INSTRUCTIONS.at(i).write_result << endl;
    }
    return;
}