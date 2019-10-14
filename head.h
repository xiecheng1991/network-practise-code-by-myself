#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<error.h>
#include<errno.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/select.h>
#define SEVR_PORT 12345
#define LISTENNQ 10
#define MAXLINE 4096
#include<signal.h>
#include<fcntl.h>
#include<pthread.h>

typedef struct {
	u_int32_t type;
	char data[1024];
} messageObject;

#define MSG_PING 1
#define MSG_PONG 2
#define MSG_TYPE1 11
#define MSG_TYPE2 21

#define KEEP_ALIVE_TIME 10
#define KEEP_ALIVE_PROBETTIMES 3
#define KEEP_ALIVE_INTERVAL 5

typedef struct {
	int len;
	char data[1024];
} mid_msg_object;

char 
rot13_char( char c ) {
	if ( ( c >= 'a' && c <= 'm' ) || ( c >= 'A' && c <= 'M' ) ) {
		return ( c += 13 );
	} else if ( ( c >= 'n' && c <= 'z' ) || ( c >= 'N' && c <= 'Z' ) ) {
		return ( c -= 13 );
	} else {
		return c;
	}
}

void
loop_echo( int fd ) {
	char outbuf[MAXLINE];
	size_t outbuf_used = 0;
	ssize_t result;
	while ( 1 ) {
		char ch;
		result = recv( fd, &ch, 1, 0 );

		if ( result == 0 ) {
			//连接断开了,这里的代码有问题 一个连接断开就退出了一个线程,如果断开多次 这个服务器上都没有工作线程了
			break;
		} else if ( result == -1 ) {
			error( 1, errno, "read failed" );
			break;
		}

		if ( outbuf_used < sizeof( outbuf ) ) {
			outbuf[outbuf_used++] = rot13_char( ch );
		}

		if ( ch == '\n' ) {
			send( fd, outbuf, outbuf_used, 0 );
			outbuf_used = 0;
			continue;
		}
	}
}
