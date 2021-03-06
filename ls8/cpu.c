#include "cpu.h"
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>




/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */


void cpu_load(struct cpu *cpu, char *filename)
{
  FILE *fp = fopen(filename, "r");
  char line[9999];

  if(fp == NULL) {
    printf("Error opening file\n");
    exit(1); 
  }

  int address = 0;
  char *endptr[10];

  while(fgets(line, sizeof(line), fp) != NULL) {
    strtoul(line, endptr, 2);
    if(endptr[0] != line) {
      cpu->ram[address] = strtoul(line, NULL, 2); 
      address++; 
    }
  }
  
  fclose(fp);
}


unsigned char cpu_ram_read(struct cpu *cpu, unsigned char PC) 
{
  return cpu->ram[PC]; 
}

void cpu_ram_write(struct cpu *cpu, unsigned char PC, unsigned char input) 
{
  cpu->ram[PC] = input; 
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op) {
    case ALU_MUL:
      cpu->R[0] = regA * regB;
      break;
    
    case ALU_ADD:
      cpu->R[0] = regA + regB;
      break;
    
    case ALU_CMP:
      if(regA == regB) {
        cpu->FL = 00000001;
        break;
      } else if(regA < regB) {
        cpu->FL = 00000100;
        break;
      } else if(regA > regB) {
        cpu->FL = 00000010;
        break;
      }
      break;
    
    case ALU_AND: 
      cpu->R[0] = regA & regB; 
      break;
    
    case ALU_OR: 
      cpu->R[0] = regA | regB; 
      break;

    case ALU_XOR: 
      cpu->R[0] = regA ^ regB; 
      break;
    
    case ALU_NOT: 
      cpu->R[0] = ~regA; 
      break;

    case ALU_SHL: 
      cpu->R[0] = regA<<regB; 
      break;

    case ALU_SHR: 
      cpu->R[0] = regA>>regB; 
      break;

    case ALU_MOD: 
      cpu->R[0] = regA % regB; 
      break;

  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; 

  while (running) {
    int c = cpu->ram[cpu->PC]; 
    unsigned char movePC = (cpu->ram[cpu->PC] >> 6);

    switch (c) {
      case LDI: 
        cpu->R[cpu->ram[cpu->PC + movePC - 1]] = cpu->ram[cpu->PC + movePC];
        cpu->PC += movePC;
        break;

      case PRN:
        printf("Decimal: %d\n", cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case MUL:
        alu(cpu, 0, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case ADD: 
        alu(cpu, 1, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case CMP: 
        alu(cpu, 2, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case AND: 
        alu(cpu, 3, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case OR:
        alu(cpu, 4, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case XOR:
        alu(cpu, 5, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case NOT:
        alu(cpu, 6, cpu->R[cpu->ram[cpu->PC + movePC]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case SHL:
        alu(cpu, 7, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case SHR:
        alu(cpu, 8, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case MOD:
        if(cpu->R[cpu->ram[cpu->PC + movePC]] == 0) {
          printf("Register B cannot equal zero when performing a MOD\n");
          running = 0; 
          break; 
        }
        alu(cpu, 9, cpu->R[cpu->ram[cpu->PC + movePC - 1]], cpu->R[cpu->ram[cpu->PC + movePC]]);
        cpu->PC += movePC;
        break;

      case PUSH:
        cpu->R[7]--; 
        cpu->ram[cpu->R[7]] = cpu->R[cpu->ram[cpu->PC + movePC]];
        cpu->PC += movePC;
        break;

      case POP:
        cpu->R[cpu->ram[cpu->PC + movePC]] = cpu->ram[cpu->R[7]]; 
        cpu->R[7]++;
        cpu->PC += movePC;
        break;

      case CALL:
        cpu->R[7]--; 
        cpu->ram[cpu->R[7]] = cpu->PC + movePC; 
        cpu->PC = cpu->R[cpu->ram[cpu->PC + movePC]]; 
        break;

      case RET:
        cpu->PC = cpu->ram[cpu->R[7]];
        cpu->R[7]++;
        break;  

      case JMP:
        cpu->PC = cpu->R[cpu->ram[cpu->PC + movePC]] - 1;
        break;
      
      case JEQ:
        if(cpu->FL == 00000001) {
          cpu->PC = cpu->R[cpu->ram[cpu->PC + movePC]] - 1;
        } else {
            cpu->PC += movePC;
        }
        break;
      
      case JNE:
        if(cpu->FL == 00000010 || cpu->FL == 00000100 || cpu->FL == 0 ) {
          cpu->PC = cpu->R[cpu->ram[cpu->PC + movePC]] - 1;
        } else {
            cpu->PC += movePC;
        } 
        break;

      case HLT:
        running = 0;
        break; 
    }
    cpu->PC++; 
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->PC = 0; 
  cpu->FL = 0; 
  
  // TODO: Zero registers and RAM
  memset(cpu->R, 0, 7 * sizeof(cpu->R[0]));
  cpu->R[7] = 0xF4; 
  memset(cpu->ram, 0, 256 * sizeof(cpu->ram[0]));
}


