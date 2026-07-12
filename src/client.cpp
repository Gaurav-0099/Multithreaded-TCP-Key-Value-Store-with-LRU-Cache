#include <iostream>
#include <sys/socket.h> //socket() ,connect()
#include <arpa/inet.h>  //inet_pton() convert IPv4 and IPv6 addresses from text to binary form
#include <unistd.h>     //close()
#include <string>
#include <netinet/in.h> //sockaddr_in
#include <cstring>      //memset()

const int port = 7777;
const int buffer_size = 1024;

int main()
{
    // step 1: create socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // step 2: connect to server
    //  Dial the server at 127.0.0.1:7777
    //  127.0.0.1 = localhost = "this same machine"
    sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address)); // zero out the struct
    server_address.sin_family = AF_INET;                // IPv4
    server_address.sin_port = htons(port);              // port in network byte order

    // inet_pton: "presentation to network"
    // converts the string "127.0.0.1" into the binary format the OS needs
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/ Address not supported\n";
        return 1;
    }

    if (connect(sock_fd, (sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        std::cerr << "ERROR: connect() failed. Is the server running?\n";
        return 1;
    }

    std::cout << "Connected to server on Port " << port << "\n";
    std::cout << "Commands: SET key value | GET key | DELETE key | quit\n\n";

    // step 3 : send and receive data loop

    std::string command;
    char buffer[buffer_size];

    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, command); // read full line including spaces

        if (command == "quit" || command == "exit")
        {
            std::cout << "Exiting...\n";
            break;
        }
        if (command.empty())
        {
            continue;
        }
        std ::string command_to_send = command + "\n";                     // append newline to signal end of command
        send(sock_fd, command_to_send.c_str(), command_to_send.size(), 0); // send command to server

        // wait for response from server
        memset(buffer, 0, buffer_size);                             // clear buffer
        int bytes_read = recv(sock_fd, buffer, buffer_size - 1, 0); // read response from server

        if (bytes_read <= 0)
        {
            std::cout << "Server closed the connection\n";
            break;
        }
        std::cout << buffer; // print server response
    }

    close(sock_fd); // close the socket
    return 0;
}
