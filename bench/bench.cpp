// #include <iostream>
// #include <chrono>
// #include <vector>
// #include <random>
// #include <algorithm>
// #include <fstream>
// #include <sstream>

// #include "cognex.h"

// using Clock = std::chrono::high_resolution_clock;

// static const size_t N = 10000;
// static const size_t QUERY_COUNT = 10000;

// std::vector<std::string> dictionary = {
//     "apple","banana","car","dog","elephant",
//     "school","database","engine","search",
//     "cloud","system","query","index","storage",
//     "network","latency","memory","disk","wal",
//     "performance","benchmark","random","value"
// };

// size_t memory_usage_mb()
// {
//     std::ifstream file("/proc/self/status");
//     std::string line;

//     while (std::getline(file, line))
//     {
//         if (line.find("VmRSS:") != std::string::npos)
//         {
//             std::string number;

//             for (char c : line)
//             {
//                 if (isdigit(c))
//                     number += c;
//             }

//             size_t kb = std::stoul(number);
//             return kb / 1024;
//         }
//     }

//     return 0;
// }

// std::string random_sentence(std::mt19937& rng)
// {
//     std::uniform_int_distribution<int> wordCountDist(3,12);
//     std::uniform_int_distribution<int> wordPick(0,dictionary.size()-1);

//     int wordCount = wordCountDist(rng);

//     std::string result;

//     for(int i=0;i<wordCount;i++)
//     {
//         result += dictionary[wordPick(rng)];

//         if(i != wordCount-1)
//             result += " ";
//     }

//     return result;
// }

// void benchmark_put(Cognex& db)
// {
//     std::mt19937 rng(42);

//     std::cout << "\n=== PUT Benchmark (Index Build) ===\n";

//     auto start = Clock::now();

//     for(size_t i=0;i<N;i++)
//     {
//         db.put(
//             Key{"key"+std::to_string(i)},
//             Value{random_sentence(rng)}
//         );
//     }

//     auto end = Clock::now();

//     double seconds =
//         std::chrono::duration<double>(end-start).count();

//     std::cout << "Documents indexed : " << N << "\n";
//     std::cout << "Index build time  : " << seconds << " sec\n";
//     std::cout << "Throughput        : "
//               << (N/seconds)
//               << " docs/sec\n";
// }

// void benchmark_get(Cognex& db)
// {
//     std::mt19937 rng(42);
//     std::uniform_int_distribution<size_t> dist(0,N-1);

//     std::cout << "\n=== GET Benchmark ===\n";

//     auto start = Clock::now();

//     for(size_t i=0;i<QUERY_COUNT;i++)
//     {
//         size_t k = dist(rng);

//         auto v = db.get(Key{"key"+std::to_string(k)});
//         (void)v;
//     }

//     auto end = Clock::now();

//     double seconds =
//         std::chrono::duration<double>(end-start).count();

//     std::cout << "Operations : " << QUERY_COUNT << "\n";
//     std::cout << "Total time : " << seconds << " sec\n";
//     std::cout << "Throughput : "
//               << (QUERY_COUNT/seconds)
//               << " ops/sec\n";
// }

// void benchmark_query(Cognex& db)
// {
//     std::mt19937 rng(42);
//     std::uniform_int_distribution<int> wordPick(0,dictionary.size()-1);

//     std::cout << "\n=== QUERY Benchmark ===\n";

//     auto start = Clock::now();

//     for(size_t i=0;i<QUERY_COUNT;i++)
//     {
//         std::string token = dictionary[wordPick(rng)];

//         Query q;
//         q.terms.push_back(token);
//         q.topK = 10;

//         auto r = db.query(q);
//         (void)r;
//     }

//     auto end = Clock::now();

//     double seconds =
//         std::chrono::duration<double>(end-start).count();

//     std::cout << "Queries     : " << QUERY_COUNT << "\n";
//     std::cout << "Total time  : " << seconds << " sec\n";
//     std::cout << "Throughput  : "
//               << (QUERY_COUNT/seconds)
//               << " queries/sec\n";
// }

// int main()
// {
//     std::cout << "Starting Cognex Benchmarks\n";

//     Cognex db(
//         WalPath{"wal.log"},
//         SnapshotPath{"snapshot.dat"},
//         ValueLogPath{"value.log"}
//     );

//     std::cout << "\nDataset target size: "
//               << N
//               << " documents\n";

//     benchmark_put(db);
//     benchmark_get(db);
//     benchmark_query(db);

//     std::cout << "\nMemory usage: "
//               << memory_usage_mb()
//               << " MB\n";

//     std::cout << "\nBenchmarks complete\n";

//     return 0;
// }