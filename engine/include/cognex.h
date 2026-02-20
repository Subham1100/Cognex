#pragma once
#include <string>
#include <optional>
#include "types.h"
#include "db.h"
#include <unordered_map>
#include <vector>
#include <string_view>
#include <functional>



class Cognex: public DB,public Persistence {
public:
	explicit Cognex(WalPath wal_path,SnapshotPath snapshot_path);

	void put(Key key, Value value) override;
	std::optional<Value> get(const Key& key) const override;
	bool del(const Key& key) override;
	std::vector<size_t> query(const std::string_view& token) const;

    //------------Recovery-----------	

	void recover() override;
	void snapshot() override;

private:
	WalPath wal_path_;
	SnapshotPath snapshot_path_;

	//------------Main DB----------- 

	std::unordered_map<Key,Value> store_;

	//------------Secondary Storages-----------

	std::vector<Entry> entries_;
	std::unordered_map<std::string,std::vector<Posting>,TransparentHash,TransparentEqual> tokenIndex_;

	//------------Helpers-----------

	void apply_record_(const std::string_view& record);
    std::vector<std::string> generate_tokens_update_tokenIndex_(std::string_view value, size_t entryId);
    void push_entry_(Key key, Value value);
};