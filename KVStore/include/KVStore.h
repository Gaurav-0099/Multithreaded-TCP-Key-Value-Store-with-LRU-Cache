#pragma once // tells the compiler to add it only once

#include <string>
#include <unordered_map>
#include <optional>

class KVStore
{
public:
    void set(const std::string &key, const std::string &value);
    std::optional<std::string> get(const std::string &key);
    bool del(const std::string &key);

private:
    std::unordered_map<std::string, std::string> store_;
};
