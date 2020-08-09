/*******************************************
* COMP1521 Assignment 2
* Written by Yip Jeremy Chung Lum, z5098112
********************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "smips.h"

#define N_BITS 32

int main(int argc, char *argv[]) {
    // Check if the number of arugments are correct
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        perror(argv[0]);
        return EXIT_FAILURE;
    }

    int bufferLength = 1000;
    char buffer[bufferLength];
    uint32_t registers[32];

    // Register[0] always equal to zero
    registers[0] = 0;
    // Assign all registers as -1, as not initialised
    for (int i = 1; i < 32; i++) {
        registers[i] = -1;
    }

    uint32_t instruction_array[bufferLength];
    // Assign all elements in the array as -1, which stands for not initialised
    for (int i = 0; i < bufferLength; i++) {
        instruction_array[i] = -1;
    }

    int i = 0;
    printf("Program\n");
    while(fgets(buffer, bufferLength, f)) {

        if (i < 10) {
            printf("  %d: ", i);
        } else {
            printf(" %d: ", i);
        }

        if ((strlen(buffer) - 1) < 8) {
            char *buffer2 = add_zero(buffer);
            // Store the binary form of the MIPS instruction
            instruction_array[i] = print_smips(buffer2);
        } else {
            // Store the binary form of the MIPS instruction
            instruction_array[i] = print_smips(buffer);
        }
        i++;
    }
    uint32_t *result = terminate_smips(instruction_array, registers);
    print_register(result);

    fclose(f);

    return EXIT_SUCCESS;
}

// Helper Function
// Given an array of mips instruction code in unsigned 32 bit integer
// and an array which holds all registers (32 registers in total).
// This function scan through the mips instruction codes and execute it. 
uint32_t *terminate_smips(uint32_t *instruction_code, uint32_t *registers) {
    printf("Output\n");
    for (int i = 0; instruction_code[i] != -1; i++) {
        char *instruction = convert_string(instruction_code[i]);
        char first_pattern[7];
        char last_pattern[12];

        int16_t s = convert_s_pattern(instruction_code[i]);
        int16_t t = convert_t_pattern(instruction_code[i]);
        int16_t d = convert_d_pattern(instruction_code[i]);
        int16_t I = convert_I_pattern(instruction_code[i]);

        for (int k = 0; k < 6; k++) {
            first_pattern[k] = instruction[k];
        }
        first_pattern[6] = '\0';

        int j = 21;
        for (int k = 0; k < 11; k++) {
            last_pattern[k] = instruction[j];
            j++;
        }
        last_pattern[11] = '\0';

        if (i == 2) {
            //printf("i: %d, s: %d, t: %d, I: %d\n", i, registers[s], t ,I);
            //printf("first_pattern: [%s], last_pattern: [%s]\n", first_pattern, last_pattern);
        }

        if (strcmp(first_pattern, "011100") == 0) { // If the instruction is mul
            registers[d] = registers[s] * registers[t];
        } else if (strcmp(first_pattern, "000100") == 0) { // Else if the instruction is beq
            if (registers[s] == registers[t]) {
                // Decrement an extra 1 because at the end of the for loop, i will be increment by 1 as well
                i += I - 1;
            }
        } else if (strcmp(first_pattern, "000101") == 0) { // Else if the instruction is bne
            if (registers[s] != registers[t]) {
                // Decrement an extra 1 because at the end of the for loop, i will be increment by 1 as well
                i += I - 1;
            }
        } else if (strcmp(first_pattern, "001000") == 0) { // Else if the instruction is addi
            registers[t] = registers[s] + I;
        } else if (strcmp(first_pattern, "001010") == 0) { // Else if the instruction is slti
            registers[t] = (registers[s] < I);
        } else if (strcmp(first_pattern, "001100") == 0) { // Else if the instruction is andi
            registers[t] = registers[s] & I;
        } else if (strcmp(first_pattern, "001101") == 0) { // Else if the instruction is ori
            registers[t] = registers[s] | I;
        } else if (strcmp(first_pattern, "001111") == 0) { // Else if the instruction is lui
            registers[t] = I << 16;  
        } else if (strcmp(first_pattern, "000000") == 0) { // Else if the instruction is add OR sub OR and OR or OR slt
            if (strcmp(last_pattern, "00000100000") == 0) { // if the instruction is add
                registers[d] = registers[s] + registers[t];
            } else if (strcmp(last_pattern, "00000100010") == 0) { // if the instruction is sub
                registers[d] = registers[s] - registers[t];
            } else if (strcmp(last_pattern, "00000100100") == 0) { // if the instruction is and
                registers[d] = registers[s] & registers[t];
            } else if (strcmp(last_pattern, "00000100101") == 0) { // if the instruction is or
                registers[d] = registers[s] | registers[t];
            } else if (strcmp(last_pattern, "00000101010") == 0) { // if the instruction is slt
                registers[d] = (registers[s] < registers[t] ? 1 : 0);
            } else if (strcmp(last_pattern, "00000001100") == 0) { // Else if the instruction is syscall
                // If $v0 == 1, print $a0 (print integer)
                if (registers[2] == 1) {
                    printf("%d", registers[4]);
                } else if (registers[2] == 11) { // Else if $v0 == 11, print $a0 (print character)
                    printf("%c", registers[4]);
                } else if (registers[2] == 10) { // Else if $v0 == 10, exit
                    return registers;
                } else { // Else, print error message and exit
                    printf("Unknown system call: %d\n", registers[2]);
                    return registers;
                }
            }
        }
        // Register 0 is always 0
        registers[0] = 0;
    }

    return registers;
}

// Helper Function
// Given a string array of mips instruction code in hex.
// This function scan through the mips instruction codes
// and prints out the program 
uint32_t print_smips(char *bits) {
    uint32_t instruction_code = convert_binary(bits);
    char *instruction = convert_string(instruction_code);
    char first_pattern[7];
    char last_pattern[12];

    int16_t s = convert_s_pattern(instruction_code);
    int16_t t = convert_t_pattern(instruction_code);
    int16_t d = convert_d_pattern(instruction_code);
    int16_t I = convert_I_pattern(instruction_code);

    for (int i = 0; i < 6; i++) {
        first_pattern[i] = instruction[i];
    }
    first_pattern[6] = '\0';

    int j = 21;
    for (int i = 0; i < 11; i++) {
        last_pattern[i] = instruction[j];
        j++;
    }
    last_pattern[11] = '\0';

    //printf("first_pattern: [%s], last_pattern: [%s]\n", first_pattern, last_pattern);
    if (strcmp(first_pattern, "011100") == 0) { // If the instruction is mul
        printf("mul  $%d, $%d, $%d\n", d, s, t);
    } else if (strcmp(first_pattern, "000100") == 0) { // Else if the instruction is beq
        printf("beq  $%d, $%d, %d\n", s, t, I);
    } else if (strcmp(first_pattern, "000101") == 0) { // Else if the instruction is bne
        printf("bne  $%d, $%d, %d\n", s, t, I);
    } else if (strcmp(first_pattern, "001000") == 0) { // Else if the instruction is addi
        printf("addi $%d, $%d, %d\n", t, s, I);
    } else if (strcmp(first_pattern, "001010") == 0) { // Else if the instruction is slti
        printf("slti  $%d, $%d, %d\n", t, s, I);
    } else if (strcmp(first_pattern, "001100") == 0) { // Else if the instruction is andi
        printf("andi  $%d, $%d, %d\n", t, s, I);
    } else if (strcmp(first_pattern, "001101") == 0) { // Else if the instruction is ori
        printf("ori  $%d, $%d, %d\n", t, s, I);
    } else if (strcmp(first_pattern, "001111") == 0) { // Else if the instruction is lui
        printf("lui  $%d, %d\n", t, I);
    } else if (strcmp(first_pattern, "000000") == 0) { // Else if the instruction is add OR sub OR and OR or OR slt
        if (strcmp(last_pattern, "00000100000") == 0) { // if the instruction is add
            printf("add  $%d, $%d, $%d\n", d, s, t);
        } else if (strcmp(last_pattern, "00000100010") == 0) { // if the instruction is sub
            printf("sub  $%d, $%d, $%d\n", d, s, t);
        } else if (strcmp(last_pattern, "00000100100") == 0) { // if the instruction is and
            printf("and  $%d, $%d, $%d\n", d, s, t);
        } else if (strcmp(last_pattern, "00000100101") == 0) { // if the instruction is or
            printf("or  $%d, $%d, $%d\n", d, s, t);
        } else if (strcmp(last_pattern, "00000101010") == 0) { // if the instruction is slt
            printf("slt  $%d, $%d, $%d\n", d, s, t);
        } else if (strcmp(last_pattern, "00000001100") == 0) { // Else if the instruction is syscall
            printf("syscall\n");
        }
    }

    return instruction_code;
}

// Helper Function
// Given an array which holds all registers (32 registers in total).
// This function prints out all registers which have values.
// (Note only registers with non-zero values are printed)
void print_register(uint32_t *registers) {
    printf("Registers After Execution\n");
    for (int i = 0; i < 32; i++) {   
        if (registers[i] != -1 && registers[i] != 0) {
            if (i < 10) {
                printf("$%d  = %d\n", i, registers[i]);
            } else {
                printf("$%d = %d\n", i, registers[i]);
            }
        }
    }
}

// Helper Function
// Given a string of hex digits
// return a binary form of 32 bit integer in char
uint32_t convert_binary(char * bits) {
    uint32_t result = 0;

    int i = 0;
    uint shift = 28;
    while(bits[i]) {
        uint32_t value = 0;
        switch (bits[i]) {
            case '0':
                value = UINT32_C(0x0) << shift;
                result = result | value;
                break;
            case '1':
                value = UINT32_C(0x1) << shift;
                result = result | value;
                break;
            case '2':
                value = UINT32_C(0x2) << shift;
                result = result | value;
                break;
            case '3':
                value = UINT32_C(0x3) << shift;
                result = result | value;
                break;
            case '4':
                value = UINT32_C(0x4) << shift;
                result = result | value;
                break;
            case '5':
                value = UINT32_C(0x5) << shift;
                result = result | value;
                break;
            case '6':
                value = UINT32_C(0x6) << shift;
                result = result | value;
                break;
            case '7':
                value = UINT32_C(0x7) << shift;
                result = result | value;
                break;
            case '8':
                value = UINT32_C(0x8) << shift;
                result = result | value;
                break;
            case '9':
                value = UINT32_C(0x9) << shift;
                result = result | value;
                break;
            case 'A':
                value = UINT32_C(0xA) << shift;
                result = result | value;
                break;
            case 'B':
                value = UINT32_C(0xB) << shift;
                result = result | value;
                break;
            case 'C':
                value = UINT32_C(0xC) << shift;
                result = result | value;
                break;
            case 'D':
                value = UINT32_C(0xD) << shift;
                result = result | value;
                break;
            case 'E':
                value = UINT32_C(0xE) << shift;
                result = result | value;
                break;
            case 'F':
                value = UINT32_C(0xF) << shift;
                result = result | value;
                break;
            case 'a':
                value = UINT32_C(0xA) << shift;
                result = result | value;
                break;
            case 'b':
                value = UINT32_C(0xB) << shift;
                result = result | value;
                break;
            case 'c':
                value = UINT32_C(0xC) << shift;
                result = result | value;
                break;
            case 'd':
                value = UINT32_C(0xD) << shift;
                result = result | value;
                break;
            case 'e':
                value = UINT32_C(0xE) << shift;
                result = result | value;
                break;
            case 'f':
                value = UINT32_C(0xF) << shift;
                result = result | value;
                break;
            default:
                //printf("\nInvalid hexa digit %c ", bits[i]);
                return result;
        }
        i++;
        shift -= 4;
    }
    return result;
}

// Helper Function
// Given an unsigned 32 bit integer
// return a null-terminated string of 32 binary digits ('1' and '0')
// storage for string is allocated using malloc
char *convert_string(uint32_t value) {
    // It's here allocating (N_BITS + 1) * sizeof(char)
    // because we have to allocate 1 more space for the NULL character
    char *result = malloc((N_BITS + 1) * sizeof(char));
    for (int i = 0; i < N_BITS; i++) {
        uint32_t mask = UINT32_C(1) << (N_BITS - 1 - i);

        // AND |  0   1
        // -------------
        //  0  |  0   0
        //  1  |  0   1
        // Which means if the i position of the value is 1,
        // then it will store the 1 into the result string
        if (value & mask) {
            result[i] = '1';
        } else {
            result[i] = '0';
        }
    }
    result[N_BITS] = '\0';
    return result;
}

// Helper Function
// Given an unsigned 32 bit integer
// return it's S pattern bits as a signed 16 bit integer
int16_t convert_s_pattern(uint32_t instruction) {
    int16_t result = 0;

    int shift_num = 10;
    // Store the bits of the s into result
    for (int i = 0; i < 5; i++) {
        int mask = UINT32_C(1) << (N_BITS - 1 - shift_num);
        int shift = UINT32_C(1) << i;

        if (instruction & mask) {
            result = result | shift;
        }
        shift_num--;
    }

    return result;
}

// Helper Function
// Given an unsigned 32 bit integer
// return it's t pattern bits as a signed 16 bit integer
int16_t convert_t_pattern(uint32_t instruction) {
    uint32_t result = 0;

    int shift_num = 15;
    // Store the bits of the t into result
    for (int i = 0; i < 5; i++) {
        int mask = UINT32_C(1) << (N_BITS - 1 - shift_num);
        int shift = UINT32_C(1) << i;

        if (instruction & mask) {
            result = result | shift;
        }
        shift_num--;
    }

    return result;
}

// Helper Function
// Given an unsigned 32 bit integer
// return it's d pattern bits as a signed 16 bit integer
int16_t convert_d_pattern(uint32_t instruction) {
    uint32_t result = 0;

    int shift_num = 20;
    // Store the bits of the d into result
    for (int i = 0; i < 5; i++) {
        int mask = UINT32_C(1) << (N_BITS - 1 - shift_num);
        int shift = UINT32_C(1) << i;

        if (instruction & mask) {
            result = result | shift;
        }
        shift_num--;
    }

    return result;
}

// Helper Function
// Given an unsigned 32 bit integer
// return it's I pattern bits as a signed 16 bit integer
int16_t convert_I_pattern(uint32_t instruction) {
    int32_t result = 0;

    int shift_num = 15;
    // Store the bits of the I into result
    for (int i = 0; i < 16; i++) {
        int mask = UINT32_C(1) << (15 - shift_num);
        int shift = UINT32_C(1) << i;

        if (instruction & mask) {
            result = result | shift;
        }
        shift_num--;
    }

    return result;
}

// Helper Function
// Add number of zeroes to the front of the buffer 
// so that the length of the buffer becomes 8
// Hence, print_smips() and terminate_smips() are able to 
// search for bit patterns with all zeros in the first 6 bits
// E.g. add, sub, and, or, slt
char *add_zero(char * buffer) {
    char *result = malloc(9 * sizeof(char));
    // There are only maximum 8 bytes per number
    // Hence 8 minus the length of the number, 
    // then we will get the number of zeroes we need to add
    int len = strlen(buffer) - 1;
    int num_zero_add = 8 - len;

    int index = 0;
    int i = 0;
    // Adding zeroes to the front
    while(i < num_zero_add) {
        result[i] = '0';
        i++;
    }
    index = i;


    int j = index;
    // Store the rest of the numbers
    for (int k = 0; k < len; k++ ) {
        result[j] = buffer[k];
        j++;
        index = j;
    }
    // Adding null character at the end of the string
    result[index] = '\0';

    return result;
}