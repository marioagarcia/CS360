#include <string.h>
#include <vector>
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
#define TYPE_GIF          "image/gif"
#define TYPE_JPEG         "image/jpeg"
#define TYPE_CGI          "text/cgi"
#define MAX_MSG_SZ        1024
#define REQ_TYPE_GET      "GET"
#define REQ_TYPE_POST     "POST"

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
  else if(strstr(file_name, ".cgi") ||
          strstr(file_name, ".pl")  ||
          strstr(file_name, ".py"))
  {
    return TYPE_CGI;
  }
  else
  {
    printf("assertion!!\n");
    return "none";
  }
}

// read the file
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

int get_content_length(std::vector<char*> header_lines)
{
  int content_size = -1;

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

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

void to_array( std::vector<std::string> vec, char** ary )
{
  for (int i = 0; i < vec.size(); ++i)
  {
    ary[ i ] = (char*) vec[ i ].c_str();
  }
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

void reset(char* buffer, int size)
{
  memset( buffer, 0, sizeof( size ) );
}
