#pragma once // tells the compiler to add it only once

#include <string>
#include <unordered_map>
#include <optional>
#include <list>
// LRU KVStore-kv store with fixed capacity
// removes least used element if it overflows the capacity
// list to keep track of time front->recent and back->older ones
// avg O(1) for get set and evict
class LRUKVStore
{
public:
    explicit LRUKVStore(size_t capaxity);

    void set(const std::string &key, const std::string &value);

    std::optional<std::string> get(const std::string &key);

    bool del(const std::string &key);

    size_t size() const; // returns current number of entries

private:
    size_t capacity_;

    // The ordered list: each element is a {key, value} pair.
    // Front = most recently used, Back = least recently used.
    // We store the KEY in the list so we can remove it from the
    // map when we evict the tail node.
    using KeyValuePair = std::pair<std::string, std::string>;
    std::list<KeyValuePair> list_;

    // Maps key → iterator pointing to that key's node in list_.
    // std::list iterators stay valid even after other nodes are
    // moved or removed — this is the property that makes LRU work.
    std::unordered_map<std::string, std::list<KeyValuePair>::iterator> map_;

    // Move an existing node to the front of the list (O(1)).
    // Called on every GET and SET to mark the key as recently used.
    void moveToFront(std::list<KeyValuePair>::iterator it);

    // Evict the least recently used entry (the tail node).
    // Called by set() when capacity is reached.
    void evictLRU();
};
