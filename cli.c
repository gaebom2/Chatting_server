#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFSIZE 100 //보낼 메세지의 버퍼 크기 정의
#define NAMESIZE 20 //이름 크기 정의

void *send_message (void *arg);
void *recv_message (void *arg);
void error_handling (char *message);

char name[NAMESIZE] = "[Default]"; //이름 변수 생성
char message[BUFSIZE]; //메세지 변수 생성

int main (int argc, char **argv)
{
	int sock;
	struct sockaddr_in serv_addr; //소켓 변수
	pthread_t snd_thread, rcv_thread;
	void *thread_result;

	printf ("================================\n");
	printf ("TCP/IP Chatting Program - Client\n");
	printf ("================================\n\n");

	if (argc != 4)
	{
		printf ("Usa%s <IP> <port> <name>\n", argv[0]);
		exit (1);
	}

	sprintf (name, "[%s]", argv[3]);

	sock = socket (PF_INET, SOCK_STREAM, 0); //1. 소켓 생성
	if (sock == -1)
		error_handling ("socket() error");

	/*소켓 주소 구조체 생성, 보낼 address 셋팅*/
	memset (&serv_addr, 0, sizeof (serv_addr)); //소켓 변수 크기만큼 0으로 초기화
	serv_addr.sin_family = AF_INET; //인터넷 소켓 (네트워크를 이용한 통신) 지정
	serv_addr.sin_addr.s_addr = inet_addr (argv[1]); //서버의 ip주소를 숫자형태로 변환
	serv_addr.sin_port = htons (atoi (argv[2])); //서버의 포트 번호

	if (connect (sock, (struct sockaddr *) &serv_addr, sizeof (serv_addr)) == -1) //2. 서버에 접속 요청
		error_handling ("connect() error");

	pthread_create (&snd_thread, NULL, send_message, (void *) sock); // send_message 쓰레드 함수 호출, 서버에 메세지 보내기
	pthread_create (&rcv_thread, NULL, recv_message, (void *) sock); //recv_message 쓰레드 함수 호출, 서버가 보낸 데이터 읽기

	pthread_join (snd_thread, &thread_result);
	pthread_join (rcv_thread, &thread_result);

	close (sock);
	return 0;
}

void *send_message (void *arg) //send 함수 정의
{
	int sock = (int) arg;
	char name_message[NAMESIZE + BUFSIZE];
	while (1)
	{
		fgets (message, BUFSIZE, stdin); //사용자로부터 보낼 메세지 입력받기
		if (!strcmp (message, "q\n"))
		{
			close (sock);
			exit (0);
		}
		sprintf (name_message, "%s %s", name, message);
		send (sock, name_message, strlen (name_message), 0);
	}
}

void *recv_message (void *arg) //receive함수 정의
{
	int sock = (int) arg;
	char name_message[NAMESIZE + BUFSIZE];
	int str_len;
	int f_read = 1;

	while (f_read)
	{
		str_len = recv (sock, name_message, NAMESIZE + BUFSIZE - 1, 0);
		if (str_len == -1)
			f_read = 0;
		else
		{
			name_message[str_len] = 0;
			fputs (name_message, stdout);
		}
	}
}

void error_handling (char *message) //예외처리 함수
{
	fputs (message, stderr);
	fputc ('\n', stderr);
	exit (1);
}
