#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/time.h>
#include <string.h>

int term = 1; // time slice 의 간격
int count = 0; // 현재 time slice 실행 횟수
int ts; // 실행할 time slice 의 횟수
int n_queue = 3; // 큐의 갯수 (우선 순위 3개)
int game_tol = 2; // priority 가 감소하는 실행 시간
int pu_term = 10; // priority 를 초기화하는 간격

struct NODE {
	pid_t data;
	struct NODE* next;
	int time;
}; // Linked list

struct NODE* head[3]; // Queue 3개를 배열로 선언

void sig_handler(int signal){
	int i = 0;
	int j = 0;
	int l = 0;
	int k = 0;
	int m = 0;// ijlkm는 iterator
	
	if (++count < ts){

		if (count % pu_term == 0) { // 10초가 실행되었을 경우
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
			} // head[0]에 모든 Node들을 연결하고 head[1], head[2] 에서 연결을 끊음.
		}

		for (i = 0; i < n_queue; i++) {
			if (head[i]->next == NULL)
				continue;
			else { // 현재 실행중인 프로세스를 찾고 Stop.
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
				} // Stop 시킨 뒤 시간에 따라서 어떤 우선순위 큐에 집어넣을지 고르고, 맨 뒤에 추가함.
			}
		}

		for (j = 0; j < n_queue; j++) {
			if (head[j]->next == NULL)
				continue;
			else {
				kill(head[j]->next->data, SIGCONT);
				break;
			}
		} // 그 다음 실행되어야 할 process를 찾아 Cont 시킴.
	}

	else {
		for (l = 0; l < n_queue; l++) {
			if (head[l]->next == NULL)
				continue;
			else {
				kill(head[l]->next->data, SIGSTOP);
				break;
			}
		} // Time slice를 모두 진행하고 만약 종료를 해야하는 시점이라면, Process를 종료시키고 exit(0)을 호출한다.
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
	ts = atoi(argv[2]); // 변수 선언 및 입력 인자를 int로 바꿈.

	if(argc != 3){ // 
		//fprintf(stderr, "unexpected input");
		return 0; 
	}

	head[0] = (struct NODE*)malloc(sizeof(struct NODE));
	head[1] = (struct NODE*)malloc(sizeof(struct NODE));
	head[2] = (struct NODE*)malloc(sizeof(struct NODE)); // malloc으로 head를 생성

	head[0]->next = NULL;
	head[1]->next = NULL;
	head[2]->next = NULL; // 다음 노드 주소는 NULL 초기화

	for(iter = 0; iter < v1; iter++){
		c_pid = fork();
		
		char a[2] = "A";
		a[0] += iter; // 인자로 넣기 위한 문자열 생성

		if(c_pid == -1){
			//fprintf(stderr, "Fork Error!");
			return 0;
		}

		else if(c_pid == 0){
			if(execl("./ku_app", "./ku_app", a, NULL) == -1){
				//fprintf(stderr, "excl Error");
				return 0;
			} //execl을 실행하지 못했을 경우 return 시킴.
		}

		else {
			struct NODE* temp = head[0];
			struct NODE* body = (struct NODE*)malloc(sizeof(struct NODE));
			body->next = NULL;
			body->data = c_pid;
			body->time = 0; //추가할 process를 list의 node로 생성.

			while(temp->next != NULL){
				temp = temp->next;
			} //맨 끝을 찾음.
			
			temp->next = body;
		}
	}

	sleep(5);

	struct itimerval temp;
	struct sigaction sa;

	temp.it_interval.tv_sec = 1;
	temp.it_interval.tv_usec = 0;
	temp.it_value.tv_sec = 1;
	temp.it_value.tv_usec = 0; //타이머 초기화

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &sig_handler;
	sigaction(SIGALRM, &sa, NULL); //sig handler를 등록

	setitimer(ITIMER_REAL, &temp, NULL); //timer 설정
	
	kill(head[0]->next->data, SIGCONT); //process를 시작시킴.

	while(1){}; //무한루프로 끝날 때 까지 기다림.

	return 0;
}
