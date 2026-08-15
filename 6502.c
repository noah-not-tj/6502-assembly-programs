#include <stdio.h>
#include <stdint.h>

#define N 0b10000000
#define V 0b01000000
#define B 0b00010000
#define D 0b00001000
#define I 0b00000100
#define Z 0b00000010
#define C 0b00000001

#define MAX_MEMORY (1024 * 64)

typedef uint8_t  byte;
typedef uint16_t word;

typedef struct {
  word PC;
  byte SP;
  byte P;
  byte A, X, Y;
} CPU;

CPU cpu;

byte mem[MAX_MEMORY];

void set_zero(byte x) {
  if (x == 0) {
    cpu.P |= Z;
  }
  else { cpu.P &= ~Z; }
  return;
}

void set_negative(byte x) {
  if (x >> 7 == 0x01) {
    cpu.P |= N;
  }
  else { cpu.P &= ~N; }

  return;
}


void init_memory(){
  for (int i = 0; i < MAX_MEMORY; i++) {
    mem[i] = 0x00;
  }
  mem[0xFFFC] = 0x00;  
  mem[0xFFFD] = 0x06;  
  
  mem[0x0600] = 0xC8;  //iny
  mem[0x0601] = 0x98;  //tya
  mem[0x0602] = 0x48;  //pha
  mem[0x0603] = 0xC8;  //iny
  mem[0x0604] = 0x98;  //tya
  mem[0x0605] = 0x68;  //pla
  
  return;
  
}
void reset_cpu() {
  cpu.PC = (mem[0xFFFD] << 8) + mem[0xFFFC];
  cpu.SP = 0xff;
  cpu.P |= I;
  cpu.A = cpu.X = cpu.Y = 0x00;
  return;
}
void cycle(int times) {
  return;
}

byte read_byte(word address) {
  return mem[address];
  cycle(2);   
}

void write_ZP(byte data, byte address) {
  
}


void SB1(byte op) {
  switch ((op & 0xf0) >> 4) {
      
    case 0xF:        //sed
      cpu.P |= D;
      break;
    case 0xE:        //inx
      cpu.X ++;
      set_zero(cpu.X);
      set_negative(cpu.X);
      break;
    case 0xD:        //cld
      cpu.P &= ~D;
      break;
    case 0xC:        // iny
      cpu.Y ++;
      set_zero(cpu.Y);
      set_negative(cpu.Y);
      break;
    case 0xB:       // clv
      cpu.P &= ~V;
      break;
    case 0xA:       //tay
      cpu.Y = cpu.A;
      break;
    case 0x9:       //tya
      cpu.A = cpu.Y;
      break;
    case 0x8:       //dey
      cpu.Y --;
      set_zero(cpu.Y);
      set_negative(cpu.Y);
      break;
    case 0x7:      //sei
      cpu.P |= I;
      break;
    case 0x6:      //pla
      cpu.A = read_byte(0x0100 + cpu.SP);
      cpu.SP ++;
      break;
    case 0x05:    //cli
      cpu.P &= ~I;
      break;
    case 0x04:    //pha
      cpu.SP --;
      mem[0x0100 + cpu.SP] = cpu.A;
      cycle(1);
      break;
    case 0x03:   //sec
      cpu.P |= C;
      break;
    case 0x02:   //plp
      cpu.P = read_byte(0x0100 + cpu.SP);
      cpu.SP ++;
      break;
    case 0x01:   //clc
      cpu.P &= ~C;
      break;
    case 0x00:   //php
      cpu.SP --;
      mem[0x0100 + cpu.SP] = cpu.P;
  }
  printf("opcode recieved: %x\n", (op & 0xf0) >> 4);
    
  cycle(2);
  return;
}

void decode(byte op) {
  if ((op & 0x0f) == 0x8) {
    SB1(op);
  }
  if ((op & 0x0f) == 0xA && (op & 0xf0) >> 4 > 0x08) {
    //SB2(op);
  }
  
  else return;
}

void execute() {
  byte instruction = read_byte(cpu.PC);
  decode(instruction);
  cpu.PC ++;
  printf("A: %i, X: %i, Y: %i, P: %08b\n", cpu.A, cpu.X, cpu.Y, cpu.P);
}


int main(void) {
  init_memory();
  reset_cpu();
  execute();
  execute();
  execute();
  execute();
  execute();
  execute();

 
  return 0;
}
