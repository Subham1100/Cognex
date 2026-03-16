#include "index/index_engine.h"


std::vector<std::string> IndexEngine::tokenize_and_update(std::string_view value, size_t entryId, std::unordered_map<std::string,
                           std::vector<Posting>,
                           TransparentHash,
                           TransparentEqual>& postings_) const
{
    std::vector<std::string> tokenVector;
    size_t startPointer = 0;
    size_t valueLength = value.length();
    size_t tokenNumber = 0;

    while(startPointer<valueLength)
    {
        while (startPointer < valueLength && value[startPointer] == ' ')
        startPointer++;
        if (startPointer >= valueLength)
        break;
        size_t endPointer = value.find(' ',startPointer);
        if(endPointer == std::string_view::npos)
            endPointer=value.length();
        std::string token = clean_token(value.substr(startPointer, endPointer - startPointer));

        if(!token.empty())
        {
            
            // find the token in tokenIndex
            auto& postingList = postings_[token]; 

            // Try to find existing posting for this entryId
            bool found = false;
            for (auto& posting : postingList) {
                if (posting.entryId == entryId) {
                    posting.frequency++;
                    posting.tokenPositions.push_back(tokenNumber);
                    found = true;
                    break;
                }
            }
           
           // If not found, add new posting
            if (!found) {
                postingList.emplace_back(
                    entryId,
                    1,
                    std::vector<size_t>{tokenNumber}
                );
            }

            tokenVector.push_back(std::move(token));
            tokenNumber++;

        }
        startPointer = endPointer + 1;
    }
    
    return tokenVector;
}

void IndexEngine::insert(
    Key key,
    Value value,
    std::vector<Entry>& entries_,
    std::unordered_map<std::string,
                       std::vector<Posting>,
                       TransparentHash,
                       TransparentEqual>& postings_,size_t& totalTokens_) const
{
    size_t currId = entries_.size();

    auto tokens = tokenize_and_update(value.value, currId, postings_ );
    totalTokens_+=tokens.size();
    entries_.emplace_back(currId, key, value, std::move(tokens));
  
}