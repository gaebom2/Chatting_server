#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>

//클라이언트 구조체 선언
typedef struct {
	int fd; 	//각 클라이언트의 연결 정보를 받을 변수
	char ip[20]; 	//각 클라이언트의 ip주소
}client;

//서버에서 각 클라이언트로 응답보내는 쓰레드 함수
void *client_thread(client *client_info);

int clientSize = 0; 		//접속중인 클라이언트 수 체크하는 변수
client client_info[5] = {0}; 	//클라이언트 정보 받는 배열 (위에서 정의한 클라이언트 구조체형)
pthread_t ptid[5] = {0}; 	//쓰레드 배열 선언

int main(int argc, char **argv){

	int idx;				//반복문 변수
	struct sockaddr_in sin, cli; 		//소켓 변수 생성
	int sd;					//소켓변수 저장할 int형 변수
	int ns, clientlen = sizeof(cli);	//소켓변수 저장할 int형 변수

	printf("=========================\n");
	printf("TCP/IP Chatting Program - Server\n");
	printf("=========================\n\n");

	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){ 	//소켓 생성
		printf("socket() error\n"); 			//-1로, 제대로 생성되지 않으면 오류메세지 출력
	}

	//정보 저장
	memset(&sin, 0, sizeof(sin));			 //sin변수 초기화
	sin.sin_family = AF_INET;			 //인터넷소켓 지정
	sin.sin_port = (htons(atoi(argv[1])));		 //포트번호 저장 (프로그램 실행 시 포트번호 입력받음)
	sin.sin_addr.s_addr = inet_addr("0.0.0.0");	 //클라이언트 ip주소 저장 (0.0.0.0은 모든 클라이언트 허용)


	if((bind(sd, (struct sockaddr *)&sin, sizeof(sin)))==-1){//위에서 입력받은 정보를 변수 sd에 binding
		printf("bind() eroor\n");			 //-1로 바인딩 실패하면  오류메세지 출력
	}

	if((listen(sd, 5))==-1){		 //대기 큐 생성, 서버 소켓에 대기할 수 있는 연결 요청의 최대 개수를 5로 지정
		printf("listen() error\n");	 //-1로 제대로 대기큐를 생성하지 못하면 오류메세지 출력
	}	

	clientlen = sizeof(cli); //클라이언트 소켓 변수 크기 저장

	while(1){
		//서버로 클라이언트가 들어올때마다 ns변수에 저장
		if((ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) ==-1){ //소켓 연결 요청 수락
			printf("socket connect error\n");			 //-1로 연결 실패하면 오류메세지 출력
		}

		if(clientSize == 5){		 //새로 연결이 된 상태에서 클라이언트가 꽉 차있는 경우
			printf("Socket Full\n"); //꽉차있다는 메세지 출력
			close(ns);		 // 새로 연결된 클라이언트 close
			continue;		 // 계속 실행
		}
		
		if(ns < 0){				//클라이언트 연결정보가 음수이면,
			printf("failed connection\n"); 	//오류메세지 출력
			continue;			//계속 실행
		}

		for(idx=0; idx<5; idx++){ 			//클라이언트 최대 개수까지 반복
			if(client_info[idx].fd == 0){ 		//클라이언트의 fd가 0이면
				client_info[idx].fd = ns;	//fd에 현재 클라이언트 연결정보 저장(원래 0이었던 fd를 현재 클라이언트 연결정보로 바꿔주고나서 break
				break;				//break
			}
		}
		strcpy(client_info[idx].ip, inet_ntoa(cli.sin_addr)); //현재 연결된 정보를 클라이언트 배열에 저장
		pthread_create(&ptid[idx], NULL, (void*)client_thread, &client_info[idx]); //클라이언트 연결될 때 마다 쓰레드 생성

		clientSize++; //클라이언트 수 증가
		//printf("현재들어와있는 소켓 : %d, 클라이언트 수 : %d\n", client_info[idx].fd, clientSize); //현재 연결 정보 출력
	}
	close(sd); //사용 다한 변수 close
	return 0;
}

//클라이언트와 데이터 주고받는 함수
void *client_thread(client* client_data){
	int result;		//메세지 길이 저장할 변수
	int current; 		//반복문 변수
	char msg[120]; 		//클라이언트에서 받은 메세지를 저장할 변수
	char allMsg[120]; 	//메세지를 다른 클라이언트에게도 보여주기위한 변수

	while(1){

		memset(allMsg, '\0', 120); 	//변수NLLL로 초기화
		memset(msg, '\0', 120); 	//변수NULL로 초기화
		//result는 메세지의 길이.
		result = read(client_data->fd, msg, sizeof(msg)); //read함수를 통해 현재 연결되어있는 클라이언트에서 메세지를 받아와 msg변수에 저장하고, read함수의 리턴값인 읽은 메세지의 길이를 result변수에 저장

		if(result <= 0) //읽은 메세지가 없으면 break
			break;
		else{
			printf("%s\n", msg); //읽은 메세지를 서버에서 출력한다.
		}
		//all메세지에 저장
		result = sprintf(allMsg, "%s\n", msg); //다른 클라이언트 창에 출력하기 위한 변수인 allMsg에 메세지 저장

		for(current=0; current<5; current++){ 				//클라이언트 수 만큼 메세지를 전송한다.
			if(client_info[current].fd == 0) continue; 		//fd가 0인경우는 클라이언트가 없는 경우이므로 건너뛴다.
			send(client_info[current].fd, allMsg, result, 0); 	//크기가 result인 메세지를 현재 연결되어있는 클라이언트의 소켓들로 보낸다. 
		}
	}
	
	close(client_info->fd);				//연결 해제 된 클라이언트 종료
	clientSize--; 					//클라이언트 수 감소
	client_info->fd = 0;				//클라이언트 접속 해제 (다시 0으로 바꿔준다.)
	printf("close(%s) \n", client_info->ip); 	//종료된 클라이언트 ip주소 출력
	return NULL;
}
