 网络编程实战13
 主要是讲udp套接字也可以调用connect
 
 调用connect 主要是为了建立upd套接字--目的地址+端口的映射关系,这样操作系统就可以将icmp报文信息返回给应用程序,应用程序再调用recv或者recvfrom时
 就可以收到系统返回的connection refuesd等信息
 
 针对服务器端一般不会调用connect函数,如果调用了connect函数的话就会只接收connect与套接字相绑定的地址的消息,此操作一般用于服务器只服务于这个唯一的客户
 
 #include"../head.h"
  
  static int count;
  
static void sig_int( int signo ) {
      printf( "\nreceived %d datagrams\n", count );
      exit( 0 );
  }
  
static void sig_pipe( int signo ) {
      printf( "\n received sig_pipe\n" );
      exit( 0 );
  }
  
  int
main( int argc, char* argv[] ) {
  
      int socket_fd;
      socket_fd = socket( AF_INET, SOCK_DGRAM, 0 );
      struct sockaddr_in server_addr;
      bzero( &server_addr, sizeof( server_addr ) );
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
      server_addr.sin_port = htons( SEVR_PORT );
  
      int rt1 = bind( socket_fd, ( struct sockaddr* )& server_addr, sizeof( server_addr ) );
      if ( rt1 < 0 ) {
          error( 1, errno, "bind failed" );
      }
    
      signal( SIGINT, sig_int );
      signal( SIGPIPE, sig_pipe );
    
      socklen_t client_len;
      struct sockaddr_in cliaddr;
    
      client_len = sizeof( cliaddr );
      char message[MAXLINE];
      message[0] = 0;
      count = 0;
  
      int n = recvfrom( socket_fd, message, MAXLINE, 0, ( struct sockaddr* )&cliaddr, &client_len );
      if ( n < 0 ) {
          error( 1, errno, "recvfrom error" );
      }
  
      message[n] = 0;
      printf( "received %d bytes %s\n", n, message );
  
      if ( connect( socket_fd, ( struct sockaddr* ) &cliaddr, client_len ) < 0 ) {
          error( 1, errno, "connect failed" );
      }
  
      while ( strncmp( message, "goodbye", 7 ) != 0 ) {
          char send_line[MAXLINE];
          sprintf( send_line, "Hi, %s\n", message );
          size_t rt = send( socket_fd, send_line, strlen( send_line ), 0 );
          if ( rt < 0 ) {
              error( 1, errno, "send failed" );
          }
  
          printf( "send bytes %zu\n", rt );
          size_t rc = recv( socket_fd, message, MAXLINE, 0 );
          if ( rc < 0 ) {
              error( 1, errno, "recv failed" );
          }
          count++;
  
      }
  
      exit( 0 );
  }
