  #include"../head.h"
  tcp并不是总是可靠的,只有通过recv或者send函数去检测
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
  
      socklen_t server_len = sizeof( server_addr );
  
      int connect_rt = connect( socket_fd, ( struct sockaddr* )&server_addr, server_len );
      if ( connect_rt < 0 ) {
          error( 1, errno, "connect failed" );                  
      }   
    char* msg = "network programming";                        
      ssize_t n_written;
      while( 1 ) {
          n_written = send( socket_fd, msg, strlen( msg ), 0 ); 
          fprintf( stdout, "send into buffer %ld\n", n_written );
          if ( n_written <= 0 ) {
              error( 1, errno, "send error" );                  
              exit( 0 ); 
          }   
      }       
          
      exit( 0 );
  }
