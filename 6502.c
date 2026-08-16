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
  return;
}

void set_negative(byte x) {
  if (x >> 7 == 0x01) {
    cpu.P |= N;
  }
  return;
}


void init_memory(){
  for (int i = 0; i < MAX_MEMORY; i++) {
    mem[i] = 0x00;
  }
  mem[0xFFFC] = 0x00;  
  mem[0xFFFD] = 0x06;  
  
  mem[0x0600] = 0xC8; 
  mem[0x0601] = 0x98; 
  mem[0x0602] = 0x38; 
  mem[0x0603] = 0x2A; 
  mem[0x0604] = 0x48; 
  mem[0x0605] = 0xBA; 
  mem[0x0606] = 0xCA; 

  
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
  cycle(2);   
  return mem[address];
}

byte read_next() {
  return read_byte(cpu.PC++);
}

word read_word(word address) {
  word low = read_byte(address);
  word high = read_byte(address + 1);
  return (high << 8) | low;
}

word read_next_word() {
  word low = read_byte(cpu.PC++);
  word high = read_byte(cpu.PC++);
  return (high << 8) | low;
}

void set_carry() {
  cpu.P |= C;
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
      set_carry();
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
  //printf("opcode recieved: %x\n", (op & 0xf0) >> 4);
    
  cycle(2);
  return;
}

void SB2(byte op) {
  switch ((op & 0xf0) >> 4) {
    case 0x00:   // asl
      if ((cpu.A >> 7) == 0x01) {
        set_carry();
      }
      cpu.A = cpu.A << 1;
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    case 0x02:  // rol
      if ((cpu.A >> 7) == 0x01) {
        set_carry();
      }
      cpu.A = cpu.A << 1;
      if (cpu.SP & C == 0x01) {
        cpu.A ++;
      }
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    case 0x04:  //lsr
      if (cpu.A & 0x01 == 0x01) {
        set_carry();
      }
      cpu.A = cpu.A >> 1;
      break;

    case 0x06:  //ror
      byte wait;
      if (cpu.A & 0x01 == 0x01) {
        wait = 0x01;
      }
      cpu.A = cpu.A >> 1;
      cpu.A += (cpu.P & C) << 7;
      if (wait == 0x01) { set_carry(); }
      break;

    case 0x08: //txa
      cpu.A = cpu.X;
      break;
    case 0x09: //txs
      cpu.SP = cpu.X;
      break;
    case 0x0a: //tax
      cpu.X = cpu.A;
      break;
    case 0x0b: //tsx
      cpu.X = cpu.SP;
      break;
    case 0x0c: //dex
      cpu.X --;
      set_zero(cpu.X);
      set_negative(cpu.X);
      break;
    case 0x0e: //nop
      break;

  }
  cycle(2);
  //printf("opcode recieved: %x\n", (op & 0xf0) >> 4);

  return;
}

typedef struct {
  word EA;
  byte value;
  byte is_immediate;
} Opcode;

void group1(byte aaa, byte bbb) {
  Opcode o = {0};
  switch (bbb) {
    case 0b000:  //indexed indrect   (Indirect,X)    zp+x = low, zp+x+1 = high
      byte zp = read_next() + cpu.X;
      word low = read_byte(zp);
      word high = read_byte(zp + 1);
      o.EA = (high << 8) | low;      
      break;

    case 0b001:  //zero page         $zp
      o.EA = read_next();
      break;

    case 0b010:  //immediate         #immediate
      o.value = read_next();
      o.is_immediate = 0xff;
      break;

    case 0b011:  //Absolue           $absolute
      o.EA = read_next_word();
      break;
      
    case 0b100:  //indirect indexed  (Indirect),Y    (zp,low zp+1,high  )+y
      byte zp = read_next();
      byte low = read_byte(zp);
      byte high = read_byte(zp + 1);      
      o.EA = ((high << 8) | low) + (word)cpu.Y;
      break;

    case 0b101:  //zero page,X       $zp,X
      byte zp = read_next() + cpu.X;
      o.EA = (word)zp; 
      break;

    case 0b110:  //Absolute,Y        $abs,Y
      word address = read_next_word();
      o.EA = address + (word)cpu.Y;
      break;

    case 0b111:  //Absolute,X        $abs,X
      word address = read_next_word();
      o.EA = address + (word)cpu.X;
      break;
    
  }
}

void group2() {

}
void group2() {

}

void decode(byte op) {
  if ((op & 0x0f) == 0x8) {
    SB1(op);
  }
  else if ((op & 0x0f) == 0xA && ((op & 0xf0) >> 4) > 7) {
    SB2(op);
  }
  else {
    byte aaa = (op & 0xE0) >> 5;
    byte bbb = (op & 0x1C) >> 2;
    byte cc  = (op & 0x03);

    switch (cc) {
      case 0b01:
        group1(aaa, bbb);

      case 0b10:
        group2();

      case 0b11:
        if (bbb == 4) {
          //conditionals
        } else if (bbb == 0 && (aaa & 0x04) >> 3 == 0x00) {
          // I/S logic
        } else {
          group3();
        }
    }
  }
  
  else return;
}

void execute() {
  byte instruction = read_byte(cpu.PC++);
  decode(instruction);
  printf("A: %08b, X: %i, Y: %i,\n\n", cpu.A, cpu.X, cpu.Y);
  printf("NV BDIZC\n");
  printf("%08b\n\n", cpu.P);
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
  execute();


 
  return 0;
}
