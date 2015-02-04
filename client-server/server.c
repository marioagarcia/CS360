#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
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
#define MAX_FILE_SIZE       126976

char * directory_path;
int thread_num;

#include "connection.c"
#include "templates.c"
#include "threading.c"
#include "queue.c"
#include "handler.c"
#include "utils.c"

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

  std::vector<char *> header_lines;

  int tid = t_info->thread_id;

  char buffer[BUFFER_SIZE];

  printf("\nthread_id: %d", t_info->thread_id);
  fflush(stdout);

  while(1)
  {
    int socket_fd = pop_task();

    printf("\nGot task: %d from the task queue", socket_fd);
    fflush(stdout);

    /* zero out buffer */
    memset( buffer, 0, sizeof( buffer ) );

    /* read request from socket into buffer */
    //read(socket_fd, buffer, BUFFER_SIZE);

    /* parse the request looking for the file/directory */
    char request_type[10];
    char file_path[100];
    char HTTP_ver[10];

    GetHeaderLines(header_lines, socket_fd, false);

    read_request( header_lines[0], request_type, file_path, HTTP_ver );


    /* get path to file/dir */
    char path_to_file[strlen(directory_path) + strlen(file_path)];
    memset( path_to_file, 0, sizeof( path_to_file ) );

    strcat(path_to_file, ".");
    strcat(path_to_file, directory_path);
    strcat(path_to_file, file_path);

    // prep_file_path(directory_path, file_path, path_to_file);

    printf("\npath to file: \"%s\"", path_to_file);
    fflush(stdout);

    struct stat filestat;
    int file_size = 0;
    char file_content[MAX_FILE_SIZE];
    const char * content_type;
    const char * status;

    std::string file_contents;

    if( stat( path_to_file, &filestat ) ) {
      printf("\nERROR in stat 1\n");
      fflush(stdout);

      file_size = 1297; //TODO make this dinamic

      prepare_file_content("./resources/not-found.html", file_size, file_content);

      content_type = TYPE_HTML;

      status = HTTP_NOT_FOUND;

    }

    if(S_ISREG(filestat.st_mode)) {

      printf("\n%s is a regular file", file_path);
      printf("\nfile size = %lld", filestat.st_size);

      file_size = filestat.st_size;

      // prepare_file_content(path_to_file, file_size, file_content);

      file_contents = get_file_content(path_to_file);

      content_type = get_content_type(path_to_file);

      status = HTTP_OK;

      printf("\nfile contents:\n%s", file_content);
      fflush(stdout);

    }

    if(S_ISDIR(filestat.st_mode)) {
      printf("%s is a directory \n", file_path);

      int len;
      DIR *dirp;
      struct dirent *dp;

      std::vector<std::string> files_in_dir;

      int found_index = 0;

      dirp = opendir( path_to_file );
      while ( ( dp = readdir( dirp ) ) != NULL )
      {
        printf( "\nname %s", dp->d_name );
        fflush( stdout );

        files_in_dir.push_back(dp->d_name);

        if( strstr( dp->d_name, "index.html" ) )
        {
          printf("\nfound index.html");
          fflush(stdout);

          strcat(path_to_file, "index.html");

          if( stat( path_to_file, &filestat ) ) {
            printf("\nERROR in stat 2\n");
            fflush(stdout);
          }

          file_size = filestat.st_size;

          printf("\nfile size = %d", file_size);

          memset( file_content, 0, sizeof( file_content ) );

          // prepare_file_content(path_to_file, file_size, file_content);

          file_contents = get_file_content(path_to_file);

          printf("\n file_content size below");
          fflush(stdout);

          printf("\n file_content size = %zu", strlen(file_content));
          fflush(stdout);

          content_type = get_content_type(path_to_file);

          status = HTTP_OK;

          printf("\nfile contents:\n\"%s\"", file_content);
          fflush(stdout);

          found_index = 1;
        }
      }

      if(!found_index)
      {
        file_contents = "<html>\n <head><meta http-equiv=\"Content-Type\"";
        file_contents += " content=\"text/html; charset=UTF-8\">";
        file_contents += "<title>Dir Listing for ";
        file_contents += path_to_file;
        file_contents += "</title><style type=\"text/css\"></style></head>";
        file_contents += "<body bgcolor=\"#FFFFFF\" text=\"#000000\"><h2>";
        file_contents += path_to_file;
        file_contents += "</h2><ol>";

        for(int i = 0; i < files_in_dir.size(); i++)
        {
          file_contents += "<li><a href=\"";
          if(files_in_dir[i] == ".")
          {
            file_contents += "#";
          }
          else
          {
            file_contents += file_path;
            file_contents += "/";
            file_contents += files_in_dir[i];
          }
          file_contents += "\"</a>";
          file_contents += files_in_dir[i];
          file_contents += "</li>";
        }

        file_contents += "</ol></body></html>";

        file_size = file_contents.size();

        content_type = TYPE_HTML;

        status = HTTP_OK;
      }

      (void)closedir( dirp );
    }

    printf("\nafter dir call");
    fflush(stdout);

    /* make response headers */
    char response_headers[ BUFFER_SIZE ];

    prepare_response_headers( file_size, status, content_type, response_headers);

    printf("\nresponse headers:\n%s", response_headers);
    fflush(stdout);

    /* send response headers */

    send_over( socket_fd, response_headers, strlen(response_headers) );

    printf("\nsent response headers");
    fflush(stdout);

    /* send file content */

    send_over( socket_fd, file_contents.c_str(), file_size );

    printf("\nsent file content");
    fflush(stdout);

  }

  return 0;
}
