网络编程实战12 连接检测
1. tcp自带的 keep-alive机制
2. 应用层自己维护心跳包
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
  
      if ( argc < 2 ) {
          error( 1, 0, "useage: tcpserver <sleepingtime>" );
      }
  
      int sleeptime = atoi( argv[1] );
  
      int listenfd;
      listenfd = socket( AF_INET, SOCK_STREAM, 0 );
      struct sockaddr_in server_addr;
      bzero( &server_addr, sizeof( server_addr ) );
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
      server_addr.sin_port = htons( SEVR_PORT );
  
      int rt1 = bind( listenfd, ( struct sockaddr* )& server_addr, sizeof( server_addr ) );
      if ( rt1 < 0 ) {
          error( 1, errno, "bind failed" );
      }
  
      int rt2 = listen( listenfd, LISTENNQ );
      if ( rt2 < 0 ) {
          error( 1, errno, "error listen" );
      }
  
      signal( SIGINT, sig_int );
      signal( SIGPIPE, sig_pipe );
  
      fd_set readall;
      fd_set readmask;
      FD_ZERO( &readall );
      FD_SET( listenfd, &readall );
      int connfd;
  
      //char message[MAXLINE];
      messageObject message;
      count = 0;
      for ( ; ; ) {
          readmask = readall;
          int select_rt = select( 1024, &readmask, NULL, NULL, 0 );
          if ( select_rt < 0 ) {
              error( 1, errno, "select error" );
          }
          if ( FD_ISSET( listenfd, &readmask ) ) {
                  struct sockaddr_in cli_addr;
                  socklen_t cli_len = sizeof( cli_addr );
                  connfd = accept( listenfd, ( struct sockaddr* )& cli_addr, &cli_len );
                  if ( connfd < 0 ) {
                          error( 1, errno, "accept error" );
                  }
                  printf( "new connect come\n" );
                  FD_SET( connfd, &readall );
          }
  
          if ( FD_ISSET( connfd, &readmask ) ) {
                  int n = read( connfd, ( char* ) &message, sizeof( message ) );
                  if ( n < 0 ) {
                          error( 1, errno, "error read" );
                  } else if ( n == 0 ) {
                          error( 1, 0, "client closed\n" );
                  }
                  printf( "received %d bytes\n", n );
                  count++;
  
                  switch( ntohl( message.type ) ) {
                      case MSG_TYPE1:
                              printf( "process MSG_TYPE1 \n" );
                              break;
                      case MSG_TYPE2:
                              printf( "process MSG_TYPE2 \n" );
                              break;
                      case MSG_PING:{
                              printf( "printf received client heartbeats \n" );
                              messageObject pongmsg;
                              pongmsg.type = MSG_PONG;
                              sleep( sleeptime );
                              ssize_t rt = send( connfd, ( char* ) &pongmsg, sizeof( pongmsg ), 0  );
                              if ( rt < 0 ) {
                                  error( 1, errno, "send failed" );
                              }
                              break;
                          }
                      default:
  
                              error( 1, 0, "unknow message type (%d)\n", ntohl( message.type ) );
  
                  }
          /*          
                  message[n] = 0;
                  printf( "received %d bytes: %s\n", n, message );
                  count++;
                  char send_line[MAXLINE];
                  sprintf( send_line, "Hi, %s", message );
                  sleep( 5 );
                  //这里的问题是在一个已经关闭的socket上写入数据会触发sigpipe信号 一般close会关闭socket,并且回收socket资源
                  //shutdown 一般是断开连接并不会断开socket也就是回收socket相关的资源等等
                  int write_nc = send( connfd, send_line, strlen( send_line ), 0 );
                  printf( "send bytes:%u \n", write_nc );
                  if ( write_nc < 0 ) {
                          error( 1, errno, "error write" );
                  }
                  */
          }
  
      }
  
      exit( 0 );
  }
