 /*
      tcp数据流协议
      主要是tcp分组问题, 例如调用两次send 第一次发送hello 第二次发送world,由于tcp分组可能会导致
      *****hellow   或者lloworld***等等情况的出现,需要在应用层方面处理报文的解析和读取
      1. 显示的指定消息的长度 接收端从字节流当做取指定长度的 数据 再进行转换得到 例如本文代码
      2. 特殊字符作为边界 例如http协议
  */
  
    #include"../head.h"
  
  static int count;
  
  static void
sig_int( int signo ) {
      printf( "\n received %d datagrams\n", count );
      exit( 0 );
  }
  
  size_t
  readn( int fd, void* buffer, size_t length ) {
      size_t count;
      ssize_t nread;
      char* ptr;
         
        
>>    ptr = buffer;
      count = length;
      while( count > 0 ) {
          nread = read( fd, ptr, count );
        
          if ( nread < 0 ) {
              if ( errno == EINTR ) {
                  continue;
              } else {
                  return -1;
              }
          } else if ( nread == 0 ) {
              break;
          }
          count -= nread;
          ptr += nread;
      }
      return length - count;
  }
  
  size_t
  read_message( int fd, char* buffer, size_t length ) {
      u_int32_t msg_length;
      u_int32_t msg_type;
      int rc;
      rc = readn( fd, ( char* ) &msg_length, sizeof( u_int32_t ) );
      if ( rc != sizeof( u_int32_t ) ) {
          return rc < 0 ? - 1 : 0;
      }
  
      msg_length = ntohl( msg_length );
  
      rc = readn( fd, ( char* ) &msg_type, sizeof( msg_type ) );
  
      if ( rc != sizeof( u_int32_t ) ) {
          return rc < 0 ? -1 : 0;
      }
  
      if ( msg_length > length ) {
          return -1;
      }
  
      rc = readn( fd, buffer, msg_length );
  
>>    if ( rc != msg_length ) {
          return rc < 0 ? -1 : 0;
      }
      return rc;
  
  }
  
  int
>>main( int argc, char* argv[] ) {
  
      int listfd;
      listfd = socket( AF_INET, SOCK_STREAM, 0 );
  
      struct sockaddr_in server_addr;
      bzero( &server_addr, sizeof( server_addr ) );
>>    socklen_t server_len;
  
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
      server_addr.sin_port = htons( SEVR_PORT );
  
      int on = 1;
  
      setsockopt( listfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof( on ) );
  
  
      int rt1 = bind( listfd, ( struct sockaddr* ) &server_addr, sizeof( server_addr ) );
      if ( rt1 < 0 ) {
          error( 1, errno, "bind failed" );
      }
  
      int rt2 = listen( listfd, LISTENNQ );
      if ( rt2 < 0 ) {
          error( 1, errno, "listen failed" );
      }
  
  
      int connfd;
      struct sockaddr_in cli_addr;
      socklen_t cli_len = sizeof( cli_addr );
  
      if ( ( connfd = accept( listfd, ( struct sockaddr* ) &cli_addr, &cli_len ) ) < 0 ) {
          error( 1, errno, "bind failed" );
      }
  
      char buf[128];
      count = 0;
  
      while( 1 ) {
          int n = read_message( connfd, buf, sizeof( buf ) );
          if ( n < 0 ) {
              error( 1, errno, "error read message" );
          } else if ( n == 0 ) {
              error( 1, 0, "client close" );
          }
          buf[n] = 0;
          printf( "received %d bytes:%s\n", n, buf );
          count++;
      }
      exit( 0 );
  }
