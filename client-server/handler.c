#include <signal.h>

void sig_handler(int signo)
{
  if (signo == SIGINT )
  {
    printf("received SIGINT signal\n");
    fflush(stdout);

    sem_close(task_queue_sem);
    sem_close(task_avail_sem);
    sem_close(q_space_sem);

    kill_all_threads( thread_num );

    exit(1);
  }

  if (signo == SIGSEGV )
  {

    printf("received SIGSEGV signal\n");
    fflush(stdout);

    sem_close(task_queue_sem);
    sem_close(task_avail_sem);
    sem_close(q_space_sem);

    kill_all_threads( thread_num );

    exit(1);
  }
}

void read_request( char* request, char* request_type, char* file_path, char* HTTP_ver )
{
  if(!strlen(request))
  {
    printf("No request received");
    exit(1);
  }
  else
  {
    printf("\nfirst header: %s", request );
    fflush(stdout);
    sscanf( request, "%s %s %s", request_type, file_path, HTTP_ver );
  }
}

void read_content(int socket_fd, char * buffer, int content_size)
{
  int total_read = 0;
  int amount_read = 0;
  int done_reading = 0;

  while(!done_reading)
  {
    amount_read = read(socket_fd, buffer + total_read, content_size - total_read);

    total_read += amount_read;

    if(total_read >= content_size)
    {
      done_reading = 1;
    }
  }
}

// write out to a socket
void send_over(int socket_fd, const char * content, int file_size )
{
  int amount_sent;
  int total_sent = 0;
  int done_writing = 0;

  while(!done_writing)
  {
    amount_sent = write(socket_fd, content + total_sent, file_size - total_sent);

    printf("after the write amount_sent: %d total_sent: %d\n", amount_sent, total_sent);
    fflush(stdout);

    total_sent += amount_sent;

    if(total_sent >= file_size)
    {
      done_writing = 1;
    }
  }
}

void respond_with_not_found( int socket_fd ) {

  std::string file_contents = get_file_content( "./resources/not-found.html" );

  int file_size = file_contents.size();

  const char* content_type = TYPE_HTML;

  const char* status = HTTP_NOT_FOUND;

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

void respond_with_file( int socket_fd, char* file_path )
{
  printf("\nrespond_with_file");
  fflush(stdout);

  printf( "\n%s is a regular file", file_path );

  std::string file_contents = get_file_content( file_path );

  std::string content_type = get_content_type( file_path );

  int file_size = file_contents.size();

  std::string status = HTTP_OK;

  printf("\nfile contents:\n%s", file_contents.c_str());
  fflush(stdout);

  /* make response headers */
  char response_headers[ BUFFER_SIZE ];

  prepare_response_headers( file_size, status.c_str(), content_type.c_str(), response_headers );

  printf("\nresponse headers:\n%s", response_headers );
  fflush(stdout);

  /* send response headers */

  send_over( socket_fd, response_headers, strlen( response_headers ) );

  printf("\nsent response headers");
  fflush(stdout);

  /* send file content */

  send_over( socket_fd, file_contents.c_str(), file_size );

  printf("\nsent file content");
  fflush(stdout);
}

void respond_with_listing( int socket_fd, char* file_path, std::vector<std::string> files_in_dir )
{
  std::string file_contents = create_listing_page( file_path, files_in_dir);

  std::string content_type = get_content_type( file_path );

  printf("\n file_content size below");
  fflush(stdout);

  printf("\n file_content size = %zu", file_contents.size());
  fflush(stdout);

  std::string status = HTTP_OK;

  printf("\nfile contents:\n\"%s\"", file_contents.c_str());
  fflush(stdout);

  /* make response headers */
  char response_headers[ BUFFER_SIZE ];

  prepare_response_headers( file_contents.size(), status.c_str(), content_type.c_str(), response_headers );

  printf("\nresponse headers:\n%s", response_headers );
  fflush(stdout);

  /* send response headers */

  send_over( socket_fd, response_headers, strlen( response_headers ) );

  printf("\nsent response headers");
  fflush(stdout);

  /* send file content */

  send_over( socket_fd, file_contents.c_str(), file_contents.size() );

  printf("\nsent file content");
  fflush(stdout);
}

int send_content_to_pipe(int pipe_fd, char* content, int content_length)
{
  return 0;
}

std::string read_until( int pipe_fd )
{
  char buffer[BUFFER_SIZE];
  std::string content = "";
  int done_reading = 0;
  int amount_read = 0;
  int total_read = 0;
  reset(buffer, strlen(buffer));

  while( ( amount_read = read( pipe_fd, buffer + total_read, BUFFER_SIZE ) ) != 0 )
  {
    total_read += amount_read;
  }

  buffer[total_read] = '\0';
  content.append(buffer);

  return content;

}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;

    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
        if (amtread > 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}

// Get the header lines from a socket
//   envformat = false when getting a request from a web client
//   envformat = true when getting lines from a CGI program

void GetHeaderLines(std::vector<char *> &headerLines, int skt, bool envformat)
{
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;

    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        if (strstr(tline, "Content-Length") || strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, (char *) "");
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat)
                line = FormatHeader(tline, (char *) "");
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "%s", tline);
            }
        }
        //fprintf(stderr, "Header --> [%s]\n", line);

        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}










