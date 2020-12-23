#include "GritVM.hpp"

// load a gvm file into the program and set data members
STATUS GritVM::load(std::string filename, const std::vector<long> &initialMemory) {
    if(status != WAITING)
        return status;
    std::ifstream iFF(filename);
    std::string gvmLine;
    if(iFF.fail()) {
        throw("CannotOpenFile"); //throw if there is an invalid file given
    }
    else {
        while (!iFF.eof()) { //parse every line until the end of the file
            getline(iFF, gvmLine);
            if(!gvmLine.empty()) { //ignore white space
                if(gvmLine.at(0) != '#') //ignore comments
                {
                    Instruction is = GVMHelper::parseInstruction(gvmLine);
                    if(is.operation == UNKNOWN_INSTRUCTION) {
                        status = ERRORED;
                        return status;
                    }
                    instruction_mem.push_back(is);
                }
            }
        }
    }

    if(instruction_mem.empty()) {
        status = WAITING;
    }
    else {
        status = READY; //set the status to ready after properly parsing the gvm
    }
    data_mem = initialMemory;
    currentInstruction = instruction_mem.begin(); //set currentInstruction to the first instruction
    iFF.close();
    return status;
}

STATUS GritVM::run() {
    if(status == READY) { //if its ready, execute the instructions
        status = RUNNING;
        while(status == RUNNING) {  //run loop that will break when the status switches        
            advance(evaluate(*currentInstruction)); //calling evaluate and jumping the appropriate amount of instructions
        }
    }
    return status;
}

// reset the machine completely
STATUS GritVM::reset() {
    accumulator = 0;
    data_mem.clear();
    instruction_mem.clear();
    status = WAITING;
    return status;
}

//Returns how many instructions to jump by and executes instruction
long GritVM::evaluate(Instruction inst) {
    switch (inst.operation)
    {
        case CLEAR: {
            accumulator = 0;
            return 1;
        }
        break;
        case AT: {
            accumulator = data_mem[inst.argument];
            return 1;
        }
        break;
        case SET: {
            data_mem[inst.argument] = accumulator;
            return 1;
        }
        break;
        case INSERT: {
            auto iter = data_mem.begin();
            data_mem.insert(iter + inst.argument, accumulator);
            return 1;
        }
        break;
        case ERASE: {
            auto iter = data_mem.begin();
            auto itErase = iter + inst.argument;
            data_mem.erase(itErase);
            return 1;
        }
        break;
        case ADDCONST: {
            accumulator += inst.argument;
            return 1;
        }
        break;
        case SUBCONST: {
            accumulator -= inst.argument;
            return 1;
        }
        break;
        case MULCONST: {
            accumulator *= inst.argument;
            return 1;
        }
        break;
        case DIVCONST: {
            accumulator /= inst.argument;
            return 1;
        } 
        break;
        case ADDMEM:{
            accumulator += data_mem[inst.argument];
            return 1;
        }
        break;
        case SUBMEM: {
            accumulator -= data_mem[inst.argument];
            return 1;
        }
        break;
        case MULMEM: {
            accumulator *= data_mem[inst.argument];
            return 1;
        }
        break;
        case DIVMEM: {
            accumulator /= data_mem[inst.argument];
            currentInstruction++;
        }
        break;
        case JUMPREL: {
            if(inst.argument == 0) {
                status = ERRORED; //argument cannot be 0
                return 0;
            }
            return inst.argument;
        }
        break;
        case JUMPZERO: {
            if(inst.argument == 0) {
                status = ERRORED; //argument cannot be 0
                return 0;
            }
            if (accumulator == 0) {
                return inst.argument;
            }
            else {
                return 1;
            }
        }
        break;
        case JUMPNZERO: {
            if(inst.argument == 0) {
                status = ERRORED; //argument cannot be 0
                return 0;
            }
            if (accumulator != 0) {
                return inst.argument;
            }
            else {
                return 1;
            }
        }
        break;
        case NOOP: {
            return 1;
        }
        break;
        case HALT:  {
            status = HALTED; //HALT the machine
            return 1;
        }
        break;
        case OUTPUT: {
            std::cout << accumulator; //output the accumulator
            return 1;
        }
        break;
        case CHECKMEM: {
            if(static_cast<long>(data_mem.size()) < inst.argument) { //cast size to compare with argument
                status = ERRORED;
            }
            return 1;   
        }
        break;
        case UNKNOWN_INSTRUCTION: { //unknown status when an unknown instruction occurs (should not happen)
            status = UNKNOWN;
            return 1;
        }
        break;
        default: {
            return 1;
        }
        break;
    }
    return 0;
}

//advances the current instruction the appropriate amount
void GritVM::advance(long jump) {
    if(jump > 0) {
        while(jump != 0) {
            currentInstruction++; //move the position jump amount of times
            jump--;
        }
    }
    else if(jump < 0) {
        while(jump != 0) {
            currentInstruction--; //move the position jump amount of times
            jump++;
        }
    }
    else if (jump == 0) {
        //do nothing
    }
}

//method to print how the GVM Machine is working
void GritVM::printVM(bool printData, bool printInstruction) {
    std::cout << "****** Output Dump ******" << std::endl;
    std::cout << "Status: " <<
    GVMHelper::statusToString(status) << std::endl;
    std::cout << "Accumulator: " << accumulator << std::endl;
    if (printData) {
        std::cout << "*** Data Memory ***" << std::endl;
        int index = 0;
        std::vector<long>::iterator it = data_mem.begin();
        while(it != data_mem.end()) {
            long item = (*it);
            std::cout << "Location " << index++ << ": " << item << std::endl;
            it++;
        }
    }
    if (printInstruction) {
        std::cout << "*** Instruction Memory ***" << std::endl;
        int index = 0;
        std::list<Instruction>::iterator it = instruction_mem.begin();
        while(it != instruction_mem.end()) {
            Instruction item = (*it);
            std::cout << "Instruction " <<
            index++ << ": " <<
            GVMHelper::instructionToString(item.operation) <<
            " " << item.argument << std::endl;
            it++;
        }
    }
}