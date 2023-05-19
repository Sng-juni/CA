#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
int *cache;
int Reg[32];
int PC;
int Max_PC;
int cnt;
int real_cnt;

int bits;
int total;

int hits;
int cache_type;
int cache_size;
int block_size;
int set_size;
int set_cnt;

int tag_cnt = 0;
int index_cnt = 0;
int offset_cnt = 0;

void read_file(const char* path, char dest[]);
int get_bits(char mem[], int add);
void R_format(int bits);
void J_format(int bits);
void I_format(int bits);
void init();
void r_cache(unsigned address);
void w_cache(unsigned address);


int main(int argc, char* argv[]) {
    if (argc < 5 || argc > 6) {
        printf("파일 읽기 에러!\n");
        exit(1);
    }
    cache_type = atoi(argv[1]);     // cache 타입
    block_size = atoi(argv[2]);     // 블럭 사이즈
    init();
    memset(inst, -1, sizeof(inst)); // inst - 0xff로 초기화
    memset(data, -1, sizeof(data)); // data - 0xff로 초기화
    cnt = atoi(argv[3]);

    if (argc == 5) {                //inst 파일만
        read_file(argv[4], inst);       
    }
    else if (argc == 6) {           //inst, data 파일 둘 다
        read_file(argv[4], inst);      
        read_file(argv[5], data);
    }

    while (cnt-- && PC <= Max_PC) {
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
        real_cnt++;
    }
    
    printf("Instructions: %d\n", real_cnt);
    printf("Total: %d\n", total);
    printf("Hits: %d\n", hits);
    printf("Misses: %d\n", total - hits);

    free(cache);

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
        r_cache(Reg[rs] + immd);
        Reg[rt] = get_bits(data, Reg[rs] + immd - 0x10000000);
        break;
    case 0b101011: // sw
        w_cache(Reg[rs] + immd);
        for (int i = 0; i < 4; i++) {
            data[Reg[rs] + immd - 0x10000000 + i] = (Reg[rt] >> (8 * (3 - i))) & 0xff;
        }
        break;
    }
    PC += 4;
}

double log2(double x) {
    return log(x)/log(2);
}

void init() {
    if (cache_type == 1) {
        cache_size = 1024;
        set_size = block_size * 1;
        set_cnt = cache_size / set_size;
    }
    else {
        cache_size = 4096;
        set_size = block_size * 2;
        set_cnt = cache_size / set_size;
    }

    cache = malloc(set_cnt * sizeof(int));

}

void r_cache(unsigned address) {
    int tagbit = 0;
    int indexbit = 0;

    unsigned _addr = address;
    _addr >>= (2 + (int)logb(block_size / 4)); 
    indexbit = _addr % (1 << ((int)logb(set_cnt)));
    _addr >>= (int)logb(set_cnt);
    tagbit = _addr;
    
    total++;

    for (int i = 0; i < set_cnt; i++) {
        if (i == indexbit) {
            if (cache[i] == tagbit) {
                hits++;

                return;
            }
            else {
                cache[i] = tagbit;
                
                return;
            }
        }
    }

    
    return;

}

void w_cache(unsigned address) {
    int tagbit = 0;
    int indexbit = 0;

    unsigned _addr = address;
    _addr >>= (2 + (int)logb(block_size / 4)); 
    indexbit = _addr % (1 << ((int)logb(set_cnt)));
    _addr >>= (int)logb(set_cnt);
    tagbit = _addr;

    total++;

    for (int i = 0; i < set_cnt; i++) {
        if (i == indexbit) {
            if (cache[i] == tagbit) {
                hits++;

                return;
            }
        }
    }
    
    if (cache_type ==2) {
        for (int i = 0; i < set_cnt; i++) {
            if (cache[i] != tagbit) {
                cache[i] = tagbit;

                return;
            }
        }
    }

    return;

}
