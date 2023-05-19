#include <stdio.h>
#include <stdlib.h>

#define OP(bits)     (((bits) >> 26) & (0b111111))
#define RS(bits)     (((bits) >> 21) & (0b11111))
#define RT(bits)     (((bits) >> 16) & (0b11111))
#define RD(bits)     (((bits) >> 11) & (0b11111))
#define SA(bits)     (((bits) >> 6) & (0b11111))
#define FN(bits)     ((bits) & (0b111111))
#define IMM(bits)    ((bits) & (0b1111111111111111))
#define ADD(bits)   ((bits) & (0b11111111111111111111111111))

char inst[300];
int f_len;
char* cur;
int bits;

void read_file(const char* path);
int get_bits(char inst[]);
void R_format(int i, int bits);
void J_format(int i, int bits);
void I_format(int i, int bits);

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("파일 읽기 에러!");
        exit(1);
    }
    read_file(argv[1]);

    for (int i = 0; i < f_len / 4; i++) {
        cur = inst + i * 4;
        bits = get_bits(cur);
        switch (OP(bits)) {
        case 0b000000:
            // R_format
            R_format(i, bits);
            break;
        case 0b000010: case 0b000011:
            // J_format
            J_format(i, bits);
            break;
        default:
            // I_format
            I_format(i, bits);
        }
    }

    return 0;
}

void read_file(const char* path) {
    FILE* fp = fopen(path, "rb");
    for (int i = 0; !feof(fp); i++) {
        fread(inst + i, 1, 1, fp);
        f_len++;
    }
    fclose(fp);
}

int get_bits(char inst[]) {
    int res = 0; 
    for (int i = 0; i < 4; i++) { 
        res |= (inst[i] << (8 * (3 - i))) & (0xff << (8 * (3 - i)));
    }
    return res;
}

void R_format(int i, int bits) {
    int rs = RS(bits);
    int rt = RT(bits);
    int rd = RD(bits);
    int shamt = SA(bits);
    int funct = FN(bits);

    switch (funct) {
    case 0b100000: // add
        printf("inst %d: %08x add $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b100001: // addu
        printf("inst %d: %08x addu $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b100100: // and
        printf("inst %d: %08x and $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b011010: // div
        printf("inst %d: %08x div $%d, $%d\n", i, bits, rs, rt);
        break;
    case 0b011011: // divu
        printf("inst %d: %08x divu $%d, $%d\n", i, bits, rs, rt);
        break;
    case 0b001001: // jalr
        printf("inst %d: %08x jalr $%d, $%d\n", i, bits, rd, rs);
        break;        
    case 0b001000: // jr
        printf("inst %d: %08x jr $%d\n", i, bits, rs);
        break;
    case 0b010000: // mfhi
        printf("inst %d: %08x mfhi $%d\n", i, bits, rd);
        break;
    case 0b010010: // mflo
        printf("inst %d: %08x mflo $%d\n", i, bits, rd);
        break;    
    case 0b010001: // mthi
        printf("inst %d: %08x mthi $%d\n", i, bits, rs);
        break;
    case 0b010011: // mtlo
        printf("inst %d: %08x mtlo $%d\n", i, bits, rs);
        break;    
    case 0b011000: // mult
        printf("inst %d: %08x mult $%d, $%d\n", i, bits, rs, rt);
        break;
    case 0b011001: // multu
        printf("inst %d: %08x multu $%d, $%d\n", i, bits, rs, rt);
        break;
    case 0b100111: // nor
        printf("inst %d: %08x nor $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;            
    case 0b100101: // or
        printf("inst %d: %08x or $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b000000: // sll
        printf("inst %d: %08x sll $%d, $%d, %d\n", i, bits, rd, rt, shamt);
        break;
    case 0b000100: // sllv
        printf("inst %d: %08x sllv $%d, $%d, $%d\n", i, bits, rd, rt, rs);
        break;        
    case 0b101010: // slt
        printf("inst %d: %08x slt $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b101011: // sltu
        printf("inst %d: %08x sltu $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b000011: // sra
        printf("inst %d: %08x sra $%d, $%d, %d\n", i, bits, rd, rt, shamt);
        break;
    case 0b000111: // srav
        printf("inst %d: %08x srav $%d, $%d, $%d\n", i, bits, rd, rt, rs);
        break;    
    case 0b000010: // srl
        printf("inst %d: %08x srl $%d, $%d, %d\n", i, bits, rd, rt, shamt);
        break;
    case 0b000110: // srlv
        printf("inst %d: %08x srlv $%d, $%d, $%d\n", i, bits, rd, rt, rs);
        break;    
    case 0b100010: // sub
        printf("inst %d: %08x sub $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b100011: // subu
        printf("inst %d: %08x subu $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;
    case 0b001100: // syscall
        printf("inst %d: %08x syscall\n", i, bits);
        break;
    case 0b100110: // xor
        printf("inst %d: %08x xor $%d, $%d, $%d\n", i, bits, rd, rs, rt);
        break;    
    default:
        printf("inst %d: %08x unknown instruction\n", i, bits);
    }
}

void J_format(int i, int bits) {
    int op = OP(bits);
    int add = ADD(bits);
    switch (op) {
    case 0b000010: // j
        printf("inst %d: %08x j %d\n", i, bits, add);
        break;
    case 0b000011: // jal
        printf("inst %d: %08x jal %d\n", i, bits, add);
        break;
    default:
        printf("inst %d: %08x unknown instruction\n", i, bits);
    }
}

void I_format(int i, int bits) {
    int op = OP(bits);
    int rs = RS(bits);
    int rt = RT(bits);
    short immd = IMM(bits);
    switch (op) {
    case 0b001000: // addi
        printf("inst %d: %08x addi $%d, $%d, %d\n", i, bits, rt, rs, immd);
        break;
    case 0b001001: // addiu
        printf("inst %d: %08x addiu $%d, $%d, %d\n", i, bits, rt, rs, (unsigned)immd);
        break;
    case 0b001100: // andi
        printf("inst %d: %08x andi $%d, $%d, %d\n", i, bits, rt, rs, immd);
        break;
    case 0b000100: // beq
        printf("inst %d: %08x beq $%d, $%d, %d\n", i, bits, rs, rt, immd);
        break;
    case 0b000101: // bne
        printf("inst %d: %08x bne $%d, $%d, %d\n", i, bits, rs, rt, immd);
        break;
    case 0b100000: // lb
        printf("inst %d: %08x lb $%d, %d($%d)\n", i, bits, rt, immd, rs);
        break;
    case 0b100100: // lbu
        printf("inst %d: %08x lbu $%d, %d($%d)\n", i, bits, rt, (unsigned)immd, rs);
        break; 
    case 0b100001: // lh
        printf("inst %d: %08x lh $%d, %d($%d)\n", i, bits, rt, immd, rs);
        break;
    case 0b100101: // lhu
        printf("inst %d: %08x lhu $%d, %d($%d)\n", i, bits, rt, (unsigned)immd, rs);
        break;           
    case 0b001111: // lui
        printf("inst %d: %08x lui $%d, %d\n", i, bits, rt, immd);
        break;
    case 0b100011: // lw
        printf("inst %d: %08x lw $%d, %d($%d)\n", i, bits, rt, immd, rs);
        break;
    case 0b001101: // ori
        printf("inst %d: %08x ori $%d, $%d, %d\n", i, bits, rt, rs, immd);
        break;
    case 0b101000: // sb
        printf("inst %d: %08x sb $%d, %d($%d)\n", i, bits, rt, immd, rs);
        break;    
    case 0b001010: // slti
        printf("inst %d: %08x slti $%d, $%d, %d\n", i, bits, rt, rs, immd);
        break;
    case 0b001011: // sltiu
        printf("inst %d: %08x sltiu $%d, $%d, %d\n", i, bits, rt, rs, (unsigned)immd);
        break;
    case 0b101001: // sh
        printf("inst %d: %08x sh $%d, %d($%d)\n", i, bits, rt, immd, rs);
        break;
    case 0b101011: // sw
        printf("inst %d: %08x sw $%d, %d($%d)\n", i, bits, rt, immd, rs);
        break;
    case 0b001110: // xori
        printf("inst %d: %08x xori $%d, $%d, %d\n", i, bits, rt, rs, immd);
        break;
    default:
        printf("inst %d: %08x unknown instruction\n", i, bits);
    }
}