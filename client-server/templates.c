
#include <string>

std::string create_listing_page( char * file_name, std::vector<std::string> files_in_dir )
{
    std::string file_contents;

	  file_contents = "<html>\n <head><meta http-equiv=\"Content-Type\"";
    file_contents += " content=\"text/html; charset=UTF-8\">";
    file_contents += "<title>Dir Listing for ";
    file_contents += file_name;
    file_contents += "</title><style type=\"text/css\"></style></head>";
    file_contents += "<body bgcolor=\"#FFFFFF\" text=\"#000000\"><h2>";
    file_contents += file_name;
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
        file_contents += file_name;
        file_contents += "/";
        file_contents += files_in_dir[i];
      }
      file_contents += "\"</a>";
      file_contents += files_in_dir[i];
      file_contents += "</li>";
    }

    file_contents += "</ol></body></html>";

    return file_contents;
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