#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#if defined(__APPLE__)
#include <mach/mach.h>
#endif

#include "cognex.h"

using Clock = std::chrono::high_resolution_clock;

static const std::vector<std::string> DICTIONARY = {
    "apple",       "banana",   "car",       "dog",       "elephant", "school",
    "database",    "engine",   "search",    "cloud",     "system",   "query",
    "index",       "storage",  "network",   "latency",   "memory",   "disk",
    "wal",         "snapshot", "performance", "benchmark", "random",   "value"
};

struct BenchConfig {
    size_t docs = 10000;
    size_t queryCount = 10000;
    uint32_t seed = 42;
    std::string dataDir = "bench_data";
    bool fresh = true;
};

struct BenchResult {
    double putSeconds = 0.0;
    double getSeconds = 0.0;
    double querySeconds = 0.0;
};

size_t memory_usage_mb()
{
#if defined(__linux__)
    std::ifstream file("/proc/self/status");
    std::string line;
    while (std::getline(file, line))
    {
        if (line.find("VmRSS:") == std::string::npos)
            continue;

        std::string number;
        for (char c : line)
        {
            if (std::isdigit(static_cast<unsigned char>(c)))
                number += c;
        }
        if (number.empty())
            return 0;
        return std::stoul(number) / 1024;
    }
    return 0;
#elif defined(__APPLE__)
    mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    kern_return_t status = task_info(
        mach_task_self(),
        MACH_TASK_BASIC_INFO,
        reinterpret_cast<task_info_t>(&info),
        &count);
    if (status != KERN_SUCCESS)
        return 0;
    return static_cast<size_t>(info.resident_size / (1024 * 1024));
#else
    return 0;
#endif
}

std::string random_sentence(std::mt19937& rng)
{
    std::uniform_int_distribution<int> wordCountDist(3, 12);
    std::uniform_int_distribution<int> wordPick(0, static_cast<int>(DICTIONARY.size()) - 1);

    int wordCount = wordCountDist(rng);
    std::string result;
    result.reserve(static_cast<size_t>(wordCount) * 8);

    for (int i = 0; i < wordCount; ++i)
    {
        result += DICTIONARY[static_cast<size_t>(wordPick(rng))];
        if (i + 1 != wordCount)
            result += " ";
    }

    return result;
}

double benchmark_put(Cognex& db, size_t docs, std::mt19937& rng)
{
    auto start = Clock::now();
    for (size_t i = 0; i < docs; ++i)
    {
        db.put(Key{"key" + std::to_string(i)}, Value{random_sentence(rng)});
    }
    auto end = Clock::now();
    return std::chrono::duration<double>(end - start).count();
}

double benchmark_get(Cognex& db, size_t docs, size_t queryCount, std::mt19937& rng)
{
    std::uniform_int_distribution<size_t> dist(0, docs - 1);
    auto start = Clock::now();
    for (size_t i = 0; i < queryCount; ++i)
    {
        (void)db.get(Key{"key" + std::to_string(dist(rng))});
    }
    auto end = Clock::now();
    return std::chrono::duration<double>(end - start).count();
}

double benchmark_query(Cognex& db, size_t queryCount, std::mt19937& rng)
{
    std::uniform_int_distribution<int> wordPick(0, static_cast<int>(DICTIONARY.size()) - 1);
    auto start = Clock::now();
    for (size_t i = 0; i < queryCount; ++i)
    {
        Query q;
        q.terms.push_back(DICTIONARY[static_cast<size_t>(wordPick(rng))]);
        q.topK = 10;
        (void)db.query(q);
    }
    auto end = Clock::now();
    return std::chrono::duration<double>(end - start).count();
}

BenchConfig parse_args(int argc, char** argv)
{
    BenchConfig cfg;
    if (argc > 1)
        cfg.docs = std::stoull(argv[1]);
    if (argc > 2)
        cfg.queryCount = std::stoull(argv[2]);
    if (argc > 3)
        cfg.seed = static_cast<uint32_t>(std::stoul(argv[3]));
    if (argc > 4)
        cfg.dataDir = argv[4];
    return cfg;
}

void reset_data_files(const std::filesystem::path& base)
{
    std::filesystem::create_directories(base);
    std::error_code ec;
    std::filesystem::remove(base / "wal.log", ec);
    std::filesystem::remove(base / "snapshot.dat", ec);
    std::filesystem::remove(base / "value.log", ec);
}

int main(int argc, char** argv)
{
    BenchConfig cfg = parse_args(argc, argv);
    std::filesystem::path base = cfg.dataDir;

    if (cfg.fresh)
        reset_data_files(base);
    else
        std::filesystem::create_directories(base);

    Cognex db(
        WalPath{(base / "wal.log").string()},
        SnapshotPath{(base / "snapshot.dat").string()},
        ValueLogPath{(base / "value.log").string()});
    db.recover();

    std::mt19937 rng(cfg.seed);
    BenchResult result;
    result.putSeconds = benchmark_put(db, cfg.docs, rng);
    result.getSeconds = benchmark_get(db, cfg.docs, cfg.queryCount, rng);
    result.querySeconds = benchmark_query(db, cfg.queryCount, rng);

    const double total = result.putSeconds + result.getSeconds + result.querySeconds;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Cognex benchmark\n";
    std::cout << "docs=" << cfg.docs
              << " query_count=" << cfg.queryCount
              << " seed=" << cfg.seed
              << " dir=" << base.string() << "\n\n";

    std::cout << "PUT   time=" << result.putSeconds
              << "s throughput=" << (cfg.docs / result.putSeconds) << " docs/s\n";
    std::cout << "GET   time=" << result.getSeconds
              << "s throughput=" << (cfg.queryCount / result.getSeconds) << " ops/s\n";
    std::cout << "QUERY time=" << result.querySeconds
              << "s throughput=" << (cfg.queryCount / result.querySeconds) << " queries/s\n\n";

    std::cout << "Time split:\n";
    std::cout << "  PUT   " << (result.putSeconds / total) * 100.0 << "%\n";
    std::cout << "  GET   " << (result.getSeconds / total) * 100.0 << "%\n";
    std::cout << "  QUERY " << (result.querySeconds / total) * 100.0 << "%\n";
    std::cout << "Memory (RSS approx): " << memory_usage_mb() << " MB\n";

    return 0;
}