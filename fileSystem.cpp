#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
using namespace std;
#define B 10
#define C 10//������
#define H 8//��ͷ��
#define S 8//������
//���̵Ĵ洢������  ���=�����*����ͷ��*��������+��ͷ��*������+������
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
//�ļ�������
struct file_description {
	int file_length;//�ļ�����
	int file_description_flag;//ռ�ñ�ʶλ
	int file_block;//�ļ�����Ĵ��̿��
	int file_block_ary[FILE_BLOCK_LENGTH];//�ļ�����Ĵ��̿������
};
//Ŀ¼
struct contents {
	char filename[FILE_NAME_LENGTH];//�ļ���
	int file_length;//�ļ���С
	int file_descriptionnum;//�ļ����������
};
//�Ѵ򿪵��ļ�
struct openfile_list {
	char buffer[BUFFER_LENGTH];//��д������,���д����
	int pointer[2];//��дָ��
	int file_descriptionnum;//�ļ�������
	int flag;//ռ�÷�
	int length;//ռ�û�������С
};


char disk_model[C][H][S];//����ģ�ͣ�����CHS �ֱ��ʾ����ţ���ͷ�ź�������
//z=BlockNum/(H*S);//�����=���/����ͷ��*��������
//c=(BlockNum%(H*S))/S;//��ͷ��=�����%����ͷ��*��������)/������
//s=(BlockNum%(H*S))%S;//������=�����%����ͷ��*��������)%������
char disk[L][B];
openfile_list open_list[FILE_NUM];//�Ѵ��ļ�����
contents file_list[FILE_NUM];//�����ļ�����

int lseek(int index, int pos);//���ļ��Ķ�дָ���ƶ���posָ����λ��
//��ʼ���ַ�����
void Init_block(char *tp, int length) {
	int i;
	for (i = 0; i<length; i++)
		tp[i] = '\0';
}
//�ú������߼���i�����ݶ��뵽ָ��pָ����ڴ�λ�ã��������ַ�����Ϊ�洢��ĳ���B
void read_block(int i, char *p) {
	char *tp = (char *)malloc(sizeof(char));
	tp = p;
	int z, c, s;
	z = i / (H*S);//�����=���/����ͷ��*��������
	c = (i % (H*S)) / S;//��ͷ��=�����%����ͷ��*��������)/������
	s = (i % (H*S)) % S;//������=�����%����ͷ��*��������)%������
	for (int a = 0; a<B;) {
		*tp = disk_model[z][c][s];
		*tp = disk[i][a];
		a++;
		tp++;
	}
}
//�ú�����ָ��pָ�������д���߼���i���������ַ�����Ϊ�洢��ĳ���B
void write_block(int i, char *p) {
	char *tp = (char *)malloc(sizeof(char));
	tp = p;
	int z, c, s;
	z = i / (H*S);//�����=���/����ͷ��*��������
	c = (i % (H*S)) / S;//��ͷ��=�����%����ͷ��*��������)/������
	s = (i % (H*S)) % S;//������=�����%����ͷ��*��������)%������
	for (int a = 0; a<B;) {
		disk[i][a] = *tp;
		disk_model[z][c][s] = *tp;
		a++;
		tp++;
	}
}
//��ʼ��ϵͳ����
void Init() {
	int i;
	char tp[B];
	for (i = 0; i<L; i++) {//��ʼ������
		Init_block(tp, B);
		write_block(i, tp);
	}
	for (i = K; i<L; i++) {//��ʼ��λͼ
		read_block((i - K) / B, tp);
		tp[(i - K) % B] = FREE;
		write_block((i - K) % B, tp);
	}
	//��ʼ��Ŀ¼�ļ�
	file_description tp_cnt_sign;
	tp_cnt_sign.file_description_flag = 1;
	tp_cnt_sign.file_length = 0;
	tp_cnt_sign.file_block = FILE_BLOCK_LENGTH;
	Init_block(tp, B);
	tp[0] = tp_cnt_sign.file_description_flag;
	tp[1] = tp_cnt_sign.file_length;
	tp[2] = tp_cnt_sign.file_block;
	for (i = 0; i<FILE_BLOCK_LENGTH; i++) {
		tp[i + 3] = K + i; //Ĭ����������ǰFILE_BLOCK_LENGTH����Ŀ¼ռ��
	}
	write_block(FILE_SIGN_AREA, tp);
	read_block(0, tp);
	for (i = 0; i<FILE_NUM; i++)
		tp[i] = FREE;
	write_block(0, tp);
}

// ����ָ�����ļ����������ļ�
int create(char *filename) {
	int i, frees;
	int freed;
	int freed2;
	char tps[B], tpc[B], tp[B];
	for (i = K; i<K + FILE_NUM; i++) {//�鿴�ļ����Ƿ����
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] == BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				cout << "��Ŀ¼�Ѿ������ļ���Ϊ" << filename << "���ļ�" << endl;
				return 0;
			}
		}
	}
	for (i = FILE_SIGN_AREA; i<K; i++) {//Ѱ�ҿ����ļ�������
		read_block(i, tp);
		if (tp[0] == FREE) {
			frees = i;
			break;
		}
	}
	if (i == K) {
		cout << "û�п����ļ�������" << endl;
		return 0;
	}
	for (i = K; i<K + FILE_NUM; i++) {//Ѱ����������Ŀ¼��������ָ���Ŀ��д洢��
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] == FREE) {
			freed = i;
			break;
		}
	}
	if (i == K + FILE_NUM) {
		cout << "�ļ������Ѵ�����" << endl;
		return 0;
	}
	for (i = K + FILE_NUM; i<L; i++) {//Ѱ���ļ����Ŀ��п������´������ļ�
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] == FREE) {
			freed2 = i;
			break;
		}
	}
	if (i == L) {
		cout << "��������,����ʧ��" << endl;
		return 0;
	}
	file_description tp_file_description;//������ʱ�ļ�������
	contents tp_contents;//������ʱĿ¼��
	tp_file_description.file_description_flag = 1;//�����ļ�������
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
	write_block(frees, tps);//д�����
							  //����Ŀ¼�����Ŀ¼�ļ���������ָ���������
	tp_contents.file_descriptionnum = frees - FILE_SIGN_AREA;
	strncpy(tp_contents.filename, filename, FILE_NAME_LENGTH);
	Init_block(tpc, B);
	tpc[0] = tp_contents.file_descriptionnum;
	tpc[1] = '\0';
	strcat(tpc, tp_contents.filename);
	write_block(freed, tpc);
	//����λͼ״̬
	read_block((freed - K) / B, tp);
	tp[(freed - K) % B] = BUSY;
	write_block((freed - K) / B, tp);
	read_block((freed2 - K) / B, tp);
	tp[(freed2 - K) % B] = BUSY;
	write_block((freed2 - K) / B, tp);
	//����Ŀ¼�ļ��������ĳ���
	read_block(FILE_SIGN_AREA, tp);
	tp[1]++;
	write_block(FILE_SIGN_AREA, tp);
	return 1;
}
//ɾ��ָ���ļ�
int destroy(char *filename) {
	int i, dtys, dtyd, use_block;
	int index;
	char tp[B];
	char tpd[B];
	//Ѱ��Ŀ¼����ļ�������
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
		cout << "û���ҵ����ļ�" << endl;
		return 0;
	}
	int list = -1;
	//�鿴�ļ��Ƿ��
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (open_list[list].flag = BUSY&&list != -1) {
		cout << "���ļ��Ѿ�����,��Ҫ�رղ���ɾ��" << endl;
		return 0;
	}
	read_block(dtys, tp);
	use_block = tp[2];
	for (i = 0; i<use_block; i++) {
		read_block((tp[i + 3] - K) / B, tpd);
		tpd[(tp[i + 3] - K) % B] = FREE;
		write_block((tp[i + 3] - K) / B, tpd);
	}
	//ɾ����Ŀ¼��
	Init_block(tp, B);
	write_block(dtys, tp);
	//ɾ���ļ�������
	Init_block(tp, B);
	write_block(dtyd, tp);
	//����λͼ
	read_block((dtyd - K) / B, tp);
	tp[(dtyd - K) % B] = FREE;
	write_block((dtyd - K) / B, tp);
	//Ŀ¼�ļ��������ȼ�1
	read_block(FILE_SIGN_AREA, tp);
	tp[1]--;
	write_block(FILE_SIGN_AREA, tp);
	return 1;
}
//���ļ�
int open(char *filename) {
	int i, opd, ops;
	int list;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//Ѱ���ļ�Ŀ¼����ļ�������
		read_block((i - K) / B, tp);
		if (tp[(i - K) % B] = BUSY) {
			read_block(i, tp);
			if (strncmp(tp + 1, filename, FILE_NAME_LENGTH) == 0) {
				opd = i;
				ops = tp[0];//�ļ�������
				break;
			}
		}
	}
	if (i == K + FILE_NUM) {
		cout << "û���ҵ����ļ�" << endl;
		return 0;
	}
	for (i = 0; i<FILE_NUM; i++) {//�鿴�ļ��Ƿ񱻴�
		if (open_list[i].file_descriptionnum == ops&&open_list[i].flag == BUSY) {
			cout << "���ļ���������" << endl;
			return 0;
		}
	}
	for (i = 0; i<FILE_NUM; i++) {//��һ��δʹ�õ��������
		if (open_list[i].flag != BUSY) {
			list = i;
			break;
		}
	}
	open_list[list].file_descriptionnum = ops;//д���ļ�������
	open_list[list].flag = BUSY;//�ñ�־λΪռ��
	index = open_list[list].file_descriptionnum;//���������ż��ļ�������
	lseek(index, 0);
	Init_block(open_list[list].buffer, BUFFER_LENGTH);
	read_block(open_list[list].pointer[0], tp);
	strncpy(open_list[list].buffer, tp, BUFFER_LENGTH);
	return 1;
}
//�ر�ָ���ļ�
int close(char *filename) {
	int i, opd, ops;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//Ѱ���ļ�Ŀ¼����ļ�������
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
	index = ops;//�ҵ�Ҫ�����ļ����ļ�������
	int list = -1;
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1 || open_list[list].flag != BUSY) {
		cout << "���ļ�û�б���" << endl;
		return 0;
	}
	Init_block(open_list[list].buffer, BUFFER_LENGTH);
	open_list[list].file_descriptionnum = FREE;
	open_list[list].flag = FREE;
	open_list[list].pointer[0] = NULL;
	open_list[list].pointer[1] = NULL;
	return 1;
}
//��ָ���ļ��д�ָ�����ڴ�λ��(mem_area)˳�����count���ֽ�
int read(char *filename, int mem_area, int count) {
	int i, opd, ops;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//Ѱ���ļ�Ŀ¼����ļ�������
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
	index = ops;//�ҵ�Ҫ�����ļ����ļ�������
	int list = -1;
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1 || open_list[list].flag != BUSY) {
		cout << "���ļ�û�б���" << endl;
		return 0;
	}
	cout << "��ȡ����Ϊ��";
	for (i = 0; i<count; i++)
		cout << open_list[list].buffer[i];
	cout << endl;
	return 1;
}
//��ָ�����ڴ�λ��(mem_area)��ʼ��count���ֽ�˳��д��ָ���ļ�
int write(char *filename, int mem_area, int count) {
	int i, opd, ops;
	char tp[B];
	int index;
	for (i = K; i<K + FILE_NUM; i++) {//Ѱ���ļ�Ŀ¼����ļ�������
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
	index = ops;//�ҵ�Ҫ�����ļ����ļ�������
	int list = -1;
	int input_length;
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1 || open_list[list].flag != BUSY) {
		cout << "���ļ�û�б���" << endl;
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
//���ļ��Ķ�дָ���ƶ���posָ����λ��
int lseek(int index, int pos) {
	int i;
	int list = -1;
	char tp[B];
	int pos_i = pos / B;//�����������ļ��еĵڼ���
	int pos_j = pos%B;//�ڵڼ����еĵڼ���λ��
	for (i = 0; i<FILE_NUM; i++) {
		if (open_list[i].file_descriptionnum == index) {
			list = i;
			break;
		}
	}
	if (list == -1) {
		cout << "û�ҵ���ǰ�ļ�,����ʧ��" << endl;
		return 0;
	}
	if (open_list[list].flag != BUSY) {
		cout << "�������������󣬲���ʧ��" << endl;
		return 0;
	}
	read_block(open_list[list].file_descriptionnum + FILE_SIGN_AREA, tp);
	if (pos_i>tp[2] - 1) {
		cout << "�쳣Խ�磬��λʧ��" << endl;
		return 0;
	}
	open_list[list].pointer[0] = tp[3 + pos_i];
	open_list[list].pointer[1] = pos_j;
	return 1;
}
//�б���ʾ�����ļ����䳤��
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
		cout << "��Ŀ¼û���ļ�" << endl;
	}
	printf("�ļ���\t\t\t��С\n");
	for (i = 1; i<FILE_NUM; i++) {
		read_block(tp[2 + i], tpd);
		if (tpd[0] != 0) {
			read_block(tpd[0] + FILE_SIGN_AREA, tps);
			if (tps[0] == BUSY&&tpd[0] != 0) {
				filelength = file_list[i].file_length;
				strcpy(filename, tpd + 1);
				printf("%-10s\t\t%-2d�ֽ�\n", filename, filelength);
			}
		}
	}
	if (filenum != 0)
		cout << "��" << filenum << "���ļ�" << endl;
}
//��ʾ�Ѵ��ļ�
int show_openlist() {
	int i, j;
	int openfile = 0;
	char tp[B];
	int index;
	//cout<<"�ļ���    ��С"<<endl;
	printf("�ļ���\t\t\t��С\n");
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

//��ʾ����
void showHelp()
{
	cout << "*********ָ������*********" << endl;
	cout << "��ѡ��������������" << endl;
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
	showHelp();//��ʾ����
	Init();
	//ϵͳ�ڴ�һ���ļ���ʱ�򣬻���Ĭ�ϴ������ļ���׼���룬��׼�������׼�������������ֱ�ռ��������ļ������������ǰ����
	create("stdin");
	create("stdout");
	create("stderror");
	char ch[CH_LENGTH];
	Init_block(ch, CH_LENGTH);
	cout << "������ָ��" << endl;
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
				cout << "�����ļ��ɹ�" << endl;
				directory();
			}
			cout << endl;
			break;
		case DELETE:
			if (destroy(filename) == 1) {
				cout << "ɾ���ļ��ɹ�" << endl;
				directory();
			}
			cout << endl;
			break;
		case OPEN:
			if (open(filename) == 1) {
				cout << "���ļ��ɹ�" << endl;
				show_openlist();
			}
			cout << endl;
			break;
		case CLOSE:
			if (close(filename) == 1) {
				cout << "�ر��ļ��ɹ�" << endl;
				show_openlist();
			}
			cout << endl;
			break;
		case READ:
			cout << "�������ȡ�ļ��Ŀ�ʼλ��" << endl;
			cin >> pos;
			cout << "�������ȡ�ļ�����" << endl;
			cin >> count;
			if (read(filename, pos, count) == 1)
				cout << "��ȡ�ļ��ɹ�" << endl;
			cout << endl;
			break;
		case WRITE:
			cout << "������д���ڴ�Ŀ�ʼλ��,�״�д��ʱΪ0" << endl;
			cin >> pos;
			cout << "������д���ļ�����" << endl;
			cin >> count;
			if (write(filename, pos, count) == 1) {
				cout << "д���ļ��ɹ�" << endl;
				show_openlist();
			}
			cout << endl;
			break;
		case HELP:
			showHelp();
			break;
		default:
			cout << "ָ�����" << endl;
			cout << endl;
			break;
		}
		Init_block(ch, CH_LENGTH);
		cout << "������ָ��" << endl;
	}
	return 0;
}