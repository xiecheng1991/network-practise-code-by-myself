#include"../head.h"

int
main( int argc, char* argv[] ) {
	if ( argc < 2 ) {
		error( 1, 0, "needs ipaddr" );
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons( 12345 );
	inet_pton( AF_INET, argv[1], &server_addr.sin_addr );

	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd == -1 ) {
		error( 1, errno, "socket failed" );
	}

	if ( connect( sockfd, ( struct sockaddr* ) &server_addr, sizeof( server_addr ) ) < 0 ) {
		error( 1, errno, "connect failed" );
	}
	
	char buf[1024];
	mid_msg_object msg;
	while( fgets( buf, sizeof( buf ), stdin ) != NULL ) {
		//客户端就简单点 直接发往客户端
		if ( send( sockfd, buf, strlen( buf ), 0 ) < 0 ) {
			error( 1, errno, "send failed" );
		}
		//要清空缓冲区 不然缓冲区会有残留
		memset( msg.data, 0, sizeof(msg.data) );
		if ( recv( sockfd, ( char* ) &msg, sizeof( msg ), 0 ) < 0 ) {
			error( 1, errno, "recv failed" );
		}
		if ( msg.len != 0 ) {

			//验证数据的有效性
			if ( msg.len != strlen( msg.data ) ) {
				//说明是外挂数据乱修改的
				continue;
			}

			//说明有回复 需要打印的
			printf( "%s\n", msg.data );
		}

	}

	exit( 0 );
}
