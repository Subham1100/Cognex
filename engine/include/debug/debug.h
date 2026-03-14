#pragma once 
#include <iostream>
#include "core/types.h"

#ifndef COGNEX_DEBUG
#define COGNEX_DEBUG 0
#endif

#if COGNEX_DEBUG
    #define DBG(x) do { std::cout << x << std::endl; } while(0)
#else
    #define DBG(x) do {} while(0)
#endif


void debug_posting_array(std::vector<Posting>& postings);
