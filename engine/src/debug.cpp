#include "debug.h"

void debug_posting_array(std::vector<Posting>& postings)
{
	for(const auto& posting : postings)
	{
		std::cout<<posting.entryId<<"\n"<<posting.frequency<<"\n";
		std::cout<<"Token position array: ";
		for(const auto& position : posting.tokenPositions )
		{
			std::cout<<position<<" ";
		}
		std::cout << "----------------\n";
		std::cout<<"\n\n";
	}
}

