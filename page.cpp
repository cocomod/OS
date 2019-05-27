#include<iostream>
#include<cstdlib>
#include<ctime>
#include<algorithm>
#include <iomanip>
using namespace std;
//-----------------���ݶ���------------
#define MAX 1000000
int pageNum;   //ҳ�����
int missCount;   //ȱҳ����
double missRag;   //ȱҳ��
int memNum;    //�ڴ��������
int memcount;   //
int ss;        //��������ʼҳ��
int NUM;       //�߼���ַҳ������
int speed;     //�������ƶ�����
double t = 0.5;
double r;
int N;            //�����ڴ�ߴ�
int list[MAX];    //ҳ������������
int wrList[MAX];  //ҳ���д���ʷ�ʽ
int mem[MAX];      //�ڴ��ʼ��Ϊ-1
int id;          //��ǰҳ�����λ��
int states[MAX];  //�ڴ�ҳ��״̬����
clock_t t1, t2;     //��ʼʱ���Լ�����ʱ��
double a1, a2, a3, a4, a5, b1, b2, b3, b4, b5;   //���ڶ���ҳ���û��㷨�Ƚ�
//������������ҳ��������������������������������������������������������
#define randomNum(a,b) rand() % (b - a + 1) + a
#define random(x) rand()%x
void listPrint();
void randOccur() {
	cout << "���������ɵ�ҳ��������еĸ���:";//10
	cin >> NUM;
	cout << "�����������ڴ�ĳߴ�:";//ҳ�����64
	cin >> N;
	cout << "��������ʼҳ�ţ�";//1
	cin >> ss;
	cout << "�����ù�����ҳ��������";//6
	cin >> pageNum;
	cout << "�����ù������ƶ����ʣ�";//4
	cin >> speed;
	cout << "������ʵ���ڴ�����";//�������32
	cin >> memNum;
	cout << "---------------------------------" << endl;
	srand((int)time(0));
	int j = 0;
	for (int i = 0; i<NUM; i++) {
		if (j < speed) {
			int x = ss + pageNum, y = N - 1;
			list[i] = randomNum(ss, min(x, y));
			double sn = random(10)*0.1;
			if (sn > 0.7) {
				wrList[i] = 1;//��д��ʽ����
			}
			else {
				wrList[i] = 0;//�Զ���ʽ����
			}
			j++;
		}
		else {
			r = random(10)*0.1;
			j = 0;
			if (r < t) {
				ss = random(N);
			}
			else {
				ss = (ss + 1) % N;
			}
		}
	}
}
//�ڴ������ʼ��
void pre() {
	//------------��ʼ��------------
	for (int i = 0; i < memNum; i++) {
		mem[i] = -1;
	}
	memcount = 0;
}
void start() {
	cout << "******************ҳ���û��㷨*****************" << endl;
	cout << "1.����û��㷨             2.�Ƚ��ȳ��û��㷨  " << endl;
	cout << "3.������δʹ���û��㷨       4.ҳ�滺���㷨PBA" << endl;
	cout << "5.�Ľ��� Clock ��̭�㷨    0.�˳� " << endl;
	cout << "***********************************************" << endl;
}
//����������
void listPrint() {
	cout << "--------------�����������---------------" << endl;
	for (int i = 0; i < NUM; i++) {
		cout << list[i] << "  ";
	}
	cout << endl;
	cout << "----------------------------------------" << endl;
}
//�����Ƿ����ڴ���
int judge(int n) {
	int a;
	for (int i = 0; i < memNum; i++) {
		if (mem[i] == list[n]) {
			a = 1;
			if (n >= memNum)
				id = i;
			states[i] = 1;
			break;
		}
		if (i == memNum - 1 && mem[i] != list[n])
			a = 0;
	}
	return a;
}
//����list����󱻷��ʵ�Ԫ��
void search(int wei) {
	int cur = 0;
	int last[MAX];
	int min = 0;
	for (int i = 0; i < memNum; i++) {
		last[i] = MAX;
	}
	for (int j = 0; j < memNum; j++) {
		for (int i = wei + 1; i < NUM; i++) {
			if (mem[j] == list[i]) {
				last[j] = i;
				break;
			}
		}
	}
	for (int i = 0; i < memNum; i++) {
		if (last[i] > min) {
			cur = i;
			min = last[i];
		}
	}
	mem[cur] = list[wei];
}
void memPrint() {
	cout << "<";
	for (int i = 0; i < memNum; i++) {
		if (i == memNum - 1) {
			cout << mem[i];
			break;
		}
		cout << mem[i] << ",";
	}
	cout << ">" << endl;
}
//�����̭�㷨
void OPT() {
	pre();
	t1 = clock();
	missCount = 0;
	int flag = -1;
	for (int i = 0; i <NUM; i++) {
		if (memcount < 3) {
			mem[memcount] = list[i];
			memcount++;
		}
		else {
			flag = judge(i);
			if (flag == 0) {
				if (memcount < memNum) {
					mem[memcount] = list[i];
					memcount++;
				}
				else {
					search(i);
					missCount++;
					cout << "����ȱҳ�ж�" << endl;
				}
			}
		}
		memPrint();
	}
	t2 = clock();
	missRag = missCount * 1.0 / NUM;
	cout << "ȱҳ��:" << missRag << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
	a1 = missRag;
	b1 = double(t2 - t1) / CLOCKS_PER_SEC;
}
//Ԥ�ȴ���FIFO��������
int first[MAX];
void prd() {
	for (int i = 0; i < memNum; i++) {
		first[i] = 0;
	}
}
void add(int n) {
	for (int i = 0; i <= n; i++) {
		first[i]++;
	}
}
//�Ƚ��ȳ���̭�㷨
void FIFO() {
	pre();
	prd();
	t1 = clock();
	missCount = 0;
	int flag = -1;
	for (int i = 0; i < NUM; i++) {
		if (memcount < 3) {
			mem[memcount] = list[i];
			add(memcount);
			memcount++;
		}
		else {
			flag = judge(i);
			if (flag == 0) {
				if (memcount < memNum) {
					mem[memcount] = list[i];
					add(memcount);
					memcount++;
				}
				else {
					int min = 0, cur = 0;
					for (int k = 0; k < memNum; k++) {
						if (first[k] > min) {
							cur = k;
							min = first[k];
						}
					}
					first[cur] = 0;
					add(memNum);
					mem[cur] = list[i];
					missCount++;
					cout << "����ȱҳ�ж�" << endl;
				}
			}
		}
		memPrint();
	}
	t2 = clock();
	missRag = missCount * 1.0 / NUM;
	cout << "ȱҳ��:" << missRag << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
	a2 = missRag;
	b2 = double(t2 - t1) / CLOCKS_PER_SEC;
}
//�������û�б����ʵ�Ԫ��
void searchL(int wei) {
	int cur = 0;
	int endf[MAX];
	int min = MAX;
	for (int i = 0; i < memNum; i++) {
		endf[i] = -1;
	}
	for (int j = 0; j < memNum; j++) {
		for (int i = wei + 1; i > -1; i--) {
			if (mem[j] == list[i]) {
				endf[j] = i;
				break;
			}
		}
	}
	for (int i = 0; i < memNum; i++) {
		if (endf[i] < min) {
			cur = i;
			min = endf[i];
		}
	}
	mem[cur] = list[wei];
}
//���δʹ��
void LRU() {
	pre();
	t1 = clock();
	missCount = 0;
	int flag = -1;
	for (int i = 0; i < NUM; i++) {
		if (memcount < 3) {
			mem[memcount] = list[i];
			memcount++;
		}
		else {
			flag = judge(i);
			if (flag == 0) {
				if (memcount < memNum) {
					mem[memcount] = list[i];
					memcount++;
				}
				else {
					searchL(i);
					missCount++;
					cout << "����ȱҳ�ж�" << endl;
				}
			}
		}
		memPrint();
	}
	t2 = clock();
	missRag = missCount * 1.0 / NUM;
	cout << "ȱҳ��:" << missRag << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
	a3 = missRag;
	b3 = double(t2 - t1) / CLOCKS_PER_SEC;
}
//ѭ��ɨ��
void Cloop(int n, int wei) {
	int temp;
	for (int i = n + 1;; i++) {
		temp = i % memNum;
		if (states[temp] == 0) {
			mem[temp] = list[wei];
			states[temp] = 1;
			id = temp;
			break;
		}
		else {
			states[temp] = 0;
		}
	}
}
//---����λ�������
void weiPrint() {
	cout << "(";
	for (int i = 0; i < memNum; i++) {
		if (i == memNum - 1) {
			cout << states[i];
			break;
		}
		cout << states[i] << ",";
	}
	cout << ")" << endl;
}
//ʱ��
void CLOCK() {
	pre();
	id = -1;
	//��ʼ��������ҳ�ķ���λ��0��
	for (int i = 0; i < memNum; i++) {
		states[i] = 0;
	}
	t1 = clock();
	missCount = 0;
	int flag = -1;
	for (int i = 0; i < NUM; i++) {
		if (memcount < 3) {
			mem[memcount] = list[i];
			states[memcount] = 1;
			memcount++;
		}
		else {
			flag = judge(i);
			if (flag == 0) {
				if (memcount < memNum) {
					mem[memcount] = list[i];
					states[memcount] = 1;
					memcount++;
				}
				else {
					Cloop(id, i);
					missCount++;
					cout << "����ȱҳ�ж�" << endl;
				}
			}
		}
		memPrint();
		weiPrint();
	}
	t2 = clock();
	missRag = missCount * 1.0 / NUM;
	cout << "ȱҳ��:" << missRag << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
	a4 = missRag;
	b4 = double(t2 - t1) / CLOCKS_PER_SEC;
}
//���ҸĽ�ʱ���㷨
void Gloop(int n, int wei) {
	int temp;
	for (int i = n + 1;; i++) {
		temp = i % memNum;
		if (states[temp] == 0 && wrList[temp] == 0) {
			mem[temp] = list[wei];
			states[temp] = 1;
			id = temp;
			break;
		}
		if (i>n + memNum - 1) {
			if (states[temp] == 0 && wrList[temp] == 1) {
				mem[temp] = list[wei];
				states[temp] = 1;
				id = temp;
				break;
			}
			states[temp] = 0;
		}
		if (i == n + 2 * memNum - 1)
			i = n;
	}
}
//---�޸�λ�������
void rPrint() {
	cout << "��";
	for (int i = 0; i < memNum; i++) {
		if (i == memNum - 1) {
			cout << wrList[i];
			break;
		}
		cout << wrList[i] << ",";
	}
	cout << "��" << endl;
}
//�Ľ�ʱ��
void GCLOCK() {
	pre();
	id = -1;
	//��ʼ��������ҳ�ķ���λ��0��
	for (int i = 0; i < memNum; i++) {
		states[i] = 0;
	}
	t1 = clock();
	missCount = 0;
	int flag = -1;
	for (int i = 0; i < NUM; i++) {
		if (memcount < 3) {
			mem[memcount] = list[i];
			states[memcount] = 1;
			memcount++;
		}
		else {
			flag = judge(i);
			if (flag == 0) {
				if (memcount < memNum) {
					mem[memcount] = list[i];
					states[memcount] = 1;
					memcount++;
				}
				else {
					Gloop(id, i);
					missCount++;
					cout << "����ȱҳ�ж�" << endl;
				}
			}
		}
		memPrint();
		weiPrint();
		rPrint();
	}
	t2 = clock();
	missRag = missCount * 1.0 / NUM;
	cout << "ȱҳ��:" << missRag << "\t" << "ʱ�俪��:" << double(t2 - t1) / CLOCKS_PER_SEC << endl;
	a5 = missRag;
	b5 = double(t2 - t1) / CLOCKS_PER_SEC;
}
void compare() {
	double a = a1 * 100;
	cout << setiosflags(ios::fixed);
	cout << "����    " << "\t" << "OPT" << "\t" << "FIFO" << "\t" << "LRU" << "\t" << "CLOCK" << "\t" << "Gclock" << endl;
	cout << "ȱҳ��  " << "\t" << setprecision(2) << a << "%\t" << a2 * 100 << "%\t" << a3 * 100 << "%\t" << a4 * 100 << "%\t" << a5 * 100 << "%" << endl;
	cout << "ʱ�俪��" << "\t" << setprecision(4) << b1 << "\t" << b2 << "\t" << b3 << "\t" << b4 << "\t" << b5 << endl;

}
int main() {
	int select;
	cout << "---------------ҳ���û���ʼ������-------------" << endl;
	randOccur();
	while (1) {
		start();
		cout << "��������Ҫ���е�ҳ����̭�㷨��";
		cin >> select;
		if (select == 1) {
			listPrint();
			OPT();
			continue;
		}
		if (select == 2) {
			listPrint();
			FIFO();
			continue;
		}
		if (select == 3) {
			listPrint();
			LRU();
			continue;
		}
		if (select == 4) {
			listPrint();
			CLOCK();
			continue;
		}
		if (select == 5) {
			listPrint();
			GCLOCK();
			continue;
		}
		if (select == 0) {
			break;
		}
	}
	system("pause");
	return 0;
}