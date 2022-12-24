# Computer_Architecture
2022-1 조형민 교수님 컴퓨터구조 개론
<hr/>

## 1차과제

**목표 : 해당 과제는 MIPS machine 코드 (binary) 를 disassemble 하여 assembly언어로 해석하는 것을 목표로 하고 있다.**

1. instruction name은 모두 소문자로 할것
2. register 들은 특정 이름이 아닌 숫자로 할것
3. 실제 주소값들은 모두 signed decimal로 할것
4. 구현해야할 instruction들 : 
add, addu , and, div, divu , jalr , jr , mfhi , mflo , mthi ,
mtlo , mult , multu , nor, or, sll , sllv , slt , sltu , sra ,
srav , srl , srlv , sub, subu , syscall , xor , addi , addiu ,
andi , beq , bne , lb , lbu , lh , lhu , lui , lw , ori , sb, slti ,
sltiu , sh , sw , xori , j, jal instruction 만 있다고 가정
5. 만일 해당 instruction에서 벗어난 binary 코드가 발견될 시에, unknown instruction 출력

### SUMMARY

MIPS 아키텍쳐의 instruction에 해당하는 바이너리 파일을 읽어서 assembly어로 출력하는 것이 이 과제의 목표이다.<br/>
따라서, 32비트씩 binary 파일을 읽어와서, int 배열에 저장해놓고 ('bin_arr[8]') 해당 배열에서 이러쿵 저러쿵 하면 쉽게 구현할 수 있다. (물론 메모리 낭비가 있다.)<br />
하지만, opcode 를 해석하기 위해서는 앞 6바이트가 필요하다. 따라서 해당 6바이트를 위해서 약간의 변조가 필요했다.<br/>
```c
(bin_arr[0]<<2)+ (bin_arr[1]>>2)
```
다음과 같은 코드를 이용하여 opcode를 추출해낼 수 있었고, 해당 opcode로 R type인지 I type인지 구분해낸다.<br/>
그 후, 나머지 26비트로 R type 내에서 or I type 내에서 instruction을 분석한다.<br/>
해당 함수로 
```c
R2inst(reg);
I2inst(reg);
```
를 선언하여 해석했다.<br/>
현재 reg는 int형 이므로 4바이트 즉, 32비트이다. <br>
reg에 들어갈 정보는
```c
for(int i = 0; i<8; i++)
        {
            fscanf(bf,"%1d",&bin_arr[i]); //바이너리 파일의 숫자들을 4byte 씩 끊어서 각 숫자들을 배열에 저장 ->그래야 instruction 따질 수 있음 4바이트씩이니까
            reg += bin_arr[i] <<((7-i)*4) ;
        }
```
이렇게 바이너리 파일을 4바이트씩 끊어서 각 숫자들을 배열에 저장한 후, 16로 저장되어 있는 bin_arr를 int 형 변수 reg 로 변환시켰다. <br/>
그 후에, reg에 앞 6비트 정보 = opcode (I type) 혹은 뒤 6비트 정보 = func(R type)를 토대로 무슨 instruction인지 프린트해내고, 각 instruction에 맞는 공통된 instruction 구조로 함수를 정의하여 구현했다.
<hr/>

## 2차과제

**목표 : 해당 과제는 MIPS instruction 들의 일부를 지원하는 instruction simulator를 만드는 과제이다.**

1. instruction들을 읽고 직접 실행시킬 것 (registrt value, memory contents의 변화만 고려한다.)
2. 프로그램 마지막에는 register들의 현재 value값들을 출력 시킬 것
3. 구현해야할 instruction 들: <br/>
  - Arithmetic/logical: add, addu , sub, subu , and, or, slt , sltu <br/>
  - Arithmetic/logical with immediate: addi , addiu , andi , ori , lui , slti , sltiu <br/>
  - Control transfer: beq , bne , j, jal , jr <br/>
  - Shift instructions: sll , srl <br/>
  - Memory access: lw , sw <br/>
  - No need to implement lh , lhu , lb , sh , shu , sb <br/>

4. add, sub, addu, subu, addiu, addi 에서 signed unsigned를 구분 지을 필요는 없다.
5. 다만, slt, slti instruction에서 는 구분 지을 것.

#### 구현해야할 data structure

- Register
모든 레지스터들의 초기 값들은 0x00000000 이다.
레지스터들은 모두 $0 ~ $31이라고 가정한다. (NAME 부여하지 않음)

- Instruction Memory
주소값 범위는 0x00000000 ~ 0x00010000 (4KB)라고 가정한다.
모든 data 들은 0xFF라고 가정한다. (word 같은 경우 0xFFFFFFFF)

- Data Memory
실제 CPU와 달리, Data Memory는 Instruction Memory와 분리되어 있다고 가정한다.
주소값 범위는 0x10000000 ~ 0x100100000 (4KB) 라고 가정한다.

#### 실행 방법
1. 2 ~ 3 개의 Command Line을 인자로 입력받는다.
2. 첫번째 인자에는 실행시킬 instruction들의 수
3. 두번째 인자에는 instruction 확장자인 binary 파일 이름
4. optional 인자로, data memory의 값들을 저장시킨 data 파일 (시작 주소는 0x10000000 임에 유의)
5. unknown instruction이 주어지면, PC+4의 unknown instruction 을 실행시키고 종료시킬것.

### SUMMARY

Instruction 전체를 한꺼번에 다루기 위해서, Instruction 구조체를 다음과 같이 선언했다.
```c
typedef struct _Instruction //Instruction 전체를 한꺼번에 다루기 위한 구조체 선언
{
    char op[7] ; //opernad에 해당하는 문자 eg.addi,slti
    int rs;
    int rt;
    int rd;
    int immValue;
    int address;
    int shamt;
    int offset;
}Instruction;
```
그리고, register 에 value들을 저장할 배열, 그리고 instruction memory, data memory를 구현해야만 했다.
```c
unsigned int registers[33][2]; //전역변수로 초기화 세로 32번쨰 index는 PC레지스터라 가정.
unsigned int inst_memory[16384][2]; //16384 는 instruction memory의 range를 4로 나눈 값임. 어차피 주소는 4바이트씩 읽으므로, 4로나눈 만큼의 배열공간이 설정되면 된다. 
                                    //1열은 "주소값/4" 를 의미하고, 2열은 각 주소(같은 행의 주소)값에 들어갈 실제 value를 의미한다. 이거는 모두 0xFFFFFFFF으로 초기화시켜주어야함.
unsigned int data_memory[65536][2]; //inst_memory와 달리 1열에는 0x10000000이 +된 값으로 초기화. 여기서는 4로나누면 안됨. 연속적으로 ,data가 주소에 저장되므로.
```
다음은 초기화 과정이다.
```c
    //registers 배열은 2차원배열로, 첫번째 열에는 순차적으로 0~31의 레지스터의 넘버링이 부여되고, (32번째는 PC register = 다음 fetch 될 instruction 주소 가리킴)
    //두번째 열에는 그 레지스터에 들어있는 value값이 저장된다.
    //후에 결과값을 print할 때에는 0을 8개의 값으로 도출하게끔, 적절히 변형하여 출력할 예정
    for(int i = 0; i<32; i++)
    {
        registers[i][0] = i; //32개 레지스터 넘버링 부여
    }

    //inst_memory 초기화.
    for (int i = 0; i < 16384; i++)
    {
        inst_memory[i][1] = UINT_MAX; //모두 0xFFFFFFFF으로 초기화시켜주어야함.
    }

    for (int i = 0; i < 65536; i++)
    {
        data_memory[i][0] += ((1<<28) + i); //data_memory 가상의 주소값(0x10000000이 초기값임에 유의)에 맞게끔 1열에다 더해줌 그리고 추가적으로 주소값 변동 i 더해준다.
        data_memory[i][1] = 255; //모두 0xFF으로 초기화시켜주어야함.
        
    }
```
다음은 각각 Instruction Memory와 Data Memory에 인자로 받은 binary .inst 파일과 .data 파일에서 값을 저장하는 과정이다.<br/>
물론 여기에서 instruction 그리고 data 들을 int형 배열 bin_arr, data_arr 에 저장해서 메모리가 많이 낭비된다. 하지만 당시에는 구현에 어려움이 있어서, 메모리 낭비를 감안했다. <br/>
그러므로 bin_arr 에서 각 원소들이 실질적으로 4바이트 공간을 차지하지만, 구현 의도는 각 원소들을 4비트를 담는다고 가정하자.
```c
// Instruction Memory 저장 과정
while ((ch1 = fgetc(fp)) != EOF) // 1바이트씩 읽기로하자. 먼저 binary파일을 쭉읽고, instruction memory 에 다저장한 다음에, instruction을 수행할 것임. 지금 이 반복문은 '저장'단계이다.
    {
        bin_arr[k1] = ch1 >> 4;
        bin_arr[++k1] = ch1 % (1 << 4);
        k1++;
        if (k1 == 8)//1바이트읽을때마다 k두번 증가시켰으므로, 총 4바이트 읽은거임.
        {
            for (int j = 0; j < 8; j++)
            {
                reg1 += bin_arr[j] << ((7 - j) * 4);
            }
            inst_memory[cur1][1] = reg1;
            cur1++;
            k1 = 0;
            reg1 = 0;
        }
    }
    
    // Data Memory 저장 과정
    if(fp2!=NULL)
    {
        while ((ch2 = fgetc(fp2)) != EOF)
        {
            data_arr[k2] = ch2 >> 4;
            data_arr[++k2] = ch2 % (1 << 4);
            k2++;
            if (k2 == 2) // 1바이트읽을때마다 k두번 증가시켰으므로, 총 4바이트 읽은거임.
            {

                reg2 += (data_arr[0] * 16 );
                reg2 += data_arr[1];

                data_memory[cur2][1] = reg2;
            }
            cur2++;
            k2 = 0;
            reg2 = 0;
        }
    }
```

이후에는, N개의 입력받은 instruction을 수행한 경우에, 약간의 예외 처리와 함께 register 값들을 출력하는 과정을 거친다. <br/>
N개의 instruction을 수행하는 과정에서는 PC register와 동일한 역할을 하는 divided_current_address 변수를 적절히 조절하여 과정을 진행한다. <br/>
기본적인 알고리즘 순서는, divided_current_address index에 존재하는 inst_memory (기억하자, inst_memory의 index는 기본적으로 주소값을 4로 나눈값임을) value값을 통해 Instruction type을 fetch 한 후, 해당 instruction을 수행한다. <br/>
이 때 만약 jal과 같은 경우에 $ra 레지스터 (이 프로젝트에서는 $31로 지정) 에 다음 명령어의 주소를 저장한다. <br/>
그렇지 않은 경우에는 1차과제와 마찬가지로, 해당 instruction에 맞게끔 각각 opcode, shamt, immediate value 등과 같은 값들을 Instruction 구조체를 이용하여 저장시킨다. <br/>
저장된 구조체를 이용하여 register value 혹은 memory value들을 바꾸는 알고리즘을 적용시켰다.

## 3차과제

**목표 : 2차 과제까지 구현했던 결과물에 cache behavior를 추가하기**

1. 마지막 instruction 이후에, 전체 cache hit과 miss된 숫자를 출력해야 한다.
2. instruction cache 는 고려하지 않고, 오직 data cache만 고려한다. (lw, sw Instruction만 고려)
3. Cache에는 두가지 종류가 있다. 

|캐시|용량|캐쉬 구조|Write policy|주소 bit|block size|
|---|---|---|---|---|---|
|Cache 1|1KB|Direct-mapped|Write-through & no write allocate|32bit 주소|block size는 변수로, command line에서 input|
|Cache 2|4KB|2-Way set associative, LRU replacement|Write-back & write allocate| 32bit 주소|block size는 변수로, command line에서 input|

4. input은 첫번째에는 Cache type, 두번째에는 block size, 세번째에는 2차과제 input을 순서대로 받는다.<br/>
예시) ./mips-sim 2 16 100 test1.inst test1.data (여기서 16은 byte로써, 4words 를 의미한다)

5. 유효한 block size 제한<br/>
- block size는 무조건 word size인 4바이트보다 크다.
- block size는 Cache capacity / Set Associative 보다 작거나 같다.
- block size는 항상 order of 2 이다.
- 여기서 invalid 한 cache size input에 대한 에러 처리는 안해도 무방. (항상 valid한 input만 받는다고 가정)

6. output 양식
```c
Instructions: 222
Total: 14
Hits: 10
Misses: 4
```

### SUMMARY
hit 구조체를 추가로 선언.
hit 정보를 나타내기위한 구조체로써,
총 instruction 수, cache hit의 수, miss 된 수를 저장함.
```c
typedef struct _Hit //Hit 정보 나타내기 위한 struct
{
    int instN;
    int hitN;
    int missN;
}Hit;
```
그리고, cache 구현을 위한 cache 배열을 두개 선언했다. <br/>
cache1 의 첫번째 열에는 'valid bit', 두번째 열에는 'tag bit'이 들어간다.<br/>
cache2 의 첫번째 열에는 'LRU', 두번째 열에는 valid bit', 세번째 열에는 'dirty bit', 네번째 열에는 'tag'가 들어간다.<br/>
```c
unsigned int cache1[1024][2];
unsigned int cache2[4096][4];
```




2차과제에서 추가된 main 함수속 while문을 보면서 어떻게 구현했는지 설명하겠다.
```c
while (1) // Instruction 수행의 단계
    {

        if(hit.instN==N)
        {

            PrintHit(&hit);
            break;
        }
        else
        {
            if(divided_current_address >=1) //이부분은 예외처리 부분임. (unknown instruction 주어질 때 종료, 신경 안써도 된다)
            {

                if (inst_memory[divided_current_address - 1][1] == UINT_MAX)
                {
                    PrintHit(&hit);
                    break;
                }
            }
        }

        if(inst_memory[divided_current_address][1]/(1<<26)==0) //R-type
        {
            registers[32][1] +=4;
            R2inst(inst_memory[divided_current_address ][1] , Inst);
        }

        else
        {
            registers[32][1] +=4;
            if(inst_memory[divided_current_address ][1]/(1<<26) ==3) //jal일경우
            {
                registers[31][1] = (divided_current_address +1)*4; //$ra에 저장 (PC+4)./m
            }

            IJ2inst(inst_memory[divided_current_address][1], Inst, block_size, type,&hit);
        }
        hit.instN++;
        divided_current_address = registers[32][1]/4;
    }
```

while문을 무한히 반복하면서 instruction 하나를 읽을 때 마다 hit.instN 을 1씩 증가시킨다.<br/>
그러다가, instN이 command로 입력받은 instruction 수와 같아질 때, cache hit 과 cache miss를 출력하면서 프로그램을 종료한다. <br/>
2차과제와 달라지는 점이 거의 없다. <br/>
**하지만 추가된 함수가 두가지 있다.**

>추가된 함수를 설명하기 전에, block address의 구조에 대해 짚고 넘어가자. <br/>
기본적인 block address 의 구조는 다음과 같다. <br/>
**TAG - INDEX - OFFSET** 으로 구성된다. <br/>
offset은 block size에 의해 정해진다. (log2( words/block * 4)) <br/>
index는 block 수에 의해 정해진다. (Cache capacity / bytes of block)<br/>
tag는 나머지 bit를 차지한다. <br/>
```c
void Iinst2reg_Cache1(Instruction *inst,int block_size, unsigned int d[][2],Hit *hit);
void Iinst2reg_Cache2(Instruction *inst,int block_size, unsigned int d[][2],Hit *hit);
```
각각은 cache1과 cache2를 구현하기 위한 함수이다. <br/>
cache1 부터 살펴보자.

### - Cache 1

```c
void Iinst2reg_Cache1(Instruction *inst,int block_size, unsigned int d[][2],Hit *hit)
{
    // cache배열의 첫번째 열은 valid bit 두번째 열은 tag bits
    unsigned int cache_address = (inst->offset) + d[inst->rs][1]; //내가 load 하고싶은 위치의 주소.
    // 1024000/blocksize 는 총 블록의 개수.
    // log2(block num) = index address
    // log2(blocksize) = offset 부분
    unsigned int offsetBits = log2(block_size); 
    unsigned int blockaddress = cache_address / (1 << offsetBits);
    unsigned int indexBits = log2(1024 / block_size);
    unsigned int index = blockaddress % (1 << indexBits);
    unsigned int tag = blockaddress / (1 << indexBits);
    if(!strcmp(inst->op, "lw"))
    {
        if(cache1[index][0] == 1 ) //valid bit이 1이면 hit을 만족하기 위한 전제조건 완성.
        {
            if(cache1[index][1] == tag) //hit이 발생
            {
                hit->hitN ++;
            }
            else
            {
                hit->missN ++;
                cache1[index][0] = 1;
                cache1[index][1] = tag;
            }
        }
        else //valid하지 않을때
        {
            hit->missN++;
            cache1[index][0] = 1;
            cache1[index][1] = tag;
        }
    }
    else if(!strcmp(inst->op,"sw"))
    {
        if(cache1[index][0] == 1 ) //valid bit이 1이면 hit을 만족하기 위한 전제조건 완성.
        {
            if(cache1[index][1] == tag) //hit이 발생 ->cache memory상에도 data값의 변화가 존재는 하지만 여기선 굳이 표현할 필요x
            {
                hit->hitN ++;
            }
            else
            {
                hit->missN ++;
                //아무것도 하지 않아도 됩니다. (memory의 값을 바꾸는 함수가 이미 선행되었기 떄문)
            }
        }
        else //valid하지 않을때
        {
            hit->missN++;
            //마찬가지
        }
    }
}
```


먼저 load하고자 하는 cache의 주소에 접근하기 위해, **instruction의 offset 부분**과 lw든 sw든 **rs**부분이 곧 dst 와 src가 되므로, 해당 **rs부분과 offset을 더하여** cache address를 구한다.<br/>
그 후, 인자로 넘겨받은 block size를 이용해 offset bits를 계산하고, block address, index bits등 구현에 필요한 부분을 다 구해놓고 시작한다.<br/>
여기서 lw 와 sw 만 고려하면 되므로, strcmp로 instruction의 op 부분이 lw인지 ,sw인지 확인후, 각각에 맞는 실행방식을 구현하였다. <br/>
cache1의 첫번째 열은 valid bit이므로, valid bit이 1 이면 hit이, 아니면 miss이다. miss 일 경우, 해당 cache address를 valid하게 바꾸고, cache에 해당 정보를 load시켜야 할 것이다. <br/>
valid bit이 1이더라도, tag가 다르면, 다른 block이므로 miss할때와 마찬가지로 cache memory를 갱신한다. <br/>
이 때, lw와 sw로 바뀐 register와 memory의 상태는 Iinst2reg_Mem 함수로 이미 구현한 상태이므로, 신경쓰지 않아도 무방하다.
```c
void Iinst2reg_Mem(Instruction *inst,unsigned int d[][2], unsigned int arr[][2]) //inst, regis, data_memory 순의 arguement
{
    if(!strcmp(inst->op, "lw"))
    {
        int num1=0;
        for(int i = 0; i<65536; i++)
        {
            if(inst->offset + d[inst->rs][1] == arr[i][0])
            {
                num1 = i; //찾는 주소값이 있있는 가로 index 탐색.
                break;
            }
        }
        d[inst->rt][1] = 0;
        for(int j = 0; j<4; j++)
        {
            d[inst->rt][1] += arr[num1++][1] <<4*(6-2*j);
        }
        

    }
    else if(!strcmp(inst->op, "sw"))
    {
        int num2=0;
        for (int i = 0; i < 65536; i++)
        {
            if (inst->offset + d[inst->rs][1] == arr[i][0])
            {
                num2 = i; //찾는 주소값이 있있는 가로 index 탐색.
                break;
            }
        }
        arr[num2++][1] = d[inst->rt][1]/(1<<24);
        arr[num2++][1] = (d[inst->rt][1]%(1<<24))/(1<<16);
        arr[num2++][1] = (d[inst->rt][1]%(1<<16))/(1<<8);
        arr[num2++][1] = (d[inst->rt][1]%(1<<8));
    }
}

//다음과 같이 구현함 (예시만 가져왔음)
case 35:
            strcpy(inst->op,"lw");
            I2branch(reg,inst); 
            if(type==1)
                Iinst2reg_Cache1(inst,blocksize,registers, hit);
            else
                Iinst2reg_Cache2(inst,blocksize,registers, hit);
            Iinst2reg_Mem(inst, registers, data_memory); // load data 한 후에 miss,
            break;
```

### - Cache 2

```c
void Iinst2reg_Cache2(Instruction *inst,int block_size, unsigned int d[][2],Hit *hit)
{
    //cache2배열의 열의 순서는 순서대로 1-LRU 2-VALID 3-DIRTY 4=-TAG
    unsigned int cache_address = inst->offset + d[inst->rs][1]; //내가 load 하고싶은 위치의 주소.
    // 1024000/blocksize 는 총 블록의 개수.
    // log2(block num) = index address
    // log2(blocksize) = offset 부분
    unsigned int offsetBits = log2(block_size); //6
    unsigned int blockaddress = cache_address / (1 << offsetBits);
    unsigned int indexBits = log2(4096 / block_size/2); //수정해야할수도있는부분
    unsigned int index = blockaddress % (1 << indexBits);
    unsigned int tag = blockaddress / (1 << indexBits);
    if(!strcmp(inst->op, "sw"))
    {

        if(cache2[2*index][1] ==1 && cache2[2*index+1][1] ==1) //한 set내 두 cache line이 valid한 경우
        {
            if(cache2[2*index][3] == tag)
            {
               hit->hitN++;
               cache2[2*index+1][0] = 1;
               cache2[2*index][0] = 1;
            }
            else if(cache2[2*index+1][3] == tag)
            {
               hit->hitN++;
               cache2[2*index+1][0] = 0;
               cache2[2*index][0] = 0;
            }
            else //miss 가 발생한 경우
            {
                hit->missN++;
                cache2[2*index+cache2[2*index][0]][2] = 1; //해당 LRU에 해당하는 것을 replace하기위해 해당 index cache line을 dirty bit으로 교체  
                cache2[2*index+cache2[2*index][0]][3] = tag; //tag replacement 해당 lru. data는 신경x
                if(cache2[2*index][0] ==0) //lru가 0인경우 1로 변화
                {
                    cache2[2*index][0] = 1;
                    cache2[2*index+1][0] = 1;
                }
                else
                {
                    cache2[2*index][0] = 0;
                    cache2[2*index+1][0] = 0;
                }

            }
    

        }
        else if(cache2[2*index][1] ==1)//둘중하나만 valid한 경우 그중 첫번째가 valid (내가 이렇게 설정함. 무조건 첫번쨰거먼저 replace)
        {
            if(cache2[2*index][3]==tag)
            {
               hit->hitN++;
               cache2[2*index+1][0] = 1;
               cache2[2*index][0] = 1;
            }
            else //valid하지 않은 cacheline에 memory 저장
            {
                hit->missN++;
                cache2[2*index+1][1] = 1;
                cache2[2*index+1][2] = 1;
                cache2[2*index+1][3] = tag;
                cache2[2*index][0] = 0; //lru 변화
                cache2[2*index+1][0] = 0; //lru변화
            }
        }
        else//둘다 invalid
        {
            hit->missN++;
            cache2[2*index][0] = 1;
            cache2[2*index+1][0] = 1;
            cache2[2*index][1] = 1; //valid하게 바꿈
            cache2[2*index][2] = 1; //dirty bit
            cache2[2*index][3] = tag;
        }
        
    }
    else if(!strcmp(inst->op,"lw"))
    {
        if(cache2[2*index][1] ==1 && cache2[2*index+1][1] ==1) //한 set내 두 cache line이 valid한 경우
        {
            if(cache2[2*index][3] == tag)
            {
               hit->hitN++;
               cache2[2*index+1][0] = 1;
               cache2[2*index][0] = 1;
            }
            else if(cache2[2*index+1][3] == tag)
            {
                hit->hitN++;
                cache2[2 * index + 1][0] = 0;
                cache2[2 * index][0] = 0;
            }
            else
            {
                hit->missN++;
                cache2[2*index+cache2[2*index][0]][3] = tag; //tag replacement 해당 lru. data는 신경x
                if(cache2[2*index][0] ==0) //lru가 0인경우 1로 변화
                {
                    cache2[2*index][0] = 1;
                    cache2[2*index+1][0] = 1;
                }
                else
                {
                    cache2[2*index][0] = 0;
                    cache2[2*index+1][0] = 0;
                }
            }

        }
        else if(cache2[2*index][1] ==1)//둘중하나만 valid한 경우 그중 첫번째가 valid (내가 이렇게 설정함. 무조건 첫번쨰거먼저 replace)
        {
            if(cache2[2*index][3]==tag)
            {
                hit->hitN++;
                cache2[2 * index + 1][0] = 1;
                cache2[2 * index][0] = 1;
            }
            else //valid하지 않은 cacheline에 memory 저장
            {
                hit->missN++;
                cache2[2*index+1][1] = 1;
                cache2[2*index+1][3] = tag;
                cache2[2*index][0] = 0; //lru 변화
                cache2[2*index+1][0] = 0; //lru변화
            }
        }
        else
        {
            hit->missN++;
            cache2[2*index][0] = 1;
            cache2[2*index+1][0] = 1;
            cache2[2*index][1] = 1; //valid하게 바꿈
            cache2[2*index][3] = tag;
        }
    }
}

```
#### cache1 과 다른점들
1. 2-Way set associative 이므로, index bit을 구할 때, 2를 추가로 나눠줬다.
2. 2-Way set associative 이므로, 한 세트를 연속된 두 행으로 생각했다. 그래서 cache[2*index]와 cache[2*index+1]은 서로 같은 set에 포함된 주소값이다.

##### 한 세트 내 두가지 cache line이 valid한 경우

3. 한 세트내에서 두가지 cache line이 valid한 경우 두 cache line을 살펴보면서 tag가 일치하는지 **모두** 확인해야만 한다. 물론, 일치하는 tag가 존재하면 cache hit이므로 바로 꺼내서 lw이든 sw든 과정을 거치면 되겠지만, 둘 다 miss 할 경우에 LRU replacement 기법에 따라, 먼저 cache에 등록된 cache 정보가 교체되어야 하기 때문이다.
4. valid한 bit을 확인 후에, 같은 tag를 가진 cache 정보가 존재한다면, valid bit을 해당 cache line이 아닌, 같은 세트 내에 존재하는 다른 cache line을 가리키게 LRU F를 조절해야한다.
다음은 해당 부분의 예시이다.
```c
if(cache2[2*index][3] == tag)
{
    hit->hitN++;
    cache2[2*index+1][0] = 1;
    cache2[2*index][0] = 1;
}
 ```

 
 5. 두가지 cache line에서 miss 가 발생할 경우, LRU bit가 가리키는 cache line을 교체해야 한다. 바꿀 때에는 dirty bit을 1로 고쳐서 write-back을 구현할 수 있게 하고, LRU bit 또한 반대쪽 cache line을 가리키게 바꾼다.
 ```c
else //miss 가 발생한 경우
    {
        hit->missN++;
        cache2[2*index+cache2[2*index][0]][2] = 1; //해당 LRU에 해당하는 것을 replace하기위해 해당 index cache line을 dirty bit으로 교체  
        cache2[2*index+cache2[2*index][0]][3] = tag; //tag replacement 해당 lru. data는 신경x
        if(cache2[2*index][0] ==0) //lru가 0인경우 1로 변화
        {
            cache2[2*index][0] = 1;
            cache2[2*index+1][0] = 1;
        }
        else
        {
            cache2[2*index][0] = 0;
            cache2[2*index+1][0] = 0;
        }

    }
```

##### 한 세트 내에서 한가지 cache line만 valid 한 경우 

6. 항상 2의 배수에 해당하는 행부터 lw 와 sw 정보를 참조했다, 따라서, 만일, 둘다 valid한 경우가 아니라면, 항상 2\*index 행이 먼저 valid하다.
```c
else if(cache2[2*index][1] ==1)//둘중하나만 valid한 경우 그중 첫번째가 valid (내가 이렇게 설정함. 무조건 첫번쨰거먼저 replace)
{
    if(cache2[2*index][3]==tag)
    {
       hit->hitN++;
       cache2[2*index+1][0] = 1;
       cache2[2*index][0] = 1;
    }
    else //valid하지 않은 cacheline에 memory 저장
    {
        hit->missN++;
        cache2[2*index+1][1] = 1;
        cache2[2*index+1][2] = 1;
        cache2[2*index+1][3] = tag;
        cache2[2*index][0] = 0; //lru 변화
        cache2[2*index+1][0] = 0; //lru변화
    }
}
```

##### 두가지 모두 invalid한 경우 (아무것도 들어와 있지 않음)
```c
else//둘다 invalid
{
    hit->missN++;
    cache2[2*index][0] = 1;
    cache2[2*index+1][0] = 1;
    cache2[2*index][1] = 1; //valid하게 바꿈
    cache2[2*index][2] = 1; //dirty bit
    cache2[2*index][3] = tag;
}
```
다음과 같이 cache에 아무것도 저장되어 있지 않다면, 2\*index 행부터 valid하게 바꾸었다.

sw instruction만 요약했지만, lw instruction 또한 같은 과정이므로 생략하겠다.


## 아쉬운 점
3차 과제에서 몇십만개의 data 와 instruction을 처리하는데, 시간이 많이 걸렸었다.
