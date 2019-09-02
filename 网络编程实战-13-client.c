网络编程实战-13客户端逻辑代码
include"../head.h"
  
int
main( int argc, char* argv[] ) {
    if ( argc != 2 ) {
        error( 1, 0, "useage: graceclient <IPaddress>" );
    }
    int socket_fd;
    socket_fd = socket( AF_INET, SOCK_DGRAM, 0 );

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

    char send_line[MAXLINE], recv_line[MAXLINE+1];

    int n;

    while( fgets( send_line, MAXLINE, stdin ) != NULL ) {
        int i = strlen( send_line );
        if ( send_line[i-1] == '\n' ) {
            send_line[i-1] = 0;
        }
        printf( "now sending %s\n", send_line );
        size_t rt = send( socket_fd, send_line, strlen( send_line ), 0 );
        if ( rt < 0 ) {
            error( 1, errno, "send failed" );
        }
        printf( "send bytes :%zu\n", rt );
        recv_line[0] = 0;
        n = recv( socket_fd, recv_line, MAXLINE, 0 );
        if ( n < 0 ) {
            error( 1, errno, "recv failed" );
        }
        recv_line[n] = 0;
        fputs( recv_line, stdout );
        fputs( "\n", stdout );
    }
    exit( 0 );
}
