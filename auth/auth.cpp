// CS370 Auction Site
// Authentication CGI Program
// Author: scotttennison

#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <mysql/mysql.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using namespace std;

// Converts URL encoded text back to normal text
// Example: "hello+world" becomes "hello world"
// Example: "test%40email.com" becomes "test@email.com"
string urlDecode(const string &encoded)
{
    string decoded;
    for (int i = 0; i < encoded.length(); i++)
    {
        if (encoded[i] == '+')
        {
            // + means a space
            decoded += ' ';
        }
        else if (encoded[i] == '%' && i + 2 < encoded.length())
        {
            // % followed by two hex characters means a special character
            string hexStr = encoded.substr(i + 1, 2);
            char decodedChar = (char)strtol(hexStr.c_str(), nullptr, 16);
            decoded += decodedChar;
            i += 2;
        }
        else
        {
            decoded += encoded[i];
        }
    }
    return decoded;
}

// This function reads and parses the form data
map<string, string> parseFormData(const string &data)
{
    // Create an empty container to hold our key-value pairs
    map<string, string> formData;

    // Now split the data into individual pieces
    // Example: "action=login&email=scott@email.com&password=abc"
    string key, value;
    bool readingKey = true;

    for (char c : data)
    {
        if (c == '=')
        {
            // Switch from reading key to reading value
            readingKey = false;
        }
        else if (c == '&')
        {
            // Save this pair with URL decoding and start a new one
            formData[urlDecode(key)] = urlDecode(value);
            key.clear();
            value.clear();
            readingKey = true;
        }
        else
        {
            // Add character to either key or value
            if (readingKey)
            {
                key += c;
            }
            else
            {
                value += c;
            }
        }
    }

    // Save the last pair (no & at the end)
    if (!key.empty())
    {
        formData[urlDecode(key)] = urlDecode(value);
    }

    return formData;
}

// This function takes a password and returns a scrambled version
string hashPassword(const string &password)
{
    // Create a container for the scrambled result
    // SHA256 always produces exactly 32 bytes
    unsigned char hash[SHA256_DIGEST_LENGTH];

    // Run the password through the blender
    SHA256(reinterpret_cast<const unsigned char *>(password.c_str()),
           password.length(),
           hash);

    // Convert the scrambled bytes into readable text
    // Like translating binary into hex characters
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }

    return ss.str();
}

// This function connects to the MySQL database
MYSQL *connectToDatabase()
{
    // Create a MySQL connection object
    // Think of this like picking up the phone
    MYSQL *conn = mysql_init(nullptr);

    // If the phone is broken, stop here
    if (conn == nullptr)
    {
        cout << "<p>Error: MySQL initialization failed</p>" << endl;
        return nullptr;
    }

    // Try to dial the number and connect
    // mysql_real_connect needs:
    // connection, host, user, password, database, port, socket, flags
    if (mysql_real_connect(conn,
                           "localhost",
                           "root",
                           "",
                           "auction_site",
                           0,
                           nullptr,
                           0) == nullptr)
    {
        // If connection failed, tell us why
        cout << "<p>Database connection failed: "
             << mysql_error(conn) << "</p>" << endl;
        mysql_close(conn);
        return nullptr;
    }

    // Connection worked! Return it so we can use it
    return conn;
}

// This function registers a new user
void registerUser(MYSQL *conn, const string &username, const string &firstName, const string &lastName, const string &email, const string &password)
{
    // Escape all user inputs to prevent SQL injection
    char safeEmail[512], safeUsername[512], safeFirst[512], safeLast[512];
    mysql_real_escape_string(conn, safeEmail,    email.c_str(),     email.length());
    mysql_real_escape_string(conn, safeUsername, username.c_str(),  username.length());
    mysql_real_escape_string(conn, safeFirst,    firstName.c_str(), firstName.length());
    mysql_real_escape_string(conn, safeLast,     lastName.c_str(),  lastName.length());

    // Step 1: Check if email already exists in database
    string checkQuery = string("SELECT user_id FROM users WHERE email = '") + safeEmail + "'";

    // Send the query to MySQL
    if (mysql_query(conn, checkQuery.c_str()))
    {
        cout << "<p>Error checking email: "
             << mysql_error(conn) << "</p>" << endl;
        return;
    }

    // Get the results back from MySQL
    MYSQL_RES *result = mysql_store_result(conn);

    // Count how many rows came back
    int numRows = mysql_num_rows(result);

    // Free the result from memory
    mysql_free_result(result);

    // Step 2: If email already exists, tell the user
    if (numRows > 0)
    {
        cout << "<p>Error: That email address is already registered.</p>" << endl;
        cout << "<p><a href=\"/login.html\">Click here to login</a></p>" << endl;
        return;
    }

    // Step 3: Hash the password before saving
    string hashedPassword = hashPassword(password);

    // Step 4: Build the INSERT query to save the new user
    string insertQuery = string("INSERT INTO users (username, first_name, last_name, email, password_hash, is_active) VALUES ('") +
                         safeUsername + "', '" +
                         safeFirst    + "', '" +
                         safeLast     + "', '" +
                         safeEmail    + "', '" +
                         hashedPassword + "', 1)";

    // Step 5: Run the INSERT query
    if (mysql_query(conn, insertQuery.c_str()))
    {
        cout << "<p>Error registering user: "
             << mysql_error(conn) << "</p>" << endl;
        return;
    }

    // Step 6: Tell the user it worked!
    cout << "<h2>Registration Successful!</h2>" << endl;
    cout << "<p>Welcome! Your account has been created.</p>" << endl;
    cout << "<p><a href=\"/login.html\">Click here to login</a></p>" << endl;
}

// This function logs in an existing user
void loginUser(MYSQL *conn, const string &email, const string &password)
{
    // Step 1: Hash the password they typed in
    // We need to compare it to what's stored in the database
    string hashedPassword = hashPassword(password);

    // Escape user input to prevent SQL injection
    char safeEmail[512];
    mysql_real_escape_string(conn, safeEmail, email.c_str(), email.length());

    // Step 2: Build a query to find this user
    string query = string("SELECT user_id, email FROM users WHERE email = '") +
                   safeEmail + "' AND password_hash = '" + hashedPassword + "'";

    // Step 3: Send the query to MySQL
    if (mysql_query(conn, query.c_str()))
    {
        cout << "<p>Error during login: "
             << mysql_error(conn) << "</p>" << endl;
        return;
    }

    // Step 4: Get the results back
    MYSQL_RES *result = mysql_store_result(conn);

    // Step 5: Count how many rows came back
    int numRows = mysql_num_rows(result);

    // Step 6: Free the result from memory
    mysql_free_result(result);

    // Step 7: If we got a row back, login was successful
    if (numRows == 1)
    {
        cout << "<h2>Login Successful!</h2>" << endl;
        cout << "<p>Welcome back, " << email << "!</p>" << endl;
        cout << "<p><a href=\"/index.html\">Go to Dashboard</a></p>" << endl;
    }
    else
    {
        // No matching row means wrong email or password
        cout << "<h2>Login Failed</h2>" << endl;
        cout << "<p>Invalid email or password.</p>" << endl;
        cout << "<p><a href=\"/login.html\">Try again</a></p>" << endl;
    }
}

int main()
{
    // Tell the browser we are sending HTML
    cout << "Content-Type: text/html\n\n";

    // Read raw data from cin ONE time only
    string rawData;
    char *rawLength = getenv("CONTENT_LENGTH");
    if (rawLength != nullptr)
    {
        int len = atoi(rawLength);
        rawData.resize(len);
        cin.read(&rawData[0], len);
    }

    // Pass raw data to parser
    map<string, string> formData = parseFormData(rawData);

    // Get the individual values from the form
    string action    = formData["action"];
    string email     = formData["email"];
    string password  = formData["password"];
    string username  = formData["username"];
    string firstName = formData["first_name"];
    string lastName  = formData["last_name"];

    // Print HTML page start
    cout << "<!DOCTYPE html>" << endl;
    cout << "<html lang=\"en\">" << endl;
    cout << "<head>" << endl;
    cout << "    <meta charset=\"UTF-8\">" << endl;
    cout << "    <title>Auction Site</title>" << endl;
    cout << "</head>" << endl;
    cout << "<body>" << endl;

    // Connect to database
    MYSQL *conn = connectToDatabase();

    // If connection failed stop here
    if (conn == nullptr)
    {
        cout << "</body></html>" << endl;
        return 1;
    }

    // Decide what to do based on which button was clicked
    if (action == "register")
    {
        registerUser(conn, username, firstName, lastName, email, password);
    }
    else if (action == "login")
    {
        loginUser(conn, email, password);
    }
    else
    {
        cout << "<p>Unknown action</p>" << endl;
    }

    // Always close the connection when done
    mysql_close(conn);

    cout << "</body>" << endl;
    cout << "</html>" << endl;

    return 0;
}
