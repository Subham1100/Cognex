#pragma once
#include <optional>
#include "types.h"

struct DB
{
	virtual ~DB() = default;

	virtual void put(Key key,Value value) = 0;
	virtual std::optional<Value> get(const Key& key) const = 0;
	virtual bool del(const Key& key) = 0;
};

struct Persistence {
    virtual ~Persistence() = default;
    
    virtual void recover() = 0;
    virtual void snapshot() = 0;
};
