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



void initMemory(){
  for (int i = 0; i < MAX_MEMORY; i++) {
    mem[i] = 0x00;
  }
  mem[0xFFFC] = 0x00;  
  mem[0xFFFD] = 0x06;  
  
  return;
  
}
void initCPU() {
  cpu.PC = (mem[0xFFFD] << 8) + mem[0xFFFC];
  cpu.SP = 0x00;
  cpu.P |= I;
  cpu.A = cpu.X = cpu.Y = 0x00;
  return;
}



int main(void) {
  initMemory();
  initCPU();
 
  return 0;
}
