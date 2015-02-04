#include <string.h>
#include <vector>             // stl vector
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string>
#include <fstream>

#define HTTP_OK           "200 OK"
#define HTTP_NOT_FOUND    "404 Not Found"
#define TYPE_TEXT         "text/plain"
#define TYPE_HTML         "text/html"
#define TYPE_GIF         "image/gif"
#define TYPE_JPEG         "image/jpeg"
#define MAX_MSG_SZ        1024

// get content type from file extension
const char * get_content_type(const char * file_name )
{
  if( strstr( file_name, ".txt" ) )
  {
    return TYPE_TEXT;
  }
  else if(strstr(file_name, ".html" ) )
  {
    return TYPE_HTML;
  }
  else if(strstr(file_name, ".gif"))
  {
    return TYPE_GIF;
  }
  else if(strstr(file_name, ".jpg"))
  {
    return TYPE_JPEG;
  }
  else
  {
    printf("assertion!!");
    return "none";
  }
}

//prepare response headers
void prepare_response_headers(int file_size, const char * status, const char* content_type, char * response_headers)
{
  if(strstr(status, HTTP_OK))
  {
    snprintf( response_headers,
              255,
              "HTTP/1.0 %s\r\nContent-Type:%s\r\nContent-Length:%d\r\n\r\n",
              HTTP_OK,
              content_type,
              file_size);
  }
  else
  {
    snprintf( response_headers, 24, "HTTP/1.0 %s\r\n\r\n", HTTP_NOT_FOUND );
  }
}

// read the file
void prepare_file_content( const char* path_to_file, int file_size, char* file_content )
{
  FILE* file;
  int amount_read;
  int total_read = 0;
  int done_reading = 0;

  file = fopen(path_to_file, "rb");

  printf("\nafter the fopen\npath_to_file: %s", path_to_file);
  fflush(stdout);

  while(!done_reading)
  {
    amount_read = fread(file_content + total_read, 1, file_size - total_read, file);

    printf("\nafter the fread, amount_read: %d total_read: %d", amount_read, total_read);
    fflush(stdout);

    total_read += amount_read;

    if(total_read >= file_size)
    {
      printf("\ndone reading, amount_read: %d total_read: %d file_content_size: %zu", amount_read, total_read, strlen(file_content));
      fflush(stdout);
      done_reading = 1;
    }
  }

  strcat(file_content, "\r\n\r\n");

}

std::string get_file_content( const char * filename )
{
  std::ifstream in( filename, std::ios::in | std::ios::binary );
  if( in )
  {
    std::string contents;
    in.seekg( 0, std::ios::end );
    contents.resize( in.tellg());
    in.seekg( 0, std::ios::beg );
    in.read( &contents[0], contents.size() );
    in.close();
    return( contents );
  }
  else
  {
    printf( "ERROR: could not read all the file contents from disk");
    return "";
  }
}

// combine the path and the file name and add a .
// char* prep_file_path(char* directory_path, char* file_path, char* path_to_file )
// {
//
// }

// write out to a socket
void send_over(int socket_fd, const char * content, int file_size )
{
  int amount_sent;
  int total_sent = 0;
  int done_writing = 0;

  while(!done_writing)
  {
    amount_sent = write(socket_fd, content + total_sent, file_size - total_sent);

    printf("\nafter the write amount_sent: %d total_sent: %d", amount_sent, total_sent);
    fflush(stdout);

    total_sent += amount_sent;

    if(total_sent >= file_size)
    {
      done_writing = 1;
    }
  }
}

// Determine if the character is whitespace
bool isWhitespace(char c)
{
    switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
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

// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;

    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');

        if (s[i] == '-')
            s[i] = '_';
    }

}


// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char* FormatHeader(char *str, char *prefix)
{
    char* result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 2;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
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
        if (strstr(tline, "Content-Length") ||
                strstr(tline, "Content-Type"))
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

//TODO: make this specific for the client
void printClientUsage()
{
    printf("\nusage: client [-d] [-c <# of times>] host port URL\n");
}

void printServerUsage()
{
    printf("\nusage: sever [-t <# of threads>] port directory\n");
}

void prepareGetCommand(char* cmd, char* url, char* host)
{
  snprintf(cmd, 255, "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n", url, host);
}

void append(char* s, char c)
{
  s[strlen(s)] = c;
  s[strlen(s) + 1 ] = '\0';
}

void reset(char * buffer, int size)
{
  for(int i = 0; i < size; i++)
  {
    buffer[i] = '\0';
  }
}
