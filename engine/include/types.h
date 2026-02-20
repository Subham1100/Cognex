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

struct ParsedCommand {
    std::string name;
    std::vector<std::string> args;
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

    explicit Entry(size_t id_,Key key_, Value value_, std::vector<std::string>tokens_)
    :id(id_),
    key(std::move(key_)),
    value(std::move(value_)),
    tokens(std::move(tokens_)){}
};

struct Posting
{
    size_t entryId;
    size_t frequency;
    std::vector<size_t> tokenPositions;

    explicit Posting(size_t entryId_, size_t frequency_, std::vector<size_t> tokenPositions_)
    :entryId(entryId_),
    frequency(frequency_),
    tokenPositions(tokenPositions_) {}
};

struct TransparentHash {
    using is_transparent = void;

    size_t operator()(std::string_view sv) const {
        return std::hash<std::string_view>{}(sv);
    }

    size_t operator()(const std::string& s) const {
        return std::hash<std::string_view>{}(s);
    }
};

struct TransparentEqual {
    using is_transparent = void;

    template<typename T, typename U>
    bool operator()(const T& a, const U& b) const noexcept {
        return a == b;
    }
};



struct Index
{
    size_t offset;
    size_t valueSize;
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