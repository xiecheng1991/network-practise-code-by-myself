#include"../head.h"
int
main( int argc, char* argv[] ) {
  
      int listfd;
      listfd = socket( AF_INET, SOCK_STREAM, 0 );
  
      struct sockaddr_in server_addr;
      bzero( &server_addr, sizeof( server_addr ) );
>>    socklen_t server_len;
  
      server_addr.sin_family = AF_INET;
      server_addr.sin_addr.s_addr = htonl( INADDR_ANY );
      server_addr.sin_port = htons( SEVR_PORT );
  
      int on = 1;
  
  #include"../head.h"
  
  int
  main( int argc, char* argv[] ) {
      if ( argc != 2 ) {
          error( 1, 0, "usage:tcpclient <IPaddress>" );
      }
  
      int socket_fd;
      socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
  
      struct sockaddr_in server_addr;
      bzero( &server_addr, sizeof( server_addr ) );
      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons( SEVR_PORT );
      inet_pton( AF_INET, argv[1], &server_addr.sin_addr );
  
>>    socklen_t server_len = sizeof( server_addr );
      
      int connect_rt = connect( socket_fd, ( struct sockaddr* )&server_addr, sizeof( server_addr ) );
      if ( connect_rt < 0 ) {
          error( 1, errno, "connect failed" );
      }
  
      struct {
          u_int32_t message_length;
          u_int32_t message_type;
          char buf[128];
      } message;
  
      int n;  
          
      while( fgets( message.buf, sizeof( message.buf ), stdin ) != NULL ) {
          n = strlen( message.buf );
          message.message_length = htonl( n );
          message.message_type = 1;
          if ( send( socket_fd, ( char* ) &message, sizeof( message.message_length )  + sizeof( message.message_type ) + n, 0 ) <   0 ) {
              error( 1, 0, "send failure" );
          }
      }
  
      exit( 0 );
  }
    
