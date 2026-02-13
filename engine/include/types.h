#pragma once
#include <string>
#include <utility>
#include <functional>
#include <cstddef>

// ---------- paths ----------

struct WalPath
{
    std::string value;
    explicit WalPath(std::string v) : value(std::move (v)){}
};

struct SnapshotPath
{
    std::string value;
    explicit SnapshotPath(std::string v): value(std::move (v)){}
};

// ---------- db data ----------

struct Key
{
    std::string value;
    explicit Key(std::string v) : value(std::move (v)) {}

    bool operator==(const Key& other) const {
        return value == other.value;
    }
};

struct Value
{
    std::string value;
    explicit Value(std::string v): value(std::move (v)) {}
};


struct Entry
{
    size_t id;
    Key key;
    Value value;
    std::vector<std::string> tokens;

    explicit Entry(ssize_t id_,Key key_, Value value_, std::vector<std::string>tokens_)
    :id(id_),
    key(std::move(key_)),
    value(std::move(value_)),
    tokens(std::move(tokens_)){}
};

// ---------- Hash specialization ----------

namespace std {
    template<>
    struct hash<Key> {
        size_t operator()(const Key& k) const noexcept {
            return hash<std::string>{}(k.value);
        }
    };
}