#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include "cognex.h"

using Clock = std::chrono::high_resolution_clock;

static const int N = 1'000'000;

std::string number_to_spaced_digits(int n) {
    std::string s = std::to_string(n);
    std::string result;

    for (char c : s) {
        result += c;
        result += ' ';
    }

    if (!result.empty())
        result.pop_back(); // remove trailing space

    return result;
}

void run_put_benchmark(Cognex& db) {
    std::cout << "\n=== PUT Benchmark ===\n";

    auto start = Clock::now();

    for (int i = 0; i < N; i++) {

        std::string value = number_to_spaced_digits(i);

        db.put(
            Key{"key" + std::to_string(i)},
            Value{value}
        );
    }

    auto end = Clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double seconds = duration.count() / 1000.0;

    std::cout << "Ops: " << N << "\n";
    std::cout << "Time: " << seconds << " sec\n";
    std::cout << "Throughput: " << (N / seconds) << " ops/sec\n";
}


void run_get_benchmark(Cognex& db) {
    std::cout << "\n=== GET Benchmark ===\n";

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, N - 1);

    auto start = Clock::now();

    for (int i = 0; i < N; i++) {
        int k = dist(rng);
        db.get(Key{"key" + std::to_string(k)});
    }

    auto end = Clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double seconds = duration.count() / 1000.0;

    std::cout << "Ops: " << N << "\n";
    std::cout << "Time: " << seconds << " sec\n";
    std::cout << "Throughput: " << (N / seconds) << " ops/sec\n";
}

void run_mixed_benchmark(Cognex& db) {
    std::cout << "\n=== Mixed Benchmark (50% PUT / 50% GET) ===\n";

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, N - 1);

    auto start = Clock::now();

    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            db.put(Key{"key" + std::to_string(i)},
               Value{"value" + std::to_string(i)});
        } else {
            int k = dist(rng);
           db.get(Key{"key" + std::to_string(k)});
        }
    }

    auto end = Clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double seconds = duration.count() / 1000.0;

    std::cout << "Ops: " << N << "\n";
    std::cout << "Time: " << seconds << " sec\n";
    std::cout << "Throughput: " << (N / seconds) << " ops/sec\n";
}
void run_query_benchmark(Cognex& db) {
    std::cout << "\n=== QUERY Benchmark ===\n";

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> digit_dist(0, 9);

    auto start = Clock::now();

    for (int i = 0; i < 10; i++) {

        int digit = digit_dist(rng);

        db.query(std::to_string(digit));
    }

    auto end = Clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    double seconds = duration.count() / 1000.0;

    std::cout << "Ops: " << N << "\n";
    std::cout << "Time: " << seconds << " sec\n";
    std::cout << "Throughput: " << (N / seconds) << " ops/sec\n";
}



int main() {
    Cognex db(WalPath{"wal.log"},SnapshotPath{"snapshot.dat"});

    std::cout << "Starting Cognex Benchmarks...\n";

    run_put_benchmark(db);
    run_get_benchmark(db);
    run_mixed_benchmark(db);
    run_query_benchmark(db);
    std::cout << "\nBenchmarks complete.\n";
    return 0;
}
