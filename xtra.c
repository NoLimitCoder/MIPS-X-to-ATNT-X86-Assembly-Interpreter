#include "xtra.h"
#include "stdio.h"
#include "xis.h"

char *reg_map64[] = {
        "%rax",
        "%rbx",
        "%rcx",
        "%rdx",
        "%rsi",
        "%rdi",
        "%r8 ",
        "%r9 ",
        "%r10",
        "%r11",
        "%r12",
        "%r13",
        "%r14",
        "%r15",  // flags
        "%rbp",  // r14  frame pointer
        "%rsp",  // r15  stack pointer
        "%rip"
};

char *reg_map8[] = {
        "%al",
        "%bl",
        "%cl",
        "%dl",
        "%si",
        "%di",
        "%r8b",
        "%r9b",
        "%r10b",
        "%r11b",
        "%r12b",
        "%r13b",
        "%r14b",
        "%r15b",  // flags
        "%bp",  // r14  frame pointer
        "%sp",  // r15  stack pointer
        "%rip"
};

// Define static variables for flag and label
static int flag = 0, label = 0;

// Function to interpret an X instruction opcode into AT&T X86 assembly instructions
void translate_instruction(short word, unsigned char registers[], unsigned short value) {
    //Label variable for branching
    unsigned short L = registers[0];
    L = ((L << 8) | registers[1]);
    // Cases for each instruction opcode
    // Printing corresponding assembly instructions with register names
    switch(word) {
        case I_RET:
            printf("ret\n");
            break;
        case I_NEG:
            printf("neg %s\n",reg_map64[registers[0]]);
            break;
        case I_NOT:
            printf("not %s\n",reg_map64[registers[0]]);
            break;
        case I_PUSH:
            printf("push %s\n",reg_map64[registers[0]]);
            break;
        case I_POP:
            printf("pop %s\n",reg_map64[registers[0]]);
            break;
        case I_OUT:
            printf("mov %%rdi, -8(%%rbp)\n");
            printf("mov %s, %%rdi\n",reg_map64[registers[0]]);
            printf("call outchar\n");
            printf("mov -8(%%rbp), %%rdi\n");
            break;
        case I_INC:
            printf("inc %s\n",reg_map64[registers[0]]);
            break;
        case I_DEC:
            printf("dec %s\n",reg_map64[registers[0]]);
            break;
        case I_BR:
            printf("cmp $1, %%r15\n");
            printf("je .L%04x \n", label+L-2);
            break;
        case I_ADD:
            printf("add %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            break;
        case I_SUB:
            printf("sub %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            break;
        case I_MUL:
            printf("imul %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            break;
        case I_AND:
            printf("and %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
        break;
        case I_OR:
            printf("or %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
        break;
        case I_XOR:
            printf("xor %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
        break;
        case I_TEST:
            printf("test %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            printf("setnz %%r15b\n");
            break;
        case I_CMP:
            printf("cmp %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            printf("setg %%r15b\n");
            break;
        case I_EQU:
            printf("cmp %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            printf("sete %%r15b\n");
            break;
        case I_MOV:
            printf("mov %s, %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            break;
        case I_LOAD:
            printf("mov (%s), %s\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            break;
        case I_STOR:
            printf("mov %s, (%s)\n",reg_map64[registers[0]], reg_map64[registers[1]]);
            break;
        case I_LOADB:
            printf("movb (%s), %s\n",reg_map64[registers[0]], reg_map8[registers[1]]);
            break;
        case I_STORB:
            printf("mov %s, (%s)\n",reg_map8[registers[0]], reg_map64[registers[1]]);
            break;
        case I_JMP:
            printf("jmp .L%04x \n", value);
            break;
        case I_CALL:
            printf("call .L%04x \n", value);
            break;
        case I_LOADI:
            printf("mov $%hu, %s\n", value, reg_map64[registers[0]]);
            break;
        default: printf("Unknown instruction\n");
            break;
    }
}


// Main function for interpreting X instructions into AT&T X86 assembly
void xtra(FILE *fileHandle){
    //printing prolog
    printf(".globl test\ntest:\npush %%rbp\nmov %%rsp, %%rbp\n");
    unsigned short word;
        // Read one byte at a time until end of file
    while (fread(&word, sizeof(word), 1, fileHandle) == 1 && (word != 0x0000)){
        //printing instruction label
        printf(".L%04x:\n", label);
        if(flag)
            {printf("call debug\n");}
        //variables to extract from word
        unsigned char instruction, tmp_regist, registers[2];
        //extract instruction
        instruction = (word & 0x00FF);
        //extract registers
        tmp_regist = (word >> 8);
        //split registers into 0 and 1
        registers[0] = (tmp_regist >> 4);
        registers[1] = (tmp_regist & 0x0F);
        if(instruction == I_JMP || instruction == I_CALL || instruction == I_LOADI){
            label += 4;
        } else{
            label +=2;
        }
        if(instruction == I_STD){
            //debug flag set
            flag = 1;
            continue;
        }
        if(instruction == I_CLD){
            //debug flag clear
            flag = 0;
            continue;
        }
        if(instruction == I_LOADI || instruction == I_JMP || instruction == I_CALL){
            //extended instruction
            unsigned short value;
            fread(&value, sizeof(value), 1, fileHandle);
            //from big to small endian
            value = (value << 8 | value >> 8);
            //translate from X to AT&Tx86-64
            translate_instruction(instruction, registers, value);
        }
        else{
            //translate from X to AT&Tx86-64
            translate_instruction(instruction, registers, 0);
        }
    }
    //last label after end of input
    printf(".L%04x:\n", label);
    if(flag) {printf("call debug\n");}
    //printing assembly epilogue
    printf("pop %%rbp\nret\n");
}