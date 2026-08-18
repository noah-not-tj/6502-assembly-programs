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

  //sus

  FILE *f = fopen("output.bin", "r");
  if (f == NULL) { }
  size_t bytes_read = fread(mem, sizeof(byte), MAX_MEMORY, f);  
  if (bytes_read < MAX_MEMORY) {
    mem[0x0600] = 0x00;
    return;
  }

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

void write_byte(word address, byte value) {
  cycle(2);
  mem[address] = value;
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
  byte is_accumulator;
} Opcode;

void group1(byte aaa, byte bbb) {
  Opcode o = {0};
  // group one addressing modes
  switch (bbb) {
    case 0b000: {  //indexed indrect   (Indirect,X)    zp+x = low, zp+x+1 = high
      byte zp = read_next() + cpu.X;
      word low = read_byte(zp);
      word high = read_byte((byte)(zp + 1));
      o.EA = (high << 8) | low;      
      break;
    }
    case 0b001: {  //zero page         $zp
      o.EA = read_next();
      break;
    }
    case 0b010: {  //immediate         #immediate
      o.value = read_next();
      o.is_immediate = 0xff;
      break;
    }
    case 0b011: {  //Absolue           $absolute
      o.EA = read_next_word();
      break;
    }  
    case 0b100: {  //indirect indexed  (Indirect),Y    (zp,low zp+1,high  )+y
      byte zp = read_next();
      byte low = read_byte(zp);
      byte high = read_byte(zp + 1);      
      o.EA = ((high << 8) | low) + (word)cpu.Y;
      break;
    }
    
    case 0b101: {  //zero page,X       $zp,X
      byte zp = read_next() + cpu.X;
      o.EA = (word)zp; 
      break;
    }
    case 0b110: {  //Absolute,Y        $abs,Y
      word address = read_next_word();
      o.EA = address + (word)cpu.Y;
      break;
    }
    case 0b111: {  //Absolute,X        $abs,X
      word address = read_next_word();
      o.EA = address + (word)cpu.X;
      break;
    }
  }
   // group 1 instrucion
   byte val = o.is_immediate ? o.value : read_byte(o.EA);
   printf("value: %x\n",val);
   switch (aaa) {
    case 0b000: { //ORA
      cpu.A |= val;
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    }
    case 0b001: {//AND
      cpu.A &= val;
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    }
    case 0b010: {//EOR
      cpu.A ^= val;
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    }
    case 0b011: {//ADC
      word carry_in = (cpu.P & C) ? 1 : 0;
      word temp = (word)cpu.A + (word)val + carry_in; 
      if (temp > 0xFF) {
        set_carry();
      } else { cpu.P &= ~C; }

      byte result = (byte)(temp & 0xFF);

      if (((cpu.A ^ result) & (val ^ result) & 0x80) != 0x00) {
        cpu.P |= V;
      } else { cpu.P &= ~V; }

      cpu.A = result;
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    }
    case 0b100: {//STA
      write_byte(o.EA, cpu.A);
      break;
    }
    case 0b101: {//LDA
      cpu.A = val;
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    }
    case 0b110: {//CMP
      if (cpu.A == val) {
        cpu.P |= Z;
      } else { cpu.P &= ~Z; } 

      if (cpu.A >= val) {
        cpu.P |= C;
      } else { cpu.P &= ~C; } 
      set_negative(cpu.A);
      break;
    }
    case 0b111: {//SBC
      // A + ~M + C
      word inverted = (~val) & 0xFF;

      word carry_in = (cpu.P & C) ? 1 : 0;
      word temp = (word)cpu.A + (word)inverted + carry_in;

      if (temp > 0xFF) {
        set_carry();
      } else { cpu.P &= ~C; }

      byte result = (byte)(temp & 0xFF);

      if (((cpu.A ^ result) & (inverted ^ result) & 0x80) != 0x00) {
        cpu.P |= V;
      } else { cpu.P &= ~V; }

      cpu.A = result;
      set_zero(cpu.A);
      set_negative(cpu.A);
      break;
    }
  }
}

void group2(byte aaa, byte bbb) {
  Opcode o = {0};
  switch (bbb) {
    case 0b000: {//Immediate [exeption for stx, dec, inc]
      // check for ldx
      if (aaa == 0b101) {
        o.value = read_next();
        o.is_immediate = 0xff;
        break;
      }
      // txa
      if (aaa == 0b100) {
        break; //(implied)
      }
      break;
    }
    case 0b001: {//zp
      o.EA = (word)read_next();
      break;
    }
    case 0b010: {//A
      o.is_accumulator = 0xff;
      break;
    }
    case 0b011: {//absolute 
      o.EA = read_next_word();
      break;
    }
    case 0b100: {//unused
      break;
    }
    case 0b101: {// zero page X, Y - STX LDX use Y
      // check if stx or ldx 
      byte zp = read_next();
      if (aaa == 0b100 || aaa == 0b101) {
        o.EA = (word)((zp + cpu.Y) & 0xFF);
        break;
      }
      o.EA = (word)((zp + cpu.X) & 0xFF);
      break;
    }
    case 0b110: {//unused
      break;
    }
    case 0b111: {//absolute x
      word abs = read_next_word();
      // check for ldx
      if (aaa == 0b101) {
        abs += (word)cpu.Y;
        o.EA = abs;
        break;
      }
      abs += (word)cpu.X;
      o.EA = abs;
      break;
    }
  }
  byte val = o.is_immediate ? o.value : read_byte(o.EA); 
  printf("value: %x\n",val);

  switch (aaa) {
    case 0b000: { //asl
      byte temp = (o.is_accumulator == 0) ? val : cpu.A;

      if ((temp >> 7) == 0x01) {
        set_carry();
      } else {
        cpu.P &= ~C;
      }
      temp = temp << 1;
      set_zero(temp);
      set_negative(temp);
      
      if (o.is_accumulator != 0) {
        cpu.A = temp;
      } else {
        mem[o.EA] = temp;
      }
      break;
    }
    case 0b001: {//rol
      byte temp = (o.is_accumulator == 0) ? val : cpu.A;

      if ((temp >> 7) == 0x01) {
        set_carry();
      } else {
        cpu.P &= ~C;
      }
      temp = cpu.A << 1;
      if ((cpu.P & C) == C) {
        temp ++;
      }
      set_zero(temp);
      set_negative(temp);

      if (o.is_accumulator != 0) {
        cpu.A = temp;
      } else {
        mem[o.EA] = temp;
      }
      break;
    }
    case 0b010: {//lsr

      byte temp = (o.is_accumulator == 0) ? val : cpu.A;

      if ((temp & 0x01) == 0x01) {
        set_carry();
      } else {
        cpu.P &= ~C;
      }

      temp = temp >> 1;

      if (o.is_accumulator != 0) {
        cpu.A = temp;
      } else {
        mem[o.EA] = temp;
      }
      break;
    }
    case 0b011: {//ror

      byte temp = (o.is_accumulator == 0) ? val : cpu.A;

      byte wait = 0;
      if ((temp & 0x01) == 0x01) {
        wait = 0x01;
      }
      temp = temp >> 1;
      temp += ((cpu.P & C) << 7);
      if (wait == 0x01) { 
        set_carry(); 
      } else {
        cpu.P &= ~C;
      }

      if (o.is_accumulator != 0) {
        cpu.A = temp;
      } else {
        mem[o.EA] = temp;
      }
      break;
    }
    case 0b100: {//stx / txa   ?not sure
      //if (bbb == 000) = txa
      if (bbb == 0b000) {
        cpu.A = cpu.X;
        set_zero(cpu.A);
        set_negative(cpu.A);
        break;
      }
      //else stx
      mem[o.EA] = cpu.X;
      break;

    }
    case 0b101: {//ldx
      cpu.X = (o.is_immediate) ? o.value : mem[o.EA];
      set_zero(cpu.X);
      set_negative(cpu.X);
      break;

    }
    case 0b110: {//dec
      val--;
      mem[o.EA] = (val);
      set_zero(val);
      set_negative(val);
      break;

    }
    case 0b111: {//inc
      val++;
      mem[o.EA] = (val);
      set_zero(val);
      set_negative(val);
      break;
    }
  }

}
void group3(byte aaa, byte bbb) {
  Opcode o = {0};
  switch (bbb) {
    case 0b000: {  //indexed indrect   (Indirect,X)    zp+x = low, zp+x+1 = high
      byte zp = read_next() + cpu.X;
      word low = read_byte(zp);
      word high = read_byte((byte)(zp + 1));
      o.EA = (high << 8) | low;      
      break;
    }
    case 0b001: {  //zero page         $zp
      o.EA = read_next();
      break;
    }
    case 0b010: {  //immediate         #immediate
      if (aaa == 0b100) {
        // supposed to XAA or ANE but i'm not gonna impliment
        o.is_immediate = 0xff;
        o.value = 0x00;
        break;
      }
      o.value = read_next();
      o.is_immediate = 0xff;
      break;
    }
    case 0b011: {  //Absolue           $absolute
      o.EA = read_next_word();
      break;
    }  
    case 0b100: {  //indirect indexed  (Indirect),Y    (zp,low zp+1,high  )+y
      byte zp = read_next();
      byte low = read_byte(zp);
      byte high = read_byte(zp + 1);      
      o.EA = ((high << 8) | low) + (word)cpu.Y;
      break;
    }
    
    case 0b101: {  //zero page,X       $zp,X
      if (aaa == 0b101 || aaa == 0b100) {
        byte zp = read_next() + cpu.Y;
        o.EA = (word)zp; 
        break;
      }
      byte zp = read_next() + cpu.X;
      o.EA = (word)zp; 
      break;
    }
    case 0b110: {  //Absolute,Y        $abs,Y
      word address = read_next_word();
      o.EA = address + (word)cpu.Y;
      break;
    }
    case 0b111: {  //Absolute,X        $abs,X
      if (aaa == 0b101) {
        word address = read_next_word();
        o.EA = address + (word)cpu.Y;
        break; 
      }
      word address = read_next_word();
      o.EA = address + (word)cpu.X;
      break;
    }
  }
  byte val = (o.is_immediate) ? o.value : read_byte(o.EA);
  printf("value: %x\n", val);

  switch(aaa) {
    case 0b000: { //SLO / ASO   = ASL + ORA
      break;

    }
    case 0b001: { //SRE / LSE   = LSR + EOR
      break;

    }
    case 0b010: { //RLA         = ROL + AND
      break;

    }
    case 0b011: { //RRA         = ROR + ADC
      break;

    }
    case 0b100: { //SAX         = STA + STX
      break;

    }
    case 0b101: { //LAX         = LDA + LDX    #immediate = OAL / ATX,   Abs Y = LAS / LAR
      break;

    }
    case 0b110: { //DCP         = DEC + CMP    #immediate = AXS / SBX
      break;

    }
    case 0b111: { //ISC         = INC + SBC    #immediate = SBC / NOP
      break;

    }
  }

}

void ISLogic(byte op) {
  //just direct opcode parsing
  switch (op) {
    case 0x4c: { //JMP ABSOLUTE
      word add = read_next_word();
      cpu.PC = add;
      break;
    }
    case 0x6c: { //JMP INDIRECT
      word vec_add = read_next_word();
      byte high_add = (byte)((vec_add & 0xFF00) >> 4);
      byte low_add  = (vec_add & 0x00FF == 0xFF) ? (byte)(0x00) : (byte)((vec_add & 0xFF00) >> 4);
      word temp = read_byte(high_add) << 8;
      temp += (read_byte(low_add));
      cpu.PC = temp;
      break;
    }
    case 0x00: {    //BRK
      cpu.PC += 2;
      mem[++cpu.SP] = (byte)(cpu.PC & 0x00FF);
      mem[++cpu.SP] = (byte)((cpu.PC & 0xFF00) >> 4);
      mem[++cpu.SP] = (cpu.P | B); 
      cpu.P |= I;
      cpu.PC = read_word(0xFFFE);
      break;
    }
    case 0x40: { // RTI

    }
    case 0x60: { // RTS
      cpu.P = read_word(--cpu.SP);
      cpu.PC = read_word(--cpu.SP);
      cpu.PC = (read_word(--cpu.SP) << 8) | cpu.PC;
      break;
    }
  }

} 

void signed_jump(byte n) {
  if (n & 0x80) { // if it is negative
    cpu.PC -= ((byte)(~n + 1));
    return;
  } else {
    cpu.PC += n;
    return;
  }
}

void conditionals(byte aaa) {
  switch (aaa) {
    case 0b000: {  // negative clear
      byte n = read_next();
      if ((cpu.P & N) == 0x00) {
        printf("offset: %x\n",n);
        signed_jump(n);
      }
      break;
    }
    case 0b001: {  // negative set
      byte n = read_next();
      if ((cpu.P & N) != 0) {
        signed_jump(n);
      }
      break;
    }
    case 0b010: {  // overflow clear
      byte n = read_next();
      if ((cpu.P & V) == 0) {
        signed_jump(n);
      }
      break;
    }
    case 0b011: {  // overflow set
      byte n = read_next();
      if ((cpu.P & V) != 0) {
        signed_jump(n);
      }
      break;
    }
    case 0b100: {  // carry clear
      byte n = read_next();
      if ((cpu.P & C) == 0) {
        signed_jump(n);
      }
      break;
    }
    case 0b101: {  // carry set
      byte n = read_next();
      if ((cpu.P & C) != 0) {
        signed_jump(n);
      }
      break;
    }
    case 0b110: {  // zero clear
      byte n = read_next();
      if ((cpu.P & V) == 0) {
        signed_jump(n);
      }
      break;
    }
    case 0b111: {  // zero set
      byte n = read_next();
      if ((cpu.P & C) != 0) {
        signed_jump(n);
      }
      break;
    }
  }
}

void decode(byte op) {
  printf("opcode recieved: %x\n",op);
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
        break;

      case 0b10:
        group2(aaa, bbb);
        break;

      default:
        if (op == 0x4c || op == 0x6c || op == 0x00) {
           ISLogic(op);
        }
        else if (bbb == 4) {  //not sure yet
          //conditionals
          conditionals(aaa);
        } else {
          group3(aaa, bbb);
        }
        break;
    }
  }
  
  return;
}

void execute() {
  byte instruction = read_byte(cpu.PC++);
  decode(instruction);
  printf("----------------------------\n");
  printf("A: %08b, X: %x, Y: %x\n\n", cpu.A, cpu.X, cpu.Y);
  printf("NV BDIZC\n");
  printf("%08b\n\n", cpu.P);
  printf("0x0300: %s\n\n", &mem[0x0300]);
  printf("PC: %x\n", cpu.PC);
  printf("SP: %x SP_VAL: %x\n", cpu.SP, mem[cpu.SP]);
  printf("----------------------------\n");
}


int main(void) {
  init_memory();
  reset_cpu();
  while (mem[cpu.PC] != 0x00) {
    execute();
  };


 
  return 0;
}
