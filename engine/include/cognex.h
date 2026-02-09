#pragma once
#include <string>
#include <optional>
#include "types.h"
#include "db.h"

class Cognex: public DB,public Persistence {
public:
	explicit Cognex(WalPath wal_path,SnapshotPath snapshot_path);

	void put(Key key, Value value) override;
	std::optional<Value> get(const Key& key) const override;
	bool del(const Key& key) override;

	void recover() override;
	void snapshot() override;

private:
	WalPath wal_path_;
	SnapshotPath snapshot_path_;
};