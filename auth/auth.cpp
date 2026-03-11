// CS370 Auction Site
// Authentication CGI Program
// Author: scotttennison

#include <iostream>
#include <string>
#include <cstdlib>
#include <mysql/mysql.h>

using namespace std;

int main()
{

    // Tell the browser we are sending HTML
    cout << "Content-Type: text/html\n\n";

    // Print a basic HTML page
    cout << "<!DOCTYPE html>" << endl;
    cout << "<html lang=\"en\">" << endl;
    cout << "<head>" << endl;
    cout << "    <meta charset=\"UTF-8\">" << endl;
    cout << "    <title>Auction Site</title>" << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;
    cout << "    <h1>Auth program is working!</h1>" << endl;
    cout << "</body>" << endl;
    cout << "</html>" << endl;

    return 0;
}