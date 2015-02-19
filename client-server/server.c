#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <queue>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>

void* serve(void * in_data);

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         4000

char * directory_path;
int thread_num;

#include "utils.c"
#include "cgi-utils.c"
#include "connection.c"
#include "templates.c"
#include "threading.c"
#include "queue.c"
#include "handler.c"

int main(int argc, char* argv[])
{
  int socket_fd, server_socket_fd, port, option;  /* handle to socket */
  int arg_index = 0;
  char buffer[BUFFER_SIZE];
  char ch;

  if (signal(SIGINT, sig_handler) == SIG_ERR)
  {
    printf("\ncan't catch SIGINT\n");
  }

  /* using getopt for parsing cmd line args */

  while ((option = getopt(argc, argv,"t:")) != -1) {
    switch (option) {
      case 't' : thread_num = atoi(optarg); break;
      default :
      printServerUsage( );
      return 0;
    }

    arg_index = optind;
  }

  if((argc - arg_index) < 2)
  {
    printServerUsage( );
    return 0;
  }
  else
  {
    port = atoi(argv[arg_index]);
    directory_path = argv[++arg_index];
  }

  printf("\nStarting server with %d threads", thread_num);

  /* create semaphores */
  create_semaphores( );

  /* make a thread pool */
  make_thread_pool(threads, thread_num);

  /* make a socket */
  printf("\nMaking socket");
  server_socket_fd = create_socket();

  /* bind a name to socket */
  bind_socket( server_socket_fd, port );

  /* establish listen queue */
  if( listen( server_socket_fd, SOMAXCONN ) == SOCKET_ERROR )
  {
    printf("\nCould not listen\n");
    return 0;
  }

  int optval = 1;
  setsockopt (server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

  while( 1 )
  {
    printf("\nWaiting for a connection\n");
    fflush(stdout);

    /* get the connected socket */
    socket_fd = accept( server_socket_fd, ( struct sockaddr* ) &Address, ( socklen_t * ) &addres_size );

    if( socket_fd < 0 )
    {
      printf( "\nERROR: could not accept connection\n" );
      fflush(stdout);
    }
    else
    {
      /* add new task to the task queue */
      push_task(socket_fd);

    }
  }

  printf( "\nClosing the socket" );
  /* close socket */
  if( close( socket_fd ) == SOCKET_ERROR )
  {
    printf( "\nCould not close socket\n" );
    return 0;
  }
}

void* serve(void * in_data)
{
  struct thread_info* t_info = ( struct thread_info* ) in_data;

  int tid = t_info->thread_id;

  printf("\nthread_id: %d", t_info->thread_id);
  fflush(stdout);

  while(1)
  {
    std::vector<char *> headers;
    std::string file_contents;
    int socket_fd = pop_task();
    int process_id;
    int content_length;
    struct stat filestat;
    const char * content_type;
    const char * status;
    char buffer[BUFFER_SIZE];
    char response_headers[1024];
    char request_type[10];
    char file_name[100];
    char HTTP_ver[10];

    GetHeaderLines( headers, socket_fd, false );

    printf("\n\n\njust got headers:\n\n");
    for ( int i = 0; i < headers.size( ); ++i )
    {
      printf("%s\n", headers[i]);            
    }
    fflush(stdout);

    read_request( headers[0], request_type, file_name, HTTP_ver );

    /* get path to file/dir */

    char file_path[strlen(directory_path) + strlen(file_name)];
    reset( file_path, strlen(file_path) );

    char file_name_cpy[ strlen( file_name ) ];
    strcpy( file_name_cpy, file_name );

    char* file_name_cstr = strtok(file_name_cpy, "?");
    char* env_args_cstr = strtok(NULL, "?");

    strcat( file_path, "." );
    strcat( file_path, directory_path );
    strcat( file_path, file_name_cstr );

    std::string file_path_str = file_path;

    content_length = get_content_length(headers);
    content_type = get_content_type( file_path );

    printf("content-type: %s request-type: %s\n", content_type, request_type );

    if( content_type == TYPE_CGI ) {

      printf("found cgi or perl file extension\n");
      fflush(stdout);

      /* pipe from cgi to server */
      int cgi_to_server_pipe[ 2 ];
      /* pipe from server to cgi */
      int server_to_cgi_pipe[ 2 ];

      if( pipe( server_to_cgi_pipe ) != 0 )
      {
        printf( "failed to create server to cgi pipe\n" );
      }

      if( pipe( cgi_to_server_pipe ) != 0 )
      {
        printf( "failed to create cgi to server pipe\n" );
      }

      process_id = fork();

      if( process_id == 0 )
      {
        /* we are in the child process */

        /* close the write side of the pipe from the server */
        close( server_to_cgi_pipe[ 1 ] );
        /* dup the pipe to stdin */
        dup2( server_to_cgi_pipe[ 0 ], 0 );
        /* close the read side of the pipe to the server */
        close( cgi_to_server_pipe[ 0 ] );
        /* dup the pipe to stdout */
        dup2( cgi_to_server_pipe[ 1 ], 1 );

        /* set up env variables */
        std::vector<char*> exec_envp;

        exec_envp.push_back( (char*) "GATEWAY_INTERFACE=CGI/1.1" );

        std::string req_uri = "REQUEST_URI=";
        req_uri.append( file_path );
        exec_envp.push_back( (char*) req_uri.c_str() );

        std::string req_method = "REQUEST_METHOD=";
        req_method.append( request_type );
        exec_envp.push_back( (char*) req_method.c_str() );

        std::string query_string = "QUERY_STRING=";
        
        if( strstr( request_type, REQ_TYPE_GET ) )
        {
          /* create query string header and add it to the array */
          std::string env_args = "";

          if( env_args_cstr != NULL )
          {
            env_args = env_args_cstr;
            query_string.append( env_args_cstr );
          }

        }

        exec_envp.push_back( (char*) query_string.c_str());

        /* turn headers into env vars */
        for ( int i = 1; i < headers.size( ); ++i )
        {
          char* header = headers[ i ];

          if( strstr( header, "Content-Type:" ) != NULL || strstr( header, "Content-Length:" ) != NULL )
          {
            exec_envp.push_back( FormatHeader( headers[ i ], (char*) "" ) );
          }
          else
          {
            exec_envp.push_back( FormatHeader( headers[ i ], (char*) "HTTP_" ) );
          }
          
        }

        /* null terminate the env vars array */
        exec_envp[ exec_envp.size() ] = (char*) NULL;
        
        std::vector<char*> exec_argv(1);

        exec_argv[ 0 ] = (char*) NULL;        
        
        /* execve */
        int err = execve( file_path_str.c_str(), &exec_argv[0], &exec_envp[0] );
        if(err == -1)
        {
          fprintf(stderr, "Errno: %d, means: %s", errno, strerror(errno));
        }

        printf("\n%s\n\n", "should not get here!!");
        fflush(stdout);

      }
      else
      {
        /* we are in the parent process */

        /* close the read side of the pipe to the CGI script */
        close( server_to_cgi_pipe[ 0 ] );
        /* close the write side of the pipe from the CGI script */
        close( cgi_to_server_pipe[ 1 ] );

        if( strstr( request_type, REQ_TYPE_POST ) )
        {
          /* we received a POST request */
          printf("\n%s\n\n", "found a post");
          fflush(stdout);

          int written_so_far = 0;
          int amount_written = 0;

          /* make env vars */

          if( content_length != -1 )
          {
            /* read body of post request */
            reset(buffer, strlen(buffer));
            read_content(socket_fd, buffer, content_length);
          }

          printf("\n\ncontents of POST:\n%s\npost size:%zu\n", buffer, strlen(buffer));
          fflush(stdout);

          /* send env vars to child process */
          send_over( server_to_cgi_pipe[ 1 ], buffer, content_length );

        }

        /* read file contents from the child process */
        file_contents = read_until( cgi_to_server_pipe[ 0 ] );

        /* make and serve headers */
        prepare_response_headers( file_contents.size( ), HTTP_OK, TYPE_HTML, response_headers );
        send_over( socket_fd, response_headers, strlen( response_headers ) );

        /* serve file contents */
        send_over( socket_fd, file_contents.c_str( ), file_contents.size( ) );

        /* all done, close the pipes */
        close(server_to_cgi_pipe[1]);
        close(cgi_to_server_pipe[0]);
      }

    }
    else
    {

      if( stat( file_path, &filestat ) ) {
        printf("\nERROR in stat 1\n");
        fflush(stdout);

        respond_with_not_found( socket_fd );

      }

      if(S_ISREG(filestat.st_mode)) {

        printf("\n%s is a regular file", file_name);
        printf("\nfile size = %lld", filestat.st_size);

        respond_with_file( socket_fd, file_path );

      }

      if(S_ISDIR(filestat.st_mode)) {
        printf("%s is a directory \n", file_name);

        DIR *dirp;
        struct dirent *dp;

        std::vector<std::string> files_in_dir;

        int found_index = 0;

        dirp = opendir( file_path );
        while ( ( dp = readdir( dirp ) ) != NULL )
        {
          printf( "\nname %s", dp->d_name );
          fflush( stdout );

          files_in_dir.push_back(dp->d_name);

          if( strstr( dp->d_name, "index.html" ) )
          {
            printf("\nfound index.html");
            fflush(stdout);

            strcat(file_path, "index.html");

            respond_with_file( socket_fd, file_path );

            found_index = 1;
          }
        }

        if(!found_index)
        {
          respond_with_listing( socket_fd, file_name, files_in_dir );
        }

        (void)closedir( dirp );
      }

    }

  }

  return 0;
}
