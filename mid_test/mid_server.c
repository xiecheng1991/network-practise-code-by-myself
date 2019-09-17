/*
 	初版代码于 2019/09/17日完成
	同时增加心跳包检测和端口重用
 */

#include "../head.h"

void 
exec( const char* cmd, char* retbuf ) {
	char buf[1024];
	FILE* pp;
	pp = popen( cmd, "r" );
	if ( !pp ) {
		error( 1, errno, "popen failed" );
	}

	while( fgets( buf, sizeof( buf ), pp ) != NULL ) {
		strcat( retbuf, buf );
	}
	pclose( pp );
}

int
main( int argc, char* argv[] ) {
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( 12345 );
	server_addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
	socklen_t len = sizeof( server_addr );

	int listenfd = socket( AF_INET, SOCK_STREAM, 0 );
	
	int on = 1; //设置是否允许地址重用避免讨厌的time_wait状态
	setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) );

	if ( bind( listenfd, ( struct sockaddr* )& server_addr, len ) < 0 ) {
		error( 1, errno, "bind failed" );
	}

	if ( listen( listenfd, LISTENNQ ) < 0 ) {
		error( 1, errno, "listen failed" );
	}

	int client_fd;
	char buf[MAXLINE];

	fd_set readall;
	fd_set readmask;
	FD_ZERO( &readall );
	FD_SET( listenfd, &readall );
	
	char sendline[1024];
	
	struct timeval tv;
	tv.tv_sec = KEEP_ALIVE_TIME;
	tv.tv_usec = 0;

	//心跳超时次数
	int heartbeats = 0; 

	while( 1 ) {
		readmask = readall;
		int select_rt = select( 1024, &readmask, NULL, NULL, &tv );
		if ( select_rt < 0 ) {
			error( 1, errno, "select failed" );
		}

		if ( select_rt == 0 ) {
			//10秒钟的心跳超时了
			tv.tv_sec = KEEP_ALIVE_INTERVAL;
			printf( "客户端超时%d次\n", heartbeats );
			if ( ++heartbeats >= KEEP_ALIVE_PROBETTIMES ) {
				printf( "客户端处于不活跃状态,五秒钟后退出\n" );
				//关闭写端，会清空发送缓冲区同时发送一个fin保给客户端
				shutdown( client_fd, SHUT_WR );
				//因为只有一个客户端,所以这个地方也可以考虑退出了,不退出的话,select还会不停的超时
				sleep( 5 );
				exit( 0 );
			}
			continue;
		}
		tv.tv_sec = KEEP_ALIVE_TIME;
		if ( FD_ISSET( listenfd, &readmask ) ) {
			struct sockaddr_in cli_addr;
			socklen_t cli_len;
			//有客户端连接上来
			client_fd = accept( listenfd,  ( struct sockaddr* )& cli_addr, &cli_len );
			FD_SET( client_fd, &readall );
		} else {
			int recv_rt = recv( client_fd, buf, MAXLINE, 0 );
			//判断接收异常
			if ( recv_rt < 0 ) {
				error( 1, errno, "recv failed" );
			}
			printf( "received %d bytes %s\n", recv_rt, buf );
			
				memset( sendline, 0, sizeof( sendline ) );
			if ( strncmp( buf, "pwd", 3 ) == 0 ) {
				exec( buf, sendline );
			} else if ( strncmp( buf, "ls", 2 ) == 0 ) {
				exec( buf, sendline );
			} else if ( strncmp( buf, "cd", 2 ) == 0 ) {
				//匹配两个cd字符 表明是要进入某个目录
				chdir( buf+2 );
			} 				
	
			mid_msg_object msg;
			msg.len = strlen( sendline );		

			if ( strlen( sendline ) != 0 ) {
				strcpy( msg.data, sendline );
			}

			printf( "send %d bytes %s\n", msg.len, msg.data );

			if ( send( client_fd, ( void* ) &msg, sizeof( msg ), 0 ) < 0 ) {
				error( 1, errno, "send failed" );
			}

		}
	}
	exit( 0 );
}
