#include <sys/types.h>
#include <sys/socket.h>

// Internet socket address stuct
struct sockaddr_in Address;
int addres_size = sizeof(struct sockaddr_in);

int create_socket( )
{
  int socket_fd;

  if( ( socket_fd =  socket( AF_INET, SOCK_STREAM, 0 ) ) == SOCKET_ERROR )
  {
    printf("\nCould not make a socket\n");
    fflush(stdout);
    exit(1);
  }

  return socket_fd;
}

void bind_socket( int server_socket_fd, int port )
{

  /* fill address struct */
  Address.sin_addr.s_addr = INADDR_ANY;
  Address.sin_port = htons(port);
  Address.sin_family = AF_INET;

  printf("\nBinding to port %d", port);

  /* bind to a port */
  int bind_result = ::bind( server_socket_fd, ( struct sockaddr* ) &Address, sizeof( Address ) );
  if( bind_result == SOCKET_ERROR )
  {
    printf("\nCould not connect to host\n");
    exit(1);
  }

  /*  get port number */
  getsockname( server_socket_fd, (struct sockaddr *) &Address, (socklen_t *) &addres_size );

  printf("\nopened socket as fd (%d) on port (%d) for stream i/o\n", server_socket_fd, ntohs( Address.sin_port ) );

  printf("Server\n\
  sin_family        = %d\n\
  sin_addr.s_addr   = %d\n\
  sin_port          = %d\n"
  , Address.sin_family
  , Address.sin_addr.s_addr
  , ntohs(Address.sin_port)
  );

}
