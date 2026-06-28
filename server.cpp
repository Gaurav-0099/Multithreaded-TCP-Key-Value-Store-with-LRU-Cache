#include "kvstore.h"
#include <iostream>
#include <sstream>
#include <string>
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
            return "ERROR: GET requires a key";
        }
        auto result = store.get(key);
        return result.has_value() ? result.value() : "NOT FOUND";
    }
    else if (op == "DELETE")
    {
        std::string key;
        iss >> key;
        if (key.empty())
        {
            return "ERROR: DELETE requires a key";
        }
        return store.del(key) ? "OK" : "NOT FOUND";
    }
    return "ERROR: Unknown command. Use SET / GET / DELETE";
}
