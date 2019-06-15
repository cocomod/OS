#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
using namespace std;
#define B 10
#define C 10//柱面数
#define H 8//磁头数
#define S 8//扇区数
//磁盘的存储块总数  块号=柱面号*（磁头数*扇区数）+磁头号*扇区数+扇区号
#define L 712
#define K 100
#define BUSY 1
#define FREE 0
#define FILE_BLOCK_LENGTH (B-3)
#define FILE_NAME_LENGTH (B-1)
#define FILE_SIGN_AREA ((L-1-K)/B+1)
#define FILE_NUM FILE_BLOCK_LENGTH
#define INPUT_LENGTH 100
#define OUTPUT_LENGTH 100
#define BUFFER_LENGTH 25
#define MENU 0
#define CREATE 1
#define DELETE 2
#define OPEN 3
#define CLOSE 4
#define READ 5
#define WRITE 6
#define HELP 7
#define EXIT 8
#define OPLIST 9
#define CH_LENGTH 20
//文件描述符
struct file_description {
	int file_length;//文件长度
	int file_description_flag;//占用标识位
	int file_block;//文件分配的磁盘块号
	int file_block_ary[FILE_BLOCK_LENGTH];//文件分配的磁盘块号数组
};
//目录
struct contents {
	char filename[FILE_NAME_LENGTH];//文件名
	int file_length;//文件大小
	int file_descriptionnum;//文件描述符序号
};
//已打开的文件
struct openfile_list {
	char buffer[BUFFER_LENGTH];//读写缓冲区,存读写内容
	int pointer[2];//读写指针
	int file_descriptionnum;//文件描述符
	int flag;//占用符
	int length;//占用缓冲区大小
};


char disk_model[C][H][S];//磁盘模型，其中CHS 分别表示柱面号，磁头号和扇区号
//z=BlockNum/(H*S);//柱面号=块号/（磁头数*扇区数）
//c=(BlockNum%(H*S))/S;//磁头号=（块号%（磁头数*扇区数）)/扇区数
//s=(BlockNum%(H*S))%S;//扇区号=（块号%（磁头数*扇区数）)%扇区数
char disk[L][B];
openfile_list open_list[FILE_NUM];//已打开文件集合
contents file_list[FILE_NUM];//所有文件集合

int lseek(int index, int pos);//把文件的读写指针移动到pos指定的位置
//初始化字符数组
void Init_block(char *tp, int length) {
	int i;
	for (i = 0; i<length; i++)
		tp[i] = '\0';
}
//该函数把逻辑块i的内容读入到指针p指向的内存位置，拷贝的字符个数为存储块的长度B
void read_block(int i, char *p) {
	char *tp = (char *)malloc(sizeof(char));
	tp = p;
	int z, c, s;
	z = i / (H*S);//柱面号=块号/（磁头数*扇区数）
	c = (i % (H*S)) / S;//磁头号=（块号%（磁头数*扇区数）)/扇区数
	s = (i % (H*S)) % S;//扇区号=（块号%（磁头数*扇区数）)%扇区数
	for (int a = 0; a<B;) {
		*tp = disk_model[z][c][s];
		*tp = disk[i][a];
		a++;
		tp++;
	}
}
//该函数把指针p指向的内容写入逻辑块i，拷贝的字符个数为存储块的长度B
void write_block(int i, char *p) {
	char *tp = (char *)malloc(sizeof(char));
	tp = p;
	int z, c, s;
	z = i / (H*S);//柱面号=块号/（磁头数*扇区数）
	c = (i % (H*S)) / S;//磁头号=（块号%（磁头数*扇区数）)/扇区数
	s = (i % (H*S)) % S;//扇区号=（块号%（磁头数*扇区数）)%扇区数
	for (int a = 0; a<B;) {
		disk[i][a] = *tp;
		disk_model[z][c][s] = *tp;
		a++;
		tp++;
	}
}
//初始化系统数据
void Init() {
	int i;
	char tp[B];
	for (i = 0; i<L; i++) {//初始化磁盘
		Init_block(tp, B);
		write_block(i, tp);
	}
	for (i = K; i<L; i++) {//初始化位图
		read_block((i - K) / B, tp);
		tp[(i - K) % B] = FREE;
		write_block((i - K) % B, tp);
	}
	//初始化目录文件
	file_description tp_cnt_sign;
	tp_cnt_sign.file_description_flag = 1;
	tp_cnt_sign.file_length = 0;
	tp_cnt_sign.file_block = FILE_BLOCK_LENGTH;
	Init_block(tp, B);
	tp[0] = tp_cnt_sign.file_description_flag;
	tp[1] = tp_cnt_sign.file_length;
	tp[2] = tp_cnt_sign.file_block;
	for (i = 0; i<FILE_BLOCK_LENGTH; i++) {
		tp[i + 3] = K + i; //默认数据区的前FILE_BLOCK_LENGTH个被目录占用
	}
	write_block(FILE_SIGN_AREA, tp);
	read_block(0, tp);
	for (i = 0; i<FILE_NUM; i++)
		tp[i] = FREE;
	write_block(0, tp);
}

// 根据指定的文件名创建新文件
int create(char *filename) {
	int i, frees;
	int freed;
	int freed2;
	char tps[B], tpc[B], tp[B];
	for (i = K; i<K + FILE_NUM; i++) {//查看文件名是否存在
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] == BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				cout << "该目录已经存在文件名为" << filename << "的文件" << endl;
				return 0;
			}
		}
	}
	for (i = FILE_SIGN_AREA; i<K; i++) {//寻找空闲文件描述符
		read_block(i, tp);
		if (tp[0] == FREE) {
			frees = i;
			break;
		}
	}
	if (i == K) {
		cout << "没有空闲文件描述符" << endl;
		return 0;
	}
	for (i = K; i<K + FILE_NUM; i++) {//寻找数据区的目录描述符所指定的空闲存储块
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] == FREE) {
			freed = i;
			break;
		}
	}
	if (i == K + FILE_NUM) {
		cout << "文件个数已达上限" << endl;
		return 0;
	}
	for (i = K + FILE_NUM; i<L; i++) {//寻找文件区的空闲块分配给新创建的文件
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] == FREE) {
			freed2 = i;
			break;
		}
	}
	if (i == L) {
		cout << "磁盘已满,分配失败" << endl;
		return 0;
	}
	file_description tp_file_description;//创建临时文件描述符
	contents tp_contents;//创建临时目录项
	tp_file_description.file_description_flag = 1;//构建文件描述符
	tp_file_description.file_length = 0;
	tp_file_description.file_block = 1;
	Init_block(tps, B);
	tps[0] = tp_file_description.file_description_flag;
	tps[1] = tp_file_description.file_length;
	tps[2] = tp_file_description.file_block;
	tps[3] = freed2;
	for (i = 4; i<FILE_BLOCK_LENGTH; i++) {
		tps[i] = '\0';
	}
	write_block(frees, tps);//写入磁盘
							  //构建目录项插入目录文件描述符所指定的数组块
	tp_contents.file_descriptionnum = frees - FILE_SIGN_AREA;
	strncpy(tp_contents.filename, filename, FILE_NAME_LENGTH);
	Init_block(tpc, B);
	tpc[0] = tp_contents.file_descriptionnum;
	tpc[1] = '\0';
	strcat(tpc, tp_contents.filename);
	write_block(freed, tpc);
	//更改位图状态
	read_block((freed - K) / B, tp);
	tp[(freed - K) % B] = BUSY;
	write_block((freed - K) / B, tp);
	read_block((freed2 - K) / B, tp);
	tp[(freed2 - K) % B] = BUSY;
	write_block((freed2 - K) / B, tp);
	//增加目录文件描述符的长度
	read_block(FILE_SIGN_AREA, tp);
	tp[1]++;
	write_block(FILE_SIGN_AREA, tp);
	return 1;
}
//删除指定文件
int destroy(char *filename) {
	int i, dtys, dtyd, use_block;
	int index;
	char tp[B];
	char tpd[B];
	//寻找目录项和文件描述符
	for (i = K; i<K + FILE_NUM; i++) {
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] == BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				dtyd = i;
				dtys = tp[0] + FILE_SIGN_AREA;
				index = tp[0];
				break;
			}
		}
	}
	if (i == K + FILE_NUM) {
		cout << "没有找到该文件" << endl;
		return 0;
	}
	int list = -1;
	//查看文件是否打开
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (open_list[list].flag = BUSY&&list != -1) {
		cout << "该文件已经被打开,需要关闭才能删除" << endl;
		return 0;
	}
	read_block(dtys, tp);
	use_block = tp[2];
	for (i = 0; i<use_block; i++) {
		read_block((tp[i + 3] - K) / B, tpd);
		tpd[(tp[i + 3] - K) % B] = FREE;
		write_block((tp[i + 3] - K) / B, tpd);
	}
	//删除该目录项
	Init_block(tp, B);
	write_block(dtys, tp);
	//删除文件描述符
	Init_block(tp, B);
	write_block(dtyd, tp);
	//更改位图
	read_block((dtyd - K) / B, tp);
	tp[(dtyd - K) % B] = FREE;
	write_block((dtyd - K) / B, tp);
	//目录文件描述长度减1
	read_block(FILE_SIGN_AREA, tp);
	tp[1]--;
	write_block(FILE_SIGN_AREA, tp);
	return 1;
}
//打开文件
int open(char *filename) {
	int i, opd, ops;
	int list;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//寻找文件目录项和文件描述符
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] = BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				opd = i;
				ops = tp[0];//文件描述符
				break;
			}
		}
	}
	if (i == K + FILE_NUM) {
		cout << "没有找到该文件" << endl;
		return 0;
	}
	for (i = 0; i<FILE_NUM; i++) {//查看文件是否被打开
		if (open_list[i].file_descriptionnum == ops&&open_list[i].flag == BUSY) {
			cout << "该文件己经被打开" << endl;
			return 0;
		}
	}
	for (i = 0; i<FILE_NUM; i++) {//找一块未使用的区域分配
		if (open_list[i].flag != BUSY) {
			list = i;
			break;
		}
	}
	open_list[list].file_descriptionnum = ops;//写入文件描述符
	open_list[list].flag = BUSY;//置标志位为占用
	index = open_list[list].file_descriptionnum;//生成索引号即文件描述符
	lseek(index, 0);
	Init_block(open_list[list].buffer, BUFFER_LENGTH);
	read_block(open_list[list].pointer[0], tp);
	strncpy(open_list[list].buffer, tp, BUFFER_LENGTH);
	return 1;
}
//关闭指定文件
int close(char *filename) {
	int i, opd, ops;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//寻找文件目录项和文件描述符
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] = BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				opd = i;
				ops = tp[0];
				break;
			}
		}
	}
	index = ops;//找到要操作文件的文件描述符
	int list = -1;
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1 || open_list[list].flag != BUSY) {
		cout << "该文件没有被打开" << endl;
		return 0;
	}
	Init_block(open_list[list].buffer, BUFFER_LENGTH);
	open_list[list].file_descriptionnum = FREE;
	open_list[list].flag = FREE;
	open_list[list].pointer[0] = NULL;
	open_list[list].pointer[1] = NULL;
	return 1;
}
//从指定文件中从指定的内存位置(mem_area)顺序读入count个字节
int read(char *filename, int mem_area, int count) {
	int i, opd, ops;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//寻找文件目录项和文件描述符
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] = BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				opd = i;
				ops = tp[0];
				break;
			}
		}
	}
	index = ops;//找到要操作文件的文件描述符
	int list = -1;
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1 || open_list[list].flag != BUSY) {
		cout << "该文件没有被打开" << endl;
		return 0;
	}
	cout << "读取内容为：";
	for (i = 0; i<count; i++)
		cout << open_list[list].buffer[i];
	cout << endl;
	return 1;
}
//从指定的内存位置(mem_area)开始的count个字节顺序写入指定文件
int write(char *filename, int mem_area, int count) {
	int i, opd, ops;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//寻找文件目录项和文件描述符
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] = BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				opd = i;
				ops = tp[0];
				break;
			}
		}
	}
	index = ops;//找到要操作文件的文件描述符
	int list = -1;
	int input_length;
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1 || open_list[list].flag != BUSY) {
		cout << "该文件没有被打开" << endl;
		return 0;
	}
	char input[INPUT_LENGTH] = { 0 };
	Init_block(input, INPUT_LENGTH);
	cin >> input;
	for (i = 0; input[i] != 0; i++);
	input_length = i;
	if (count <= BUFFER_LENGTH) {
		strncat(open_list[list].buffer, input + mem_area, count);
		open_list[list].length = count + open_list[list].length;
		file_list[index].file_length = open_list[list].length;
	}
	return 1;
}
//把文件的读写指针移动到pos指定的位置
int lseek(int index, int pos) {
	int i;
	int list = -1;
	char tp[B];
	int pos_i = pos / B;//计算在所有文件中的第几块
	int pos_j = pos%B;//在第几块中的第几个位置
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1) {
		cout << "没找到当前文件,操作失败" << endl;
		return 0;
	}
	if (open_list[list].flag != BUSY) {
		cout << "输入索引号有误，操作失败" << endl;
		return 0;
	}
	read_block(open_list[list].file_descriptionnum + FILE_SIGN_AREA, tp);
	if (pos_i>tp[2] - 1) {
		cout << "异常越界，定位失败" << endl;
		return 0;
	}
	open_list[list].pointer[0] = tp[3 + pos_i];
	open_list[list].pointer[1] = pos_j;
	return 1;
}
//列表显示所有文件及其长度
void directory() {
	int i;
	int filenum;
	int filelength;
	char filename[FILE_NAME_LENGTH];
	char tp[B];
	char tpd[B];
	char tps[B];
	read_block(FILE_SIGN_AREA, tp);
	filenum = tp[1];
	if (filenum == 0) {
		cout << "该目录没有文件" << endl;
	}
	printf("文件名\t\t\t大小\n");
	for (i = 1; i<FILE_NUM; i++) {
		read_block(tp[2 + i], tpd);
		if (tpd[0] != 0) {
			read_block(tpd[0] + FILE_SIGN_AREA, tps);
			if (tps[0] == BUSY&&tpd[0] != 0) {
				filelength = file_list[i].file_length;
				strcpy(filename, tpd + 1);
				printf("%-10s\t\t%-2d字节\n", filename, filelength);
			}
		}
	}
	if (filenum != 0)
		cout << "共" << filenum << "个文件" << endl;
}
//显示已打开文件
int show_openlist() {
	int i, j;
	int openfile = 0;
	char tp[B];
	int index;
	//cout<<"文件名    大小"<<endl;
	printf("文件名\t\t\t大小\n");
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].flag == BUSY) {
			index = open_list[i].file_descriptionnum;
			openfile++;
			read_block(FILE_SIGN_AREA + index, tp);
			for (j = K; j<K + FILE_NUM; j++) {
				read_block(j, tp);
				if (tp[0] == index) {
					cout << tp + 1;
				}
			}
			printf("\t\t\t%d\n", open_list[i].length);
		}
	}
	return openfile;
}

//显示帮助
void showHelp()
{
	cout << "*********指令输入*********" << endl;
	cout << "请选择以下命令输入" << endl;
	cout << "1.menu" << endl;
	cout << "2.create file" << endl;
	cout << "3.delete file" << endl;
	cout << "4.open file" << endl;
	cout << "5.close file" << endl;
	cout << "6.read file" << endl;
	cout << "7.write file" << endl;
	cout << "8.help" << endl;
	cout << "9.exit" << endl;
	cout << endl;
}

int main()
{
	showHelp();//显示帮助
	Init();
	//系统在打开一个文件的时候，会先默认打开三个文件标准输入，标准输出，标准错误，它们三个分别占据了这个文件描述符数组的前三个
	create("stdin");
	create("stdout");
	create("stderror");
	char ch[CH_LENGTH];
	Init_block(ch, CH_LENGTH);
	cout << "请输入指令" << endl;
	while (cin >> ch) {
		int cmd;
		char filename[FILE_NAME_LENGTH];
		cmd = -1;
		Init_block(filename, FILE_NAME_LENGTH);
		if (strncmp("menu", ch, 4) == 0) {
			cmd = MENU;
		}
		if (strncmp("create", ch, 6) == 0) {
			cmd = CREATE;
			strcat(filename, ch + 7);
		}
		if (strncmp("delete", ch, 6) == 0) {
			cmd = DELETE;
			strcat(filename, ch + 7);
		}
		if (strncmp("open", ch, 4) == 0) {
			cmd = OPEN;
			strcat(filename, ch + 5);
		}
		if (strncmp("close", ch, 5) == 0) {
			cmd = CLOSE;
			strcat(filename, ch + 6);
		}
		if (strncmp("read", ch, 4) == 0) {
			cmd = READ;
			strcat(filename, ch + 5);
		}
		if (strncmp("write", ch, 5) == 0) {
			cmd = WRITE;
			strcat(filename, ch + 6);
		}
		if (strncmp("help", ch, 4) == 0) {
			cmd = HELP;
		}
		if (strncmp("exit", ch, 4) == 0) {
			cmd = EXIT;
			break;
		}
		//int index;
		int count, pos;
		switch (cmd) {
		case MENU:
			directory();
			cout << endl;
			break;
		case CREATE:
			if (create(filename) == 1) {
				cout << "创建文件成功" << endl;
				directory();
			}
			cout << endl;
			break;
		case DELETE:
			if (destroy(filename) == 1) {
				cout << "删除文件成功" << endl;
				directory();
			}
			cout << endl;
			break;
		case OPEN:
			if (open(filename) == 1) {
				cout << "打开文件成功" << endl;
				show_openlist();
			}
			cout << endl;
			break;
		case CLOSE:
			if (close(filename) == 1) {
				cout << "关闭文件成功" << endl;
				show_openlist();
			}
			cout << endl;
			break;
		case READ:
			cout << "请输入读取文件的开始位置" << endl;
			cin >> pos;
			cout << "请输入读取文件长度" << endl;
			cin >> count;
			if (read(filename, pos, count) == 1)
				cout << "读取文件成功" << endl;
			cout << endl;
			break;
		case WRITE:
			cout << "请输入写入内存的开始位置,首次写入时为0" << endl;
			cin >> pos;
			cout << "请输入写入文件长度" << endl;
			cin >> count;
			if (write(filename, pos, count) == 1) {
				cout << "写入文件成功" << endl;
				show_openlist();
			}
			cout << endl;
			break;
		case HELP:
			showHelp();
			break;
		default:
			cout << "指令错误" << endl;
			cout << endl;
			break;
		}
		Init_block(ch, CH_LENGTH);
		cout << "请输入指令" << endl;
	}
	return 0;
}