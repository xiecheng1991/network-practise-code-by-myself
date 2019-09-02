keep-alive和心跳包探测对方连接状态

客户端中采用select超时来探测与服务器连接的心跳包

#include"../head.h"

int
main( int argc, char* argv[] ) {
    if ( argc != 2 ) {
        error( 1, 0, "useage: graceclient <IPaddress>" );
    }
    int socket_fd;
    socket_fd = socket( AF_INET, SOCK_STREAM, 0 );

    struct sockaddr_in server_addr;
    bzero( &server_addr, sizeof( server_addr ) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons( SEVR_PORT );
    inet_pton( AF_INET, argv[1], &server_addr.sin_addr.s_addr );
    socklen_t server_len = sizeof( server_addr );

    int connect_rt = connect( socket_fd, ( struct sockaddr* ) &server_addr, server_len );

    if ( connect_rt < 0 ) {
        error( 1, errno, "connect error" );
    }

//  char send_line[MAXLINE], recv_line[MAXLINE+1];
    char recv_line[MAXLINE];
             
    int n;
    fd_set readmask;
    fd_set allreads;
    
    FD_ZERO( &allreads );
    FD_SET( 0, &allreads ); //0标准输入
    FD_SET( socket_fd, &allreads );
    
    struct timeval tv;
    int heartbeats = 0;

    tv.tv_sec = KEEP_ALIVE_TIME;
    tv.tv_usec = 0;

    messageObject msgobj;


    for ( ; ; ) {
        readmask = allreads;
        int rc = select( socket_fd + 1, &readmask, NULL, NULL, &tv );
        if ( rc < 0 ) {
            error( 1, errno, "select error" );
        }

        if ( rc == 0 ) {
            printf( "select overtime\n" );
            if ( ++heartbeats >= KEEP_ALIVE_PROBETTIMES ) {
                error( 1, 0, "connection dead\n" );
            }
            printf( "sending heartbeat #%d\n", heartbeats );
            msgobj.type = htonl( MSG_PING );
            rc = send( socket_fd, ( char * ) &msgobj, sizeof( msgobj ), 0 );
            if ( rc < 0 ) {
                error( 1, errno, "error send" );
            }
            tv.tv_sec = KEEP_ALIVE_INTERVAL;
            continue;
        }

        if ( FD_ISSET( socket_fd, &readmask ) ) {
            n = read( socket_fd, recv_line, MAXLINE );
            if ( n < 0 ) {
                error( 1, errno, "read error" );
            } else if( n == 0 ) {
                error( 1, 0, "server terminated \n" );
            }
//          recv_line[n] = 0;
//          fputs( recv_line, stdout );
//          fputs( "\n", stdout );
            //收到了心跳包重置
            printf( "received heartbeat, make heartbeat to 0 \n" );
        }
    /*
        if ( FD_ISSET( 0, &readmask ) ) {
            if ( fgets( send_line, MAXLINE, stdin ) != NULL ) {
                if ( strncmp( send_line, "shutdown", 8  ) == 0 ) {
                    //shutdown关闭socket
                    FD_CLR( 0, &allreads );
                    if ( shutdown( socket_fd, 1 ) ) {
                        error( 1, errno, "shutdown failed" );
                    }
                } else if ( strncmp( send_line, "close", 5 ) == 0 ) {
                    FD_CLR( 0, &allreads );
                    if ( close( socket_fd ) ) {
                        error( 1, errno, "close failed" );
                    }
                    size_t rt = write( socket_fd, send_line, strlen( send_line ) );
//          sleep( 6 );
                    exit( 0 );
                } else {
                    int i = strlen( send_line );
                    if ( send_line[i-1] == '\n' ) {
                        send_line[i-1] = 0;
                    }
                    printf( "now sending %s\n", send_line );
                    size_t rt = write( socket_fd, send_line, strlen( send_line ) );
                    if ( rt < 0 ) {
                        error( 1, errno, "write failed" );
                    }
                    printf( "send bytes:%zu \n", rt );
                }

            } 
        }
        */

    }

}
