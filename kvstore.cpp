#include "kvstore.h"

// Constructor: store the capacity limit
LRUKVStore::LRUKVStore(size_t capacity) : capacity_(capacity)
{
    // 'explicit' in the header prevents accidental implicit conversions
    // like: LRUKVStore db = 5;  ← this would be confusing, explicit blocks it
}

// GET key
std::optional<std::string> LRUKVStore::get(const std::string &key)
{
    auto it = map_.find(key);

    if (it == map_.end())
    {
        return std::nullopt; // Key doesn't exist
    }

    // it->second is the list iterator pointing to this key's node
    // Move the node to front — this key was just used
    moveToFront(it->second);

    // After moving, the node is now at the front
    // list_.front() gives us the {key, value} pair
    return list_.front().second; // .second = the value
}

// SET key value
void LRUKVStore::set(const std::string &key, const std::string &value)
{
    auto it = map_.find(key);

    if (it != map_.end())
    {
        // Key already exists: update value and move to front
        it->second->second = value; // update the value in the list node
        moveToFront(it->second);
        return;
    }

    // New key: check if we're at capacity
    if (map_.size() >= capacity_)
    {
        evictLRU(); // Remove least recently used entry
    }

    // Insert new {key, value} at the FRONT of the list
    list_.emplace_front(key, value);

    // Store the iterator to this new front node in the map
    // list_.begin() points to the front node we just inserted
    map_[key] = list_.begin();
}

// DELETE key
bool LRUKVStore::del(const std::string &key)
{
    auto it = map_.find(key);

    if (it == map_.end())
    {
        return false; // Key doesn't exist
    }

    // Remove from list using the stored iterator — O(1)
    list_.erase(it->second);

    // Remove from map
    map_.erase(it);

    return true;
}

// Returns current number of stored entries
size_t LRUKVStore::size() const
{
    return map_.size();
}

// moveToFront: splice the node to the front of the list
// std::list::splice moves a node WITHOUT copying or reallocating.
// It just rewires the prev/next pointers — true O(1).
// Crucially, the iterator stored in map_ stays valid after splice.
void LRUKVStore::moveToFront(std::list<KeyValuePair>::iterator it)
{
    // splice(destination, source_list, source_iterator)
    // Moves the element at 'it' to position 'list_.begin()' (the front)
    list_.splice(list_.begin(), list_, it);
}

// evictLRU: remove the tail node (least recently used)
void LRUKVStore::evictLRU()
{
    // The back of the list is the least recently used entry
    auto lru = list_.back(); // copy the {key, value} pair
    list_.pop_back();        // remove from list
    map_.erase(lru.first);   // remove from map using the key
    // lru.first = key, lru.second = value
}