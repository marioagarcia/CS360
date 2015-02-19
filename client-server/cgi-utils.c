// credit to http://stackoverflow.com/users/140311/marco-m for this algorithm

std::vector<std::string> split( std::string & str, const char* delimiters ) {
    std::vector<std::string> tokens;
    int start = 0;
    auto pos = str.find_first_of( delimiters, start );
    
    while( pos != std::string::npos ) {
        if( pos != start ) // ignore empty tokens
            tokens.emplace_back( str, start, pos - start );
        start = pos + 1;
        pos = str.find_first_of( delimiters, start );
    }
    
    if( start < str.length( ) ) // ignore trailing delimiter
        tokens.emplace_back( str, start, str.length() - start ); // add what's left of the string

    return tokens;
}