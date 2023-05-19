#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OP(bits)     (((bits) >> 26) & (0b111111))
#define RS(bits)     (((bits) >> 21) & (0b11111))
#define RT(bits)     (((bits) >> 16) & (0b11111))
#define RD(bits)     (((bits) >> 11) & (0b11111))
#define SA(bits)     (((bits) >> 6) & (0b11111))
#define FN(bits)     ((bits) & (0b111111))
#define IMM(bits)    ((bits) & (0b1111111111111111))
#define ADD(bits)    ((bits) & (0b11111111111111111111111111))

char inst[0x10001];
char data[0x10001];
int Reg[32];
int PC;
int Max_PC;
int count;
int bits;

void read_file(const char* path, char dest[]);
int get_bits(char mem[], int add);
void R_format(int bits);
void J_format(int bits);
void I_format(int bits);


int main(int argc, char* argv[]) {
    if (argc <= 2 || argc >= 5) {
        printf("파일 읽기 에러!\n");
        exit(1);
    }
    memset(inst, -1, sizeof(inst)); // inst - 0xff로 초기화
    memset(data, -1, sizeof(data)); // data - 0xff로 초기화
    count = atoi(argv[1]);

    if (argc == 3) {                //inst 파일만
        read_file(argv[2], inst);       
    }
    else if (argc == 4) {           //inst, data 파일 둘 다
        read_file(argv[2], inst);      
        read_file(argv[3], data);
    }

    while (count-- && PC <= Max_PC) {
        bits = get_bits(inst, PC);
        switch (OP(bits)) {
        case 0b000000:                // R_format
            R_format(bits);
            break;
        case 0b000010: case 0b000011: // J_format
            J_format(bits);
            break;
        default:                      // I_format
            I_format(bits);
        }
    }
    
    for (int i = 0; i < 32; i++) {            //레지스터 값 출력
        printf("$%d: 0x%08x\n", i, Reg[i]);
    }
    printf("PC: 0x%08x\n", PC);               //PC 값 출력

    return 0;
}


void read_file(const char* path, char dest[]) {
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("파일 열기 에러\n");
        exit(1);
    }
    for (int i = 0; !feof(fp); i++) {
        fread(dest + i, 1, 1, fp);
    }
    if (Max_PC == 0) {
        Max_PC = ftell(fp);
    }
    fclose(fp);
}

int get_bits(char mem[], int add) {
    int res = 0;
    for (int i = 0; i < 4; i++) {
        res |= (mem[add + i] << (8 * (3 - i))) & (0xff << (8 * (3 - i)));
    }
    return res;
}

void R_format(int bits) {
    int rs = RS(bits);
    int rt = RT(bits);
    int rd = RD(bits);
    int shamt = SA(bits);
    int funct = FN(bits);

    switch (funct) {
    case 0b100000: // add
        Reg[rd] = Reg[rs] + Reg[rt];
        break;
    case 0b100001: // addu
        Reg[rd] = Reg[rs] + Reg[rt];
        break;
    case 0b100010: // sub
        Reg[rd] = Reg[rs] - Reg[rt];
        break;
    case 0b100011: // subu
        Reg[rd] = Reg[rs] - Reg[rt];
        break;
    case 0b100100: // and
        Reg[rd] = Reg[rs] & Reg[rt];
        break;   
    case 0b100101: // or
        Reg[rd] = Reg[rs] | Reg[rt];
        break;    
    case 0b101010: // slt
        Reg[rd] = Reg[rs] < Reg[rt] ? 1 : 0;
        break;
    case 0b101011: // sltu
        Reg[rd] = (unsigned)Reg[rs] < (unsigned)Reg[rt] ? 1 : 0;
        break;
    case 0b001000: // jr
        PC = (unsigned)Reg[rs];
        break;
    case 0b000000: // sll
        Reg[rd] = Reg[rt] << shamt;
        break;
    case 0b000010: // srl
        Reg[rd] = Reg[rt] >> shamt;
        for (int i = 0; i < shamt; i++) {       // logical shift이기 때문에 밀린만큼 0으로 마스킹
            Reg[rd] = Reg[rd] & ~(1 << (31 - i));
        }
        break;       
    default:
        printf("unknown instruction\n");
    }    
    PC += 4;        
}

void J_format(int bits) {
    int op = OP(bits);
    int add = ADD(bits);
    switch (op) {
    case 0b000010: // j
        PC = add * 4;
        break;
    case 0b000011: // jal
        Reg[31] = PC + 4;
        PC = add * 4;
        break;
    default:
        printf("unknown instruction\n");
        PC += 4;
    }
}

void I_format(int bits) {
    int op = OP(bits);
    int rs = RS(bits);
    int rt = RT(bits);
    short immd = IMM(bits);
    switch (op) {
    case 0b001000: // addi
        Reg[rt] = Reg[rs] + immd;
        break;
    case 0b001001: // addiu
        Reg[rt] = Reg[rs] + immd;
        break;
    case 0b001100: // andi
        Reg[rt] = Reg[rs] & (immd & 0xffff);
        break;
    case 0b001101: // ori
        Reg[rt] = Reg[rs] | (immd & 0xffff);
        break;
    case 0b001111: // lui
        Reg[rt] = immd << 16;
        break;
    case 0b001010: // slti
        Reg[rt] = Reg[rs] < immd ? 1 : 0;  
        break;
    case 0b001011: // sltiu
        Reg[rt] = (unsigned)Reg[rs] < (unsigned)immd ? 1 : 0;  
        break;            
    case 0b000100: // beq
        PC = PC + (Reg[rs] == Reg[rt] ? immd * 4 : 0);
        break;
    case 0b000101: // bne
        PC = PC + (Reg[rs] != Reg[rt] ? immd * 4 : 0);
        break;
    case 0b100011: // lw
        Reg[rt] = get_bits(data, Reg[rs] + immd - 0x10000000);
        break;
    case 0b101011: // sw
        for (int i = 0; i < 4; i++) {
            data[Reg[rs] + immd - 0x10000000 + i] = (Reg[rt] >> (8 * (3 - i))) & 0xff;
        }
        break;
    default:
        printf("unknown instruction\n");
        break;
    }
    PC += 4;
}