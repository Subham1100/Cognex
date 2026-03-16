#pragma once

#include "core/types.h"
#include "core/utils.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>

class IndexEngine
{
public:

    void insert(Key key,
                Value value,
                std::vector<Entry>& entries_,
                std::unordered_map<std::string,
                                   std::vector<Posting>,
                                   TransparentHash,
                                   TransparentEqual>& postings_, size_t& totalTokens_) const;

private:

    std::vector<std::string> tokenize_and_update(
        std::string_view value,
        size_t entryId,
        std::unordered_map<std::string,
                           std::vector<Posting>,
                           TransparentHash,
                           TransparentEqual>& postings_) const;
};