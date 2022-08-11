#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/time.h>
#include <string.h>

int term = 1; // time slice �� ����
int count = 0; // ���� time slice ���� Ƚ��
int ts; // ������ time slice �� Ƚ��
int n_queue = 3; // ť�� ���� (�켱 ���� 3��)
int game_tol = 2; // priority �� �����ϴ� ���� �ð�
int pu_term = 10; // priority �� �ʱ�ȭ�ϴ� ����

struct NODE {
	pid_t data;
	struct NODE* next;
	int time;
}; // Linked list

struct NODE* head[3]; // Queue 3���� �迭�� ����

void sig_handler(int signal){
	int i = 0;
	int j = 0;
	int l = 0;
	int k = 0;
	int m = 0;// ijlkm�� iterator
	
	if (++count < ts){

		if (count % pu_term == 0) { // 10�ʰ� ����Ǿ��� ���
			struct NODE* tempno;
			struct NODE* heado;

			for (m = 0; m < n_queue; m++) {
				if (head[m]->next == NULL)
					continue;
				else {
					heado = head[m];

					if (heado->next->time != m * 2 && heado->next->time % 2 == 0) {
						heado->next;
						heado = heado->next;
					}

					while (heado->next != NULL) {
						heado->next->time %= 2;
						heado = heado->next;
					}
					m++;
					for (; m < n_queue; m++) {
						heado = head[m];
						while (heado->next != NULL) {
							heado->next->time = 0;
							heado = heado->next;
						}
					}
				}
			}

			for (k = 1; k < n_queue; k++) {
				tempno = head[0];
				while (tempno->next != NULL) {
					tempno = tempno->next;
				}
				tempno->next = head[k]->next;
				head[k]->next = NULL;
			} // head[0]�� ��� Node���� �����ϰ� head[1], head[2] ���� ������ ����.
		}

		for (i = 0; i < n_queue; i++) {
			if (head[i]->next == NULL)
				continue;
			else { // ���� �������� ���μ����� ã�� Stop.
				kill(head[i]->next->data, SIGSTOP);	
				
				struct NODE* temp;
				struct NODE* body = (struct NODE*)malloc(sizeof(struct NODE));

				body->data = head[i]->next->data;
				body->next = NULL;
				body->time = head[i]->next->time + term;

				if (body->time < game_tol * (i + 1)) {
					temp = head[i];
					while (temp->next != NULL) {
						temp = temp->next;
					}
					temp->next = body;
					head[i]->next = head[i]->next->next;
					break;
				}

				else if (body->time >= game_tol * (i + 1) && i != n_queue - 1) {
					temp = head[i + 1];
					while (temp->next != NULL) {
						temp = temp->next;
					}
					temp->next = body;
					head[i]->next = head[i]->next->next;
					break;
				}
				else {
					temp = head[i];
					while (temp->next != NULL) {
						temp = temp->next;
					}
					temp->next = body;
					head[i]->next = head[i]->next->next;
					break;
				} // Stop ��Ų �� �ð��� ���� � �켱���� ť�� ��������� ����, �� �ڿ� �߰���.
			}
		}

		for (j = 0; j < n_queue; j++) {
			if (head[j]->next == NULL)
				continue;
			else {
				kill(head[j]->next->data, SIGCONT);
				break;
			}
		} // �� ���� ����Ǿ�� �� process�� ã�� Cont ��Ŵ.
	}

	else {
		for (l = 0; l < n_queue; l++) {
			if (head[l]->next == NULL)
				continue;
			else {
				kill(head[l]->next->data, SIGSTOP);
				break;
			}
		} // Time slice�� ��� �����ϰ� ���� ���Ḧ �ؾ��ϴ� �����̶��, Process�� �����Ű�� exit(0)�� ȣ���Ѵ�.
		for (; l < n_queue; l++) {
			if (head[l]->next != NULL) {
				kill(head[l]->next->data, SIGKILL);
				head[l]->next;
			}
			else continue;
		}
		exit(0);
	}
}

int main(int argc, char* argv[]){
	pid_t c_pid;
	int iter;
	int i;
	int v1 = atoi(argv[1]); 
	ts = atoi(argv[2]); // ���� ���� �� �Է� ���ڸ� int�� �ٲ�.

	if(argc != 3){ // 
		//fprintf(stderr, "unexpected input");
		return 0; 
	}

	head[0] = (struct NODE*)malloc(sizeof(struct NODE));
	head[1] = (struct NODE*)malloc(sizeof(struct NODE));
	head[2] = (struct NODE*)malloc(sizeof(struct NODE)); // malloc���� head�� ����

	head[0]->next = NULL;
	head[1]->next = NULL;
	head[2]->next = NULL; // ���� ��� �ּҴ� NULL �ʱ�ȭ

	for(iter = 0; iter < v1; iter++){
		c_pid = fork();
		
		char a[2] = "A";
		a[0] += iter; // ���ڷ� �ֱ� ���� ���ڿ� ����

		if(c_pid == -1){
			//fprintf(stderr, "Fork Error!");
			return 0;
		}

		else if(c_pid == 0){
			if(execl("./ku_app", "./ku_app", a, NULL) == -1){
				//fprintf(stderr, "excl Error");
				return 0;
			} //execl�� �������� ������ ��� return ��Ŵ.
		}

		else {
			struct NODE* temp = head[0];
			struct NODE* body = (struct NODE*)malloc(sizeof(struct NODE));
			body->next = NULL;
			body->data = c_pid;
			body->time = 0; //�߰��� process�� list�� node�� ����.

			while(temp->next != NULL){
				temp = temp->next;
			} //�� ���� ã��.
			
			temp->next = body;
		}
	}

	sleep(5);

	struct itimerval temp;
	struct sigaction sa;

	temp.it_interval.tv_sec = 1;
	temp.it_interval.tv_usec = 0;
	temp.it_value.tv_sec = 1;
	temp.it_value.tv_usec = 0; //Ÿ�̸� �ʱ�ȭ

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &sig_handler;
	sigaction(SIGALRM, &sa, NULL); //sig handler�� ���

	setitimer(ITIMER_REAL, &temp, NULL); //timer ����
	
	kill(head[0]->next->data, SIGCONT); //process�� ���۽�Ŵ.

	while(1){}; //���ѷ����� ���� �� ���� ��ٸ�.

	return 0;
}
