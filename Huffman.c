#include <stdio.h>
#include <stdlib.h>

#define NUM_LEAFE 256
#define MAX_LEN_CODE 30
#define NUM_NODE 511

//哈夫曼树结点
typedef struct{
    unsigned char id;    //存的是 ASCII 码 0-255
    int weight;
    int parent, lchild, rchild;
}HTNode;

//编码表
typedef struct{
    unsigned code;
    int length;
}HTCODE;

//构造哈夫曼树
int creatHT(HTNode Node[] , long *Weight){
    long SumWeight = 0;    //记录权值总和
    int NotShow = 0;    //未出现得字符的个数

    //初始化：构造哈夫曼树，每个结点构成一棵树
    for (int i = 0 ; i < NUM_NODE ; i++){
        //初始化结点
        Node[i].id = i;
        Node[i].parent = Node[i].lchild = Node[i].rchild = -1;
        Node[i].weight = 0;
        //根据传入的 Weight 数组初始化叶节点
        if (i < NUM_LEAFE){
            Node[i].weight = Weight[i];
            if (Weight[i] == 0) NotShow++;    //统计未出现的字符数
            SumWeight += Weight[i];    //统计总权值
        }
    }

    //构造哈夫曼树
    int i = NUM_LEAFE;
    long MinWeightA , MinWeightB;    //记录最小的两个权值
    //从第 256 个结点开始循环，构造哈夫曼树
    while (i < NUM_NODE - NotShow){
        MinWeightA = MinWeightB = SumWeight + 10;    //初始化最小的权值为最大可能值 + 10
        unsigned PosA , PosB;    //记录权值最小的两个结点的位置
        //内层循环找到权值最小的两个结点
        for (int j = 0 ; j < i ; j++){
            if (Node[j].weight == 0) continue;
            if (Node[j].parent == -1){
                if (Node[j].weight < MinWeightA){
                    //更新 MinWeight 和 Pos
                    PosB = PosA;
                    MinWeightB = MinWeightA;
                    PosA = j;
                    MinWeightA = Node[j].weight;
                }else if (Node[j].weight < MinWeightB){
                    PosB = j;
                    MinWeightB = Node[j].weight;
                }
            }
        }
        //根据找到的权值最小的两个结点构造非叶节点
        Node[i].lchild = PosA;
        Node[i].rchild = PosB;
        Node[i].weight = MinWeightA + MinWeightB;
        Node[PosA].parent = Node[PosB].parent = i++;
    }

    return NUM_NODE - NotShow - 1;    //返回根节点的下标
}

//从文件中读入数据，统计各个字符出现的次数，返回 Weight 数组
unsigned char *ReadFile(const char filename[] , long *Weight , long *filesize){
    FILE *fp = fopen(filename , "rb");    //以二进制格式读入文件 filename
    if (fp == NULL){
        printf("Fail to read the file.");
        exit(0);
    }

    fseek(fp , 0 , SEEK_END);    //将文件指针移动到文件末尾
    *filesize = ftell(fp);    //获取文件大小
    rewind(fp);    //移动文件指针至文件开头

    unsigned char *Data = (unsigned char *)malloc((*filesize) * sizeof(unsigned char));    //读取文件数据
    if (Data == NULL){
        printf("Fail to malloc");
        exit(0);
    }
    fread(Data , sizeof(unsigned char) , *filesize , fp);
    fclose(fp);

    //初始化：权值数组置零
    for (int i = 0 ; i < NUM_LEAFE ; i++){
        Weight[i] = 0;
    }
    //统计所有字符出现的次数
    for (int i = 0 ; i < *filesize ; i++){
        Weight[Data[i]] += 1;
    }

    return Data;
}

void printHtree0(HTNode t[], int ridx) {
	if (ridx >= 0) { //递归出口 
		if (t[ridx].weight > 0){
			printf("%d->%ld\n", ridx < NUM_LEAFE ? t[ridx].id : t[ridx].id + NUM_LEAFE, t[ridx].weight);
        }
		printHtree0(t, t[ridx].lchild);
		printHtree0(t, t[ridx].rchild);
	}
}

//输入字符，输出哈夫曼编码
void getCode(HTNode Node[] , int Char , unsigned *Code , int *Lenth){
    int code = 0;    //编码（临时）
    int lenth = 0;    //编码长度（临时）
    unsigned MASK = 1;    //位掩码

    int parent = Node[Char].parent;
    while (parent >= 0){
        if (Node[parent].rchild == Char){
            code |= MASK;    //按位‘‘ 或 ’’
        }
        lenth++;

        Char = parent;
        parent = Node[Char].parent;

        MASK <<= 1;    //左移掩码
    }
    
    //返回值
    *Code = code;
    *Lenth = lenth;
}

//从哈夫曼树生成完整的哈夫曼编码表
void GenHCodeTable(HTNode Node[] , HTCODE TABLE[]){
    for (int i = 0 ; i < NUM_LEAFE ; i++){
        if (Node[i].weight > 0){
            getCode(Node , Node[i].id , &(TABLE[i].code) , &(TABLE[i].length));
        }
    }
}

/***  编解码实现 ***/
//利用哈夫曼编码表，将原长度为 OriginalLenght 的内容 OriginalContent 编码为长度为 NewLength 的内容为 NewContent 的文件
void EnCode(unsigned char* OriginalContent , long OriginalLength , unsigned char* NewContent , long *NewLength , HTCODE TABLE[]){
    long Oindex = 0;    //OriginalContent 的索引下标
    long Nindex = 0;    //NewContent 的索引下标

    //初始化
    NewContent[Nindex] = 0;
    unsigned Code;
    int Length;
    int UsedBits = 0;

    //对原始数据逐个字符处理
    while (Oindex < OriginalLength){
        int ID = OriginalContent[Oindex];
        Code = TABLE[ID].code;
        Length = TABLE[ID].length;

        //根据 NewContent 计算 Code 的有效长度
        while (Length + UsedBits >= 8){
            unsigned TempCode = Code >> (Length + UsedBits - 8);
            NewContent[Nindex] |= TempCode;
            Length = Length + UsedBits - 8;
            UsedBits = 0;
            NewContent[++Nindex] = 0;
        }

        //将编码写入 NewContent
        Code &= ((1 << Length) -1);
        Code <<= (8 - Length - UsedBits);
        NewContent[Nindex] |= Code;
        UsedBits += Length;
        Oindex++;
    }
     
    *NewLength = Nindex + 1;
}

//压缩文件
void Zip(const char Input[] , const char Output[]){
    HTNode Node[NUM_NODE];    //哈夫曼树
    HTCODE TABLE[NUM_LEAFE];    //编码表
    long Weight[NUM_LEAFE];    //字符权值
    long FILESIZE;    //文件大小

    //获取带压缩文件内容
    unsigned char* UnZipContent = ReadFile(Input , Weight , &FILESIZE);
    //生成哈夫曼树并记录根节点 root
    int root = creatHT(Node , Weight);
    //生成哈夫曼编码表
    GenHCodeTable(Node , TABLE);

    unsigned char* ZipContent;    //编码后文件内容
    long ZipSize;    //压缩后文件大小
    ZipContent = (unsigned char*)malloc(sizeof(unsigned char) * (FILESIZE + 10000));
    if (!ZipContent){
        printf("Fail to malloc.");
        exit(0);
    }
    //压缩编码
    EnCode(UnZipContent , FILESIZE , ZipContent , &ZipSize , TABLE);

    //保存压缩文件
    FILE* fp = fopen(Output , "wb");    //二进制写入
    if (!fp){
        printf("Fail to wright zip.");
        exit(0);
    }

    long HTreeSize = sizeof(HTNode) * NUM_NODE;    //哈夫曼树大小
    fwrite(&HTreeSize , sizeof(HTreeSize) , 1 , fp);    //将哈夫曼树大小写入输出文件
    fwrite(&ZipSize , sizeof(ZipSize) , 1 , fp);    //将编码后文件大小写入输出文件
    fwrite(&FILESIZE , sizeof(FILESIZE) , 1 , fp);    //原文件大小
    fwrite(&root , sizeof(root) , 1 , fp);    //哈夫曼树根节点下标
    fwrite(Node , sizeof(HTNode) , NUM_NODE , fp);    //保存哈夫曼树
    fwrite(ZipContent , sizeof(unsigned char) , ZipSize , fp);    //保存压缩编码

    //写入结束
    fclose(fp);
    free(ZipContent);
    free(UnZipContent);
    printf("Zipped file %s has being created successfully!\n" , Output);
}

//解压文件
void UnZip(const char Input[] , const char Output[]){
    FILE *fp = fopen(Input , "rb");
    if (fp == NULL){
        printf("Fail to open file.");
        exit(0);
    }

    long HTreeSize , ZipSize , UnZipSize;
    int root;
    fread(&HTreeSize , sizeof(HTreeSize) , 1 , fp);
    fread(&ZipSize , sizeof(ZipSize) , 1 , fp);
    fread(&UnZipSize , sizeof(UnZipSize) , 1 , fp);
    fread(&root , sizeof(root) , 1 , fp);

    unsigned char *ZipContent = (unsigned char*)malloc(ZipSize);
    unsigned char *UnZipContent = (unsigned char*)malloc(UnZipSize);

    HTNode Node[NUM_NODE];
    fread(Node , HTreeSize , 1 , fp);
    fread(ZipContent , ZipSize , 1 , fp);
    fclose(fp);

    fp = fopen(Output , "wb");
    if (fp == NULL) {
		printf("Fail to write file.\n");
		exit(0);
	}

    int index = root;
    int Zindex = 0 , UZindex = 0;
    int Control;
    while (UZindex < UnZipSize){
        Control = 128;    //10000000
        while (Control > 0){
            if ((ZipContent[Zindex] & Control) > 0){    //这一 bit 是 1，哈夫曼树向右访问
                if (Node[index].rchild == -1){    //叶子结点
                    UnZipContent[UZindex++] = Node[index].id;    //写入
                    index = root;
                    Control <<= 1;
                }else{
                    index = Node[index].rchild;
                }
            }else{    //这一 bit 是 0，哈夫曼树向左访问
                if (Node[index].lchild == -1){
                    UnZipContent[UZindex++] = Node[index].id;    //写入
                    index = root;
                    Control <<= 1;
                }else{
                    index = Node[index].lchild;
                }
            }
            Control >>= 1;
        }
        Zindex++;    //准备读取下一个字符
    }
    
    //写入输出文件
    fwrite(UnZipContent , UnZipSize , 1 , fp);
    fclose(fp);
    free(UnZipContent);
    free(ZipContent);
    printf("UnZipped done!\n");
}

int check(char file1[], char file2[]){	// 返回1说明文件一样，返回0说明文件不一样
	FILE *fp1 = fopen(file1,"rb"), *fp2 = fopen(file2,"rb");
	unsigned fsize1,fsize2;
	fseek(fp1, 0, SEEK_END);	fseek(fp2, 0, SEEK_END);
	fsize1 = ftell(fp1);		fsize2 = ftell(fp2);
	rewind(fp1);				rewind(fp2);
	if(fsize1 != fsize2) return 0;
	char c1,c2;
	for(unsigned i = 0; i < fsize1; ++i){
		fread(&c1,1,1,fp1);		fread(&c2,1,1,fp2);
		if(c1 != c2) return printf("at fsize = %d, c1 is %d, c2 is %d\n",i,c1,c2);
	}
	fclose(fp1);				fclose(fp2);
	return 1;
}

int main(){

    Zip("pic.png" , "pic.myzip");
    UnZip("pic.myzip" , "out_pic.png");
    check("pic.png" , "out_pic.png");

/*
    Zip("test" , "test.myzip");
    UnZip("test.myzip" , "out_test");
    check("test" , "out_test");
*/
    return 0;
}