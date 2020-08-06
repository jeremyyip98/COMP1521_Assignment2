#ifndef SMIPS_H
#define SMIPS_H

uint32_t *terminate_smips(uint32_t *instruction_code, uint32_t *registers);
uint32_t print_smips(char *bits);
void print_register(uint32_t *registers);
uint32_t convert_binary(char * bits);
char *convert_string(uint32_t value);
int16_t convert_s_pattern(uint32_t instruction);
int16_t convert_t_pattern(uint32_t instruction);
int16_t convert_d_pattern(uint32_t instruction);
int16_t convert_I_pattern(uint32_t instruction);
char *add_zero(char * buffer);

#endif