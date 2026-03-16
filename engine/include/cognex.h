#pragma once
#include <string>
#include <optional>
#include "core/types.h"
#include "core/db.h"
#include "query/query_engine.h"
#include "index/index_engine.h"
#include "storage/storage_engine.h"
#include "storage/wal.h"
#include "storage/snapshot.h"
#include "debug/debug.h"
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
	// std::vector<size_t> query(std::string_view token) const;
	std::vector<QueryResult> query(const Query& query) const;
	// uint64_t append_to_value_log_(uint32_t keySize, std::string_view value);
	// std::optional<Value> read_from_log_(uint64_t offset, uint32_t keySize) const;
	// void generate_candidates(QueryContext& ctx) const;
	// void apply_filters(QueryContext& ctx) const;
	// void rank_results(QueryContext& ctx) const;
	// void apply_topk(QueryContext& ctx) const;
	const Entry& get_entry(size_t entryId) const;

    //------------Recovery-----------	

	void recover() override;
	void snapshot() override;

private:
	// WalPath wal_path_;
	// SnapshotPath snapshot_path_;
	// ValueLogPath valuelog_path_;

	//------------Main DB----------- 

	// std::unordered_map<Key,Value(offset)> store_;
	std::unordered_map<Key, IndexEntry> index_;

	//------------Secondary Storages-----------

	std::vector<Entry> entries_;
	// //tokenIndex_ tokenIndex[token]->vector<"Entry.entryId"> forward indexing
	// std::unordered_map<std::string,std::vector<size_t>,TransparentHash,TransparentEqual> tokenIndex_;
	// posting_ posting[token]->vector<"posting"> //inverted indexing
	std::unordered_map<std::string,std::vector<Posting>,TransparentHash,TransparentEqual> postings_;

	// //------ file ------
	// int valueLogFd_ = -1;

	// // //------ state vairables-----
	// size_t walWrites_ = 0;
	// size_t walFsyncEveryNWrites_ = 1000;

	// size_t valueLogWrites_ = 0;
	// size_t valueLogFsyncEveryNWrites_ = 5000;

	size_t snapshotWriteOps_ = 0;
	size_t snapshotEveryNWriteOps_ = 10000000;

	size_t totalTokens_ = 0;

	//------------Helpers-----------

	void apply_record_(const std::string_view& record);
    // std::vector<std::string> generate_tokens_update_tokenIndex_(std::string_view value, size_t entryId);
    // void push_entry_(Key key, Value value);
    // void open_value_log_if_needed_();

    //---query engine-----
    QueryEngine queryEngine_;
    IndexEngine indexEngine_;
    StorageEngine storageEngine_;
};