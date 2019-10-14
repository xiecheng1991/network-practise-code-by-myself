#include"../head.h"

#define THREAD_NUMBER 4
#define BLOCK_QUEUE_SIZE 100

extern void loop_echo( int );

typedef struct {
	pthread_t thread_tid;
	long thread_count;
} Thread;

typedef struct {
	int number;
	int *fd;
	int front;
	int rear;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} block_queue;

void 
block_queue_init( block_queue* blockQueue, int number ) {
	blockQueue->number = number;
	blockQueue->fd = ( int* )calloc( number, sizeof( int ) );
	blockQueue->front = blockQueue->rear = 0;
	pthread_mutex_init( &blockQueue->mutex, NULL );
	pthread_cond_init( &blockQueue->cond, NULL );
}

void 
block_queue_push( block_queue* blockQueue, int fd ) {
	pthread_mutex_lock( &blockQueue->mutex );
	blockQueue->fd[blockQueue->rear] = fd; 
	if ( ++blockQueue->rear == blockQueue->number ) {
		blockQueue->rear = 0;
	}

	pthread_cond_signal( &blockQueue->cond );

	pthread_mutex_unlock( &blockQueue->mutex );

}

int 
block_queue_pop( block_queue* blockQueue ) {
	pthread_mutex_lock( &blockQueue->mutex );
	while ( blockQueue->front == blockQueue->rear ) {
		pthread_cond_wait( &blockQueue->cond, &blockQueue->mutex );
	}

	int fd = blockQueue->fd[blockQueue->front];
	if ( ++blockQueue->front == blockQueue->number ) {
		blockQueue->front = 0;
	}

	printf( "pop fd :%d\n", fd );
	pthread_mutex_unlock( &blockQueue->mutex );
	
	return fd;
}

void *
Thread_run( void* args ) {
	pthread_t tid = pthread_self();
	pthread_detach( tid );
	block_queue* blockQueue = ( block_queue* ) args;
	while ( 1 ) {
		int fd = block_queue_pop( blockQueue );
		printf( "get fd in Thread_run, fd = %d, tid = %d\n", fd, tid );
		loop_echo( fd );
	}
}

int
main( int argc, char* argv[] ) {
	int listenfd;
	listenfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( listenfd < 0 ) {
		error( 1, errno, "socket failed" );
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons( 43211 );
	addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

	if ( bind( listenfd, ( struct sockaddr* ) &addr, sizeof( addr ) ) < 0 ) {
		error( 1, errno, "bind failed" );
	}

	if ( listen( listenfd, LISTENNQ ) < 0 ) {
		error( 1, errno, "error failed" );
	}
	
	Thread* thread_array = ( Thread* )calloc( THREAD_NUMBER, sizeof( Thread ) );

	block_queue blockQueue;
	block_queue_init( &blockQueue, BLOCK_QUEUE_SIZE );

	for ( int i = 0; i < THREAD_NUMBER; ++i ) {
		pthread_create( &( thread_array[i].thread_tid ), NULL, &Thread_run, ( void* ) &blockQueue );
	}

	while( 1 ) {
		struct sockaddr_in cli_addr;
		socklen_t cli_len = sizeof( cli_addr );
		int fd = accept( listenfd, ( struct sockaddr* ) &cli_addr, &cli_len );
		if ( fd < 0 ) {
			error( 1, errno, "accpet failed" );
		} else {
			block_queue_push( &blockQueue, fd );
		}
	}

	exit( 0 );
}
