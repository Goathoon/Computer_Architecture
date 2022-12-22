#include <stdio.h>
#include <stdlib.h>

void R2inst(int reg);
void IJ2inst(int reg);// I,J- TYPE 혼용.
void R2rstd(int reg);
void R2rstd_sa(int reg);
void R2rstd_shift(int reg);
void R2rs(int reg);
void R2rd(int reg);
void R2rst(int reg);
void I2tsi(int reg);
void I2branch_2(int reg);
void I2branch(int reg);
void J2address(int reg);

int main(int argc, char**argv)
{
    FILE *bf;
    bf= fopen(argv[1],"rb");
    int a, b, c, d;
    if(bf == NULL)
    {
        printf("파일을 찾을 수 없습니다");
        return 1;
    }
    
    int bin_arr[8];

    //R-type, I-type  J-type 구분을 위한 정보 (opcode 6자리를 10진법으로 바꿈)
    int I_arr[] = {8,9,12,4,5,32,36,33,37,15,35,13,40,10,11,41,43,14};
    int J_arr[] = {2,3};
    while (fscanf(bf ,"%d", bin_arr) != EOF) //바이너리파일 끝까지 탐색
    {
        int i = 0; // inst 순서 나타냄
        //register 에 저장된 비트를 int를 십진법으로 바꿔냄
        unsigned int reg = 0;
        for(int i = 0; i<8; i++)
        {
            fscanf(bf,"%1d",&bin_arr[i]); //바이너리 파일의 숫자들을 4byte 씩 끊어서 각 숫자들을 배열에 저장 ->그래야 instruction 따질 수 있음 4바이트씩이니까
            reg += bin_arr[i] <<((7-i)*4) ;
        }

        printf("inst %d: ", i);
        for(int i = 0; i<8 ; i++)
            printf("%d",bin_arr[i]);
        printf(" ");
        //opcode 6자리를 십진법으로 바꾼 값에 따라 type 결정
        if((bin_arr[0]<<2)+ (bin_arr[1]>>2) ==0)         //R-type
        {
            R2inst(reg);
            printf("\n");
        }
        else
        {
            IJ2inst(reg);
            printf("\n");
        }
    }
    

}

void R2inst(int reg) //R-type 은 아니지만 R- type으로 여겨지는 jr과 jalr이 존재
{
    switch(reg % (1<<6)){ //func 비트에 따라서 instruction 결정
        case 32: //add
            printf("add ");
            R2rstd(reg);
            break;
        case 33:
            printf("addu ");
            R2rstd(reg);
            break;
        case 36:
            printf("and ");
            R2rstd(reg);
            break;
        case 26:
            printf("div ");
            R2rstd(reg);
            break;
        case 27:
            printf("divu ");
            R2rstd(reg);
            break;
        case 9:
            printf("jalr ");
            R2rs(reg);
            break;
        case 8:
            printf("jr ");
            R2rs(reg);
            break;
        case 16:
            printf("mfhi ");
            R2rs(reg);
            break;
        case 18:
            printf("mflo ");
            R2rd(reg);
            break;
        case 17:
            printf("mthi ");
            R2rs(reg);
            break;
        case 19:
            printf("mtlo ");
            R2rs(reg);
            break;
        case 24:
            printf("mult ");
            R2rst(reg);
            break;
        case 25:
            printf("multu ");
            R2rst(reg);
            break;
        case 39:
            printf("nor ");
            R2rstd(reg);
            break;
        case 37:
            printf("or ");
            R2rstd(reg);
            break;
        case 0:
            printf("sll ");
            R2rstd_sa(reg);
            break;
        case 4:
            printf("sllv ");
            R2rstd_shift(reg);
            break;
        case 42:
            printf("slt ");
            R2rstd(reg);
            break;
        case 43:
            printf("sltu ");
            R2rstd(reg);
            break;
        case 3:
            printf("sra ");
            R2rstd_sa(reg);
            break;
        case 7:
            printf("srav ");
            R2rstd_shift(reg);
            break;
        case 2:
            printf("srl ");
            R2rstd_sa(reg);
            break;
        case 6:
            printf("srlv ");
            R2rstd_shift(reg);
            break;
        case 34:
            printf("sub ");
            R2rstd(reg);
            break;
        case 35:
            printf("subu ");
            R2rstd(reg);
            break;
        case 12:
            printf("syscall");
            break;
        case 38:
            printf("xor ");
            R2rstd(reg);
            break;
        default :
            printf("unknown instruction");
            break;
    }
}

void IJ2inst(int reg)
{
    switch(reg/(1<<26)){
        case 8:
            printf("addi ");
            I2tsi(reg);
            break;
        case 9:
            printf("addiu ");
            I2tsi(reg);
            break;
        case 12:
            printf("andi ");
            I2tsi(reg);
            break;
        case 4:
            printf("beq ");
            I2branch_2(reg);
            break;
        case 5:
            printf("bne ");
            I2branch_2(reg);
            break;
        case 32:
            printf("lb ");
            I2branch(reg);
            break;
        case 36:
            printf("lbu ");
            I2branch(reg);
            break;
        case 33:
            printf("lh ");
            I2branch(reg);
            break;
        case 37:
            printf("lhu ");
            I2branch(reg);
            break;
        case 15:
            printf("lui ");
            I2branch(reg);
            break;
        case 35:
            printf("lw ");
            I2branch(reg);
            break;
        case 13:
            printf("ori ");
            I2branch_2(reg);
            break;
        case 40:
            printf("sb ");
            I2branch(reg);
            break;
        case 10:
            printf("slti ");
            I2branch_2(reg);
            break;
        case 11:
            printf("sltiu ");
            I2branch_2(reg);
            break;
        case 41:
            printf("sh ");
            I2branch(reg);
            break;
        case 43:
            printf("sw ");
            I2branch(reg);
            break;
        case 14:
            printf("xoir ");
            I2branch_2(reg);
            break;
        // 여기서부터 j
        case 2:
            printf("j ");
            J2address(reg);
            break;
        case 3:
            printf("ja; ");
            J2address(reg);
            break;
        default:
            printf("unknown instruction");
            break;
    }

}       


void R2rstd(int reg)
{
    int all_r; //레지스터 위치에 해당하는 비트들을 정수로 표현 (shamt, funct, opcode 부분은 제외하고 생각하였다.)
    int rd;
    int rs;
    int rt;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rd = all_r % (1<<5);
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) % (1<<10);

    printf("$%d, $%d, $%d", rd,rs,rt);
}

void R2rst(int reg)
{
    int all_r;
    int rs;
    int rt;

    all_r = (reg%(1<<26)) >> 16;
    rt = all_r % (1<<5);
    rs = (all_r>>5) % (1<<5);

    printf("$%d, $%d", rs, rt);
}

void R2rs(int reg)
{
    int all_r;
    int rs;
    int rd;

    all_r = (reg%(1<<26)) >> 16;
    rs = (all_r >> 5) % (1 << 5);

    printf("$%d",rs);
}

void R2rd(int reg)
{
    int all_r;
    int rs;
    int rd;

    all_r = (reg%(1<<26)) >> 16;
    rd = all_r % (1 << 5);

    printf("$%d",rd);
}

void R2rstd_sa(int reg)
{
    int all_r; //레지스터 위치에 해당하는 비트들을 정수로 표현 (shamt, funct, opcode 부분은 제외하고 생각하였다.)
    int rd;
    int sa;
    int rt;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rd = all_r % (1<<5);
    rt = (all_r>>5) % (1<<5);
    sa = (reg % (1<<11))-(all_r%(1<<6)); //shamt (shift amount)

    printf("$%d, $%d, $%d", rd,rt,sa);
}

void R2rstd_shift(int reg)
{
    int all_r; //레지스터 위치에 해당하는 비트들을 정수로 표현 (shamt, funct, opcode 부분은 제외하고 생각하였다.)
    int rd;
    int rs;
    int rt;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rd = all_r % (1<<5);
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) % (1<<10);

    printf("$%d, $%d, $%d", rd,rt,rs);
}

void I2tsi(int reg)
{
    int all_r;
    int rs;
    int rt;
    int imm;
    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) % (1<<10);
    imm = (reg%(1<<11));

    printf("$%d, $%d, %d",rt,rs,imm);
}

void I2branch_2(int reg)
{
    int all_r;
    int rs;
    int rt;
    int offset;

    all_r = (reg % (1<<26)) >>11;  //opcode부분, shamt, funct부분 싹다 제외.
    rt = (all_r>>5) % (1<<5);
    rs = (all_r>>5) % (1<<10);
    offset = (reg%(1<<11));

    printf("$%d, $%d, %d", rs,rt,offset);
}

void I2branch(int reg)
{
    int all_r;
    int rs;
    int rt;
    int offset;

    all_r = (reg % (1 << 26)) >> 11; // opcode부분, shamt, funct부분 싹다 제외.
    rt = (all_r >> 5) % (1 << 5);
    rs = (all_r >> 5) % (1 << 10);
    offset = (reg % (1 << 11));

    printf("$%d, %d($%d)", rt,offset,rs);
}

void J2address(int reg)
{
    int address;

    address = reg%(1<<26);
    printf("%d", address);
}