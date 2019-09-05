    网络编程实战第十一节内容:优雅的关闭还是粗暴的关闭
   本节主要是讲了 关闭连接的两种方式
   1. close
      close并不一定会发送fin包给对方,比如用fork创建进程对fd进行了拷贝则导致描述符引用计数+1 所以close只是对套接字fd的引用计数-1直到引用计数为0
           并且立即关闭 读和写两个操作
           在读方面 系统内核将套接字设置为不可读，任何对该套接字进行读的操作都将返回错误
           在写方面 系统内核将套接字设置为不可写,并且把发送缓冲区的内容发送给对方,并发送一个fin保温 任何对该套接字进行写操作都将返回错误
           如果对端没有检测到套接字已关闭,还继续发送报文,将会收到一个rst报文
           所以总得来说close是非常粗暴的关闭
   2. shutdow
      shutdow( opt )
      opt = 0(shut_rd)关闭读端,如果对已经关闭读端的套接字进行读操作会返回EOF,并且会立即丢弃套接字上接收缓冲区的内容,如果对方有数据发送过来则会立即丢弃并回复一个ack
      opt = 1(shut_wr)关闭写端,立即进入半关闭状态,此时不管套接字引用计数为多少,立即关闭写端，并把发送缓冲区的数据立即发送给对方,最后发送一个fin包,
              如果此时再进行写操作就会出错
      opt = 2(shut_rdwr) 分别执行一次shut_rd和shut_wr操作关闭读和写两个方向的操作
      shutdown总得来说是非常优雅的关闭
   在客户端中还简单的使用了一下select模型,select模型和epoll模型的主要差别在于 select可以检测的套接字数量较少,并且每次轮询过后还要再次向内核注册需要
   关心的套接字,所以在内核和应用层来回交换数据
    1 #include"../head.h"
    2 
    3 int
    4 main( int argc, char* argv[] ) {
    5     if ( argc != 2 ) {
    6         error( 1, 0, "useage: graceclient <IPaddress>" );
    7     }               
    8     int socket_fd;            
    9     socket_fd = socket( AF_INET, SOCK_STREAM, 0 );
   10                                           
   11     struct sockaddr_in server_addr;   
   12     bzero( &server_addr, sizeof( server_addr ) );
   13     server_addr.sin_family = AF_INET;
   14     server_addr.sin_port = htons( SEVR_PORT );
   15     inet_pton( AF_INET, argv[1], &server_addr.sin_addr.s_addr );
   16     socklen_t server_len = sizeof( server_addr );
   17                                       
   18     int connect_rt = connect( socket_fd, ( struct sockaddr* ) &server_addr, server_len );
   19                                         
   20     if ( connect_rt < 0 ) {              
   21         error( 1, errno, "connect error" );       
   22     }
   23                                           
   24     char send_line[MAXLINE], recv_line[MAXLINE+1]; 
   25                                                             
   26     int n;                                                   
   27     fd_set readmask;                       
   28     fd_set allreads;                                
   29                                             
   30     FD_ZERO( &allreads );                            
   31     FD_SET( 0, &allreads ); //0标准输入      
   32     FD_SET( socket_fd, &allreads );           
   33     for ( ; ; ) {                              
   34         readmask = allreads;                            
   35         int rc = select( socket_fd + 1, &readmask, NULL, NULL, NULL );
   36         if ( rc < 0 ) {                          
   37             error( 1, errno, "select error" );    
   38         }                                                  
   39         if ( FD_ISSET( socket_fd, &readmask ) ) {
   40             n = read( socket_fd, recv_line, MAXLINE );
   41             if ( n < 0 ) {
   42                 error( 1, errno, "read error" );
   43             } else if( n == 0 ) {
   44                 error( 1, 0, "server terminated \n" );
   45             }
   46             recv_line[n] = 0;
   47             fputs( recv_line, stdout );
   48             fputs( "\n", stdout );
   49         }
   50 
   51         if ( FD_ISSET( 0, &readmask ) ) {
   52             if ( fgets( send_line, MAXLINE, stdin ) != NULL ) {
   53                 if ( strncmp( send_line, "shutdown", 8  ) == 0 ) {
   54                     //shutdown关闭socket
   55                     FD_CLR( 0, &allreads );
   56                     if ( shutdown( socket_fd, 1 ) ) {
   57                         error( 1, errno, "shutdown failed" );
   58                     }
   59                 } else if ( strncmp( send_line, "close", 5 ) == 0 ) {
   60                     FD_CLR( 0, &allreads );
   61                     if ( close( socket_fd ) ) {
   62                         error( 1, errno, "close failed" );
   63                     }
   64                     size_t rt = write( socket_fd, send_line, strlen( send_line ) );
   65 //          sleep( 6 );
   66                     exit( 0 );
   67                 } else {
   68                     int i = strlen( send_line );
   69                     if ( send_line[i-1] == '\n' ) {
   70                         send_line[i-1] = 0;
   71                     }
   72                     printf( "now sending %s\n", send_line );
   73                     size_t rt = write( socket_fd, send_line, strlen( send_line ) );
   74                     if ( rt < 0 ) {
   75                         error( 1, errno, "write failed" );
   76                     }
   77                     printf( "send bytes:%zu \n", rt );
   78                 }
   79 
   80             }
   81         }
   82 
   83     }
   84 
   85 }
  
             
