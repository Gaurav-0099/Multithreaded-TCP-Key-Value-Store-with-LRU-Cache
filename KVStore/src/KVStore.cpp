#include "KVstore.h"

//set key value
//override if already there
void KVStore::set(const std::string &key, const std::string &value)
{
    store_[key] = value;
}

//get key value
//if key does not exists return "nothing"
std::optional<std::string> KVStore::get(const std::string &key)
{
    auto it = store_.find(key);
    if (it == store_.end())
    {
        return std::nullopt;
    }
    return it->second;
    // we use find instead of directly getting the value from key because unordered map generates a 
    //zero value if key is not there
}

//delete key
bool KVStore::del(const std::string&key){
    //erase returns number of elements removed 0 or 1 in case of map so we use it as bool
    return store_.erase(key);
}
