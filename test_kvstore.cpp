#include <iostream>
#include <cassert>
#include "kvstore.h"

// Each test creates its own LRUKVStore with a small capacity
// so we can trigger eviction easily without storing millions of keys.

void test_basic_set_and_get()
{
    LRUKVStore db(3); // capacity = 3

    db.set("name", "Alice");
    db.set("age", "25");

    auto name = db.get("name");
    assert(name.has_value() && name.value() == "Alice");

    auto age = db.get("age");
    assert(age.has_value() && age.value() == "25");

    std::cout << "[PASS] basic set and get\n";
}

void test_overwrite()
{
    LRUKVStore db(3);

    db.set("city", "Delhi");
    db.set("city", "Mumbai"); // overwrite

    auto result = db.get("city");
    assert(result.has_value() && result.value() == "Mumbai");

    // Size should still be 1, not 2
    assert(db.size() == 1);

    std::cout << "[PASS] overwrite\n";
}

void test_eviction_order()
{
    LRUKVStore db(3); // capacity = 3

    // Fill to capacity
    db.set("a", "1"); // order: [a]
    db.set("b", "2"); // order: [b, a]
    db.set("c", "3"); // order: [c, b, a]  ← 'a' is LRU

    // Now access 'a' — it becomes most recently used
    db.get("a"); // order: [a, c, b]  ← 'b' is now LRU

    // Add a 4th key — cache is full, 'b' should be evicted
    db.set("d", "4"); // evicts 'b', order: [d, a, c]

    assert(!db.get("b").has_value() && "'b' should have been evicted");
    assert(db.get("a").has_value() && "'a' should still exist");
    assert(db.get("c").has_value() && "'c' should still exist");
    assert(db.get("d").has_value() && "'d' should exist");

    std::cout << "[PASS] eviction order\n";
}

void test_lru_evicts_oldest_unused()
{
    LRUKVStore db(2); // capacity = 2

    db.set("x", "10"); // order: [x]
    db.set("y", "20"); // order: [y, x]  ← 'x' is LRU

    // Don't touch 'x' — add 'z', 'x' should be evicted
    db.set("z", "30"); // evicts 'x', order: [z, y]

    assert(!db.get("x").has_value() && "'x' should be evicted (oldest unused)");
    assert(db.get("y").has_value() && "'y' should still exist");
    assert(db.get("z").has_value() && "'z' should exist");

    std::cout << "[PASS] LRU evicts oldest unused\n";
}

void test_delete()
{
    LRUKVStore db(3);

    db.set("token", "abc");
    bool deleted = db.del("token");

    assert(deleted && "del should return true for existing key");
    assert(!db.get("token").has_value() && "key should be gone after del");
    assert(db.size() == 0);

    std::cout << "[PASS] delete\n";
}

void test_capacity_respected()
{
    LRUKVStore db(3);

    db.set("a", "1");
    db.set("b", "2");
    db.set("c", "3");
    db.set("d", "4"); // triggers eviction
    db.set("e", "5"); // triggers eviction

    // Size should never exceed capacity
    assert(db.size() <= 3 && "size should never exceed capacity");

    std::cout << "[PASS] capacity respected\n";
}

int main()
{
    std::cout << "=== Day 2: LRU Cache Tests ===\n\n";

    test_basic_set_and_get();
    test_overwrite();
    test_eviction_order();
    test_lru_evicts_oldest_unused();
    test_delete();
    test_capacity_respected();

    std::cout << "\nAll tests passed!\n";
    return 0;
}