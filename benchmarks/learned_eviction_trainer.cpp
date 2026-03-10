#include "observer/access_observer.h"
#include "cache/eviction_policy.h"
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <unordered_set>
#include <vector>

using namespace aqa;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << (argc >= 1 ? argv[0] : "learned_eviction_trainer")
                  << " <trace_file> <output_policy_file> [ring_cap=256] [cache_cap=64] [feedback_horizon=64] [lr=0.05]\n";
        return 1;
    }
    const std::string trace_path(argv[1]);
    const std::string output_path(argv[2]);
    size_t ring_cap = (argc > 3) ? static_cast<size_t>(std::stoull(argv[3])) : 256;
    size_t cache_cap = (argc > 4) ? static_cast<size_t>(std::stoull(argv[4])) : 64;
    size_t feedback_horizon = (argc > 5) ? static_cast<size_t>(std::stoull(argv[5])) : 64;
    double lr = (argc > 6) ? std::stod(argv[6]) : 0.05;

    std::ifstream trace(trace_path);
    if (!trace) {
        std::cerr << "Failed to open trace file: " << trace_path << "\n";
        return 1;
    }

    AccessObserver observer(ring_cap);
    LearnedPageEvictionPolicy policy(&observer, feedback_horizon, lr);
    std::list<uint32_t> cache_lru;  /* front = MRU, back = LRU */
    std::unordered_set<uint32_t> in_cache;

    uint32_t page_id;
    size_t accesses = 0;
    while (trace >> page_id) {
        bool hit = (in_cache.count(page_id) != 0);
        observer.record_page_access(page_id, hit);

        if (hit) {
            cache_lru.remove(page_id);
            cache_lru.push_front(page_id);
        } else {
            if (cache_lru.size() >= cache_cap) {
                std::vector<uint32_t> unpinned(cache_lru.rbegin(), cache_lru.rend());
                uint32_t victim = policy.choose_victim(unpinned);
                cache_lru.remove(victim);
                in_cache.erase(victim);
            }
            cache_lru.push_front(page_id);
            in_cache.insert(page_id);
        }
        ++accesses;
    }

    double wr, wc, ws;
    policy.get_weights(wr, wc, ws);
    std::ofstream out(output_path);
    if (!out) {
        std::cerr << "Failed to open output file: " << output_path << "\n";
        return 1;
    }
    out << "# Learned eviction weights (trace accesses=" << accesses << ")\n";
    out << wr << " " << wc << " " << ws << "\n";
    std::cout << "Wrote policy to " << output_path << " (accesses=" << accesses << ", w_recency=" << wr << " w_count=" << wc << " w_scan=" << ws << ")\n";
    return 0;
}
