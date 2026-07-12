#include "kvstore.h"
#include <iostream>
#include <sstream>
#include <string>
#include <thread> //for multithreading
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

const int PORT = 7777;

const int buffer_size = 1024;

std::string parseAndExecute(const std::string &command, LRUKVStore &store)
{
    // std::istringstream lets us treat a string like std::cin
    // so we can extract words one by one with >>

    std::istringstream iss(command);
    std::string op;
    iss >> op; // first word is the operation

    if (op == "SET")
    {
        std::string key, value;
        iss >> key >> value;
        if (key.empty() || value.empty())
        {
            return "ERROR: SET requires a key and a value";
        }
        store.set(key, value);
        return "OK";
    }
    else if (op == "GET")
    {
        std::string key;
        iss >> key;
        if (key.empty())
        {
            return "ERROR: GET requires a key\n";
        }
        auto result = store.get(key);
        return result.has_value() ? result.value() : "NOT FOUND\n";
    }
    else if (op == "DELETE")
    {
        std::string key;
        iss >> key;
        if (key.empty())
        {
            return "ERROR: DELETE requires a key\n";
        }
        return store.del(key) ? "OK" : "NOT FOUND\n";
    }
    return "ERROR: Unknown command. Use SET / GET / DELETE\n";
}

// handleClient: runs in its own thread for each connected client.
// Takes ownership of client_fd — responsible for closing it.
//
// Why pass store by reference?
// All threads share the SAME KVStore — that's the whole point.
// One database, many clients accessing it simultaneously.
// (We'll make this safe with shared_mutex on Day 5.)
void handleClient(int client_fd, LRUKVStore &store)
{
    std::cout << "Thread " << std::this_thread::get_id()
              << " handling client (fd=" << client_fd << ")\n";

    char buffer[buffer_size];

    while (true)
    {
        memset(buffer, 0, buffer_size);
        int bytes_read = recv(client_fd, buffer, buffer_size - 1, 0);

        if (bytes_read <= 0)
        {
            std::cout << "Client disconnected (fd=" << client_fd << ")\n";
            break;
        }

        std::string command(buffer);
        if (!command.empty() && command.back() == '\n')
            command.pop_back();
        if (!command.empty() && command.back() == '\r')
            command.pop_back();

        std::cout << "[Thread " << std::this_thread::get_id()
                  << "] Received: " << command << "\n";

        std::string response = parseAndExecute(command, store);
        response += "\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }

    close(client_fd);
    // Thread finishes here and cleans itself up (because we detach it)
}

int main()
{
    LRUKVStore store(100); // capacity of 100 entries
    // step 1: create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        std::cerr << "ERROR IN CREATING SOCKET\n";
        return 1;
    }
    // SO_REUSEADDR: lets us restart the server immediately without
    // waiting ~60 seconds for the OS to release the port.
    // Without this, you'd get "Address already in use" on restart.
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // step 2: bind socket to port
    sockaddr_in address;
    memset(&address, 0, sizeof(address)); // zero out the struct
    address.sin_family = AF_INET;         // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // accept connections on any network interface

    address.sin_port = htons(PORT); // htons = "host to network short"
                                    // converts port to network byte order

    if (bind(server_fd, (sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "ERROR: bind() failed\n";
        return 1;
    }

    // step 3: listen for incoming connections
    //  Marks the socket as passive — ready to accept connections.
    //  The second argument (5) is the backlog: how many pending
    //  connections to queue while we're busy handling another.
    //  ─────────────────────────────────────────────
    if (listen(server_fd, 5) < 0)
    {
        std::cerr << "ERROR: listen() failed\n";
        return 1;
    }
    std::cout << "Multithreaded server listening on port " << PORT << "...\n";
    // main loop: accept connections and handle them one at a time
    while (true)
    {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        // accept() still runs in main thread — waits for new connections
        int client_fd = accept(server_fd, (sockaddr *)&client_address, &client_len);
        if (client_fd < 0)
        {
            std::cerr << "ERROR: accept() failed\n";
            continue;
        }

        std::cout << "New client connected (fd=" << client_fd
                  << ") — spawning thread\n";

        // Spawn a new thread for this client.
        // The thread runs handleClient(client_fd, store) independently.
        // main() immediately loops back to accept() — ready for the next client.
        std::thread t(handleClient, client_fd, std::ref(store));
        // std::ref(store) — pass store by reference to the thread.
        // Without std::ref, std::thread would try to COPY store — wrong.

        // detach(): let the thread run independently.
        // main() won't wait for it. Thread cleans up when handleClient returns.
        t.detach();

        // At this point main() is already back at accept(), waiting for
        // the next client — while the detached thread handles the current one.
    }
    close(server_fd);
    return 0;
}