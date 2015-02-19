#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "utils.c"
#include "handler.c"

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         1024
#define HOST_NAME_SIZE      255
#define URL_SIZE            255
#define REQUEST_SIZE        255

int create_socket()
{
  // printf("\nMaking a socket");
  int h_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if(h_socket == SOCKET_ERROR)
  {
    printf("\nCould not make a socket\n");
    exit(0);
  }

  return h_socket;
}

void close_socket(int h_socket)
{
  // printf("\nClosing socket\n");
  if(close(h_socket) == SOCKET_ERROR)
  {
    printf("\nCould not close socket\n");
    exit(0);
  }
}

void connectToHost(int socket, int nHostPort, struct hostent* pHostInfo)
{
  long nHostAddress;
  struct sockaddr_in Address;  /* Internet socket address stuct */

  if(pHostInfo == NULL)
  {
    printf("\nServer not found\n");
    exit(0);
  }

  /* copy address into long */
  memcpy(&nHostAddress, pHostInfo->h_addr, pHostInfo->h_length);

  /* fill address struct */
  Address.sin_addr.s_addr = nHostAddress;
  Address.sin_port = htons(nHostPort);
  Address.sin_family = AF_INET;

  /* connect to host */
  if(connect(socket, (struct sockaddr*)&Address, sizeof(Address)) == SOCKET_ERROR)
  {
    printf("\nCould not connect to host\n");
    exit(0);
  }
}

void printHeaderLines(std::vector<char *> header_lines)
{
  puts("\nHTTP Headers:\n");

  for(int header_index = 0; header_index < header_lines.size(); header_index++)
  {
    puts(header_lines[header_index]);
  }
}

int getContentSize(std::vector<char *> header_lines)
{
  int content_size;

  for(int header_index = 0; header_index < header_lines.size(); header_index++)
  {
    const char* content_len_str = "Content-Length: ";
    char* ch_ptr;

    if((ch_ptr = strstr(header_lines[header_index], content_len_str)) > 0)
    {
      content_size = atoi(header_lines[header_index] + strlen(content_len_str));
    }
  }

  return content_size;
}

int  main(int argc, char* argv[])
{
  int hSocket;                 /* handle to socket */
  struct hostent* pHostInfo;   /* holds info about a machine */
  long nHostAddress;
  char pBuffer[BUFFER_SIZE];
  unsigned nReadAmount;
  char strHostName[HOST_NAME_SIZE];
  char url[URL_SIZE];
  char http_get_request[REQUEST_SIZE];
  std::vector<char *> header_lines;
  int nHostPort;
  int option;
  int arg_index = 0;
  int debug_flag = 0;
  int repeat_flag = 1;
  int done_reading = 0;
  int total_read = 0;
  int content_size = 0;
  int successful_downloads = 0;

  //using getopt for parsing cmd line args
  while ((option = getopt(argc, argv,"dc:")) != -1) {
    switch (option) {
      case 'd' : debug_flag = 1; break;
      case 'c' : repeat_flag = atoi(optarg); break;
      default :
      printClientUsage();
      return 0;
    }

    arg_index = optind;
  }

  if((argc - arg_index) < 3)
  {
    printClientUsage();
    return 0;
  }
  else
  {
    if(arg_index == 0)
      arg_index++;

      strcpy(strHostName, argv[arg_index++]);
      nHostPort = atoi(argv[arg_index++]);
      strcpy(url, argv[arg_index]);
    }

    for(int rep = 0; rep < repeat_flag; rep++)
    {
      /* reset all cached items */
      reset(pBuffer, BUFFER_SIZE);
      header_lines.clear();

      /* make a socket */
      hSocket = create_socket();

      /* get IP address from name */
      pHostInfo = gethostbyname(strHostName);

      /* connect to host */
      connectToHost(hSocket, nHostPort, pHostInfo);

      /* make the http command */
      prepareGetCommand(http_get_request, url, strHostName);

      /* print out the http GET request for debug purposes*/
      if(debug_flag)
      {
        printf("\n\nHTTP GET Request:\n\n%s", http_get_request);
      }

      /* write the http request to the server */
      write(hSocket, http_get_request, strlen(http_get_request) + 1);

      GetHeaderLines(header_lines, hSocket, false);

      if(debug_flag)
      {
        printHeaderLines(header_lines);
      }

      /* get content size from headers */
      content_size = getContentSize(header_lines);

      /* read from socket into buffer */
      read_content(hSocket, pBuffer, content_size);

      /* print out the HTTP Response for debug purposes*/
      if(debug_flag)
      {
        printf("\n\nContent:\n\n%s", pBuffer);
      }

      /* close socket */
      close_socket(hSocket);

      successful_downloads++;

    }

    printf("\nSuccessful downloads: %d\n\n", successful_downloads);
  }
