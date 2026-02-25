#pragma once
#include <string>
#include <optional>
#include "types.h"
#include "db.h"
#include <unordered_map>
#include <vector>
#include <string_view>
#include <functional>
#include <fstream>



class Cognex: public DB,public Persistence {
public:
	explicit Cognex(WalPath wal_path,SnapshotPath snapshot_path, ValueLogPath valuelog_path);

	void put(Key key, Value value) override;
	std::optional<Value> get(const Key& key) const override;
	bool del(const Key& key) override;
	const std::vector<size_t>& query(std::string_view token) const;
	uint64_t append_to_value_log_(uint32_t keySize, std::string_view value);
	std::optional<Value> read_from_log_(uint64_t offset, uint32_t keySize) const;

    //------------Recovery-----------	

	void recover() override;
	void snapshot() override;

private:
	WalPath wal_path_;
	SnapshotPath snapshot_path_;
	ValueLogPath valuelog_path_;

	//------------Main DB----------- 

	// std::unordered_map<Key,Value> store_;
	std::unordered_map<Key, IndexEntry> index_;

	//------------Secondary Storages-----------

	std::vector<Entry> entries_;
	//stores tokenIndex[token]->"Entry.entryId"
	std::unordered_map<std::string,std::vector<size_t>,TransparentHash,TransparentEqual> tokenIndex_;
	std::vector<Posting> postings_;

	//------ file ------
	int valueLogFd_ = -1;


	//------------Helpers-----------

	void apply_record_(const std::string_view& record);
    std::vector<std::string> generate_tokens_update_tokenIndex_(std::string_view value, size_t entryId);
    void push_entry_(Key key, Value value);
    void open_value_log_if_needed_();
};