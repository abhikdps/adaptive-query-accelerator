#include "observer/access_observer.h"
#include "db/database.h"
#include "storage/storage_engine.h"
#include "storage/mapped_file.h"
#include "storage/page_cache.h"
#include <iostream>
#include <filesystem>
#include <cassert>
#include <vector>

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        std::cerr << "FAIL: " << #a << " (" << (a) << ") != " << #b << " (" << (b) << ")" << std::endl; \
        exit(1); \
    }

#define ASSERT_TRUE(c) \
    if (!(c)) { \
        std::cerr << "FAIL: " << #c << " is false" << std::endl; \
        exit(1); \
    }

#define ASSERT_FALSE(c) ASSERT_TRUE(!(c))

void test_observer_direct_page_cache() {
    std::string path = "access_observer_direct.db";
    std::filesystem::remove(path);
    aqa::AccessObserver obs(16);
    {
        aqa::MappedFile file(path);
        file.grow_file(2);
        aqa::PageCache cache(file, 4, &obs);
        auto h0 = cache.fetch_page(0);
        auto h1 = cache.fetch_page(1);
        (void)h0;
        (void)h1;
    }
    ASSERT_EQ(obs.get_total_recorded(), 2u);
    auto ids = obs.get_recent_page_ids(4);
    ASSERT_EQ(ids.size(), 2u);
    ASSERT_EQ(ids[0], 0u);
    ASSERT_EQ(ids[1], 1u);
    std::filesystem::remove(path);
}

void test_record_and_retrieve() {
    aqa::AccessObserver obs(10);

    obs.record_page_access(1, false);
    obs.record_page_access(2, true);
    obs.record_page_access(3, false);

    ASSERT_EQ(obs.get_size(), 3u);
    ASSERT_EQ(obs.get_total_recorded(), 3u);

    auto events = obs.get_recent_page_accesses(5);
    ASSERT_EQ(events.size(), 3u);
    ASSERT_EQ(events[0].page_id, 1u);
    ASSERT_FALSE(events[0].is_hit);
    ASSERT_EQ(events[1].page_id, 2u);
    ASSERT_TRUE(events[1].is_hit);
    ASSERT_EQ(events[2].page_id, 3u);
    ASSERT_FALSE(events[2].is_hit);

    auto ids = obs.get_recent_page_ids(2);
    ASSERT_EQ(ids.size(), 2u);
    ASSERT_EQ(ids[0], 1u);
    ASSERT_EQ(ids[1], 2u);
}

void test_ring_wraparound() {
    aqa::AccessObserver obs(4);

    obs.record_page_access(10, false);
    obs.record_page_access(20, true);
    obs.record_page_access(30, false);
    obs.record_page_access(40, true);
    ASSERT_EQ(obs.get_size(), 4u);

    obs.record_page_access(50, false);
    obs.record_page_access(60, true);
    ASSERT_EQ(obs.get_size(), 4u);
    ASSERT_EQ(obs.get_total_recorded(), 6u);

    auto events = obs.get_recent_page_accesses(4);
    ASSERT_EQ(events.size(), 4u);
    ASSERT_EQ(events[0].page_id, 30u);
    ASSERT_EQ(events[1].page_id, 40u);
    ASSERT_EQ(events[2].page_id, 50u);
    ASSERT_EQ(events[3].page_id, 60u);
}

void test_record_access_from_rid() {
    aqa::AccessObserver obs(5);
    obs.record_record_access(aqa::RecordID{100, 2});
    obs.record_record_access(aqa::RecordID{101, 0});

    auto ids = obs.get_recent_page_ids(5);
    ASSERT_EQ(ids.size(), 2u);
    ASSERT_EQ(ids[0], 100u);
    ASSERT_EQ(ids[1], 101u);
}

void test_request_more_than_stored() {
    aqa::AccessObserver obs(20);
    obs.record_page_access(1, true);
    obs.record_page_access(2, false);

    auto events = obs.get_recent_page_accesses(100);
    ASSERT_EQ(events.size(), 2u);
}

void test_null_observer_no_crash() {
    std::string path = "access_observer_null.db";
    std::filesystem::remove(path);
    {
        aqa::Database db(path, 10, 100, nullptr);
        std::vector<uint8_t> key = {'k', '1'};
        std::vector<uint8_t> val = {'v', '1'};
        db.put(key, val);
        auto res = db.get(key);
        ASSERT_TRUE(res.has_value());
    }
    std::filesystem::remove(path);
}

void test_observer_receives_page_accesses_via_database() {
    std::string path = "access_observer_db.db";
    std::filesystem::remove(path);
    if (std::filesystem::exists(path + ".wal")) {
        std::filesystem::remove(path + ".wal");
    }

    std::vector<uint8_t> k1 = {'a'};
    std::vector<uint8_t> v1 = {'1'};
    {
        aqa::Database db(path, 4, 100, nullptr);
        db.put(k1, v1);
    }

    aqa::AccessObserver obs(64);
    {
        aqa::Database db(path, 4, 100, &obs);
        aqa::StorageEngine* engine = db.get_engine_ptr_for_benchmarking();
        ASSERT_TRUE(engine != nullptr);
        if (engine->get_total_pages() > 0) {
            auto h = engine->fetch_page(0);
            (void)h;
        }
        auto res = db.get(k1);
        ASSERT_TRUE(res.has_value());
    }
    ASSERT_TRUE(obs.get_total_recorded() >= 1u);
    auto ids = obs.get_recent_page_ids(16);
    ASSERT_TRUE(!ids.empty());

    std::filesystem::remove(path);
    if (std::filesystem::exists(path + ".wal")) {
        std::filesystem::remove(path + ".wal");
    }
}

void test_observer_receives_hits_and_misses() {
    std::string path = "access_observer_hits.db";
    std::filesystem::remove(path);
    if (std::filesystem::exists(path + ".wal")) {
        std::filesystem::remove(path + ".wal");
    }

    std::vector<uint8_t> k1 = {'x'};
    std::vector<uint8_t> k2 = {'y'};
    std::vector<uint8_t> val = {'v'};
    {
        aqa::Database db(path, 4, 100, nullptr);
        db.put(k1, val);
        db.put(k2, val);
    }

    aqa::AccessObserver obs(64);
    {
        aqa::Database db(path, 4, 100, &obs);
        db.get(k1);
        db.get(k2);
    }

    auto events = obs.get_recent_page_accesses(16);
    ASSERT_TRUE(!events.empty());
    bool had_miss = false;
    bool had_hit = false;
    for (const auto& e : events) {
        if (e.is_hit) had_hit = true;
        else had_miss = true;
    }
    ASSERT_TRUE(had_miss);
    ASSERT_TRUE(had_hit);

    std::filesystem::remove(path);
    if (std::filesystem::exists(path + ".wal")) {
        std::filesystem::remove(path + ".wal");
    }
}

int main() {
    test_observer_direct_page_cache();
    test_record_and_retrieve();
    test_ring_wraparound();
    test_record_access_from_rid();
    test_request_more_than_stored();
    test_null_observer_no_crash();
    test_observer_receives_page_accesses_via_database();
    test_observer_receives_hits_and_misses();

    std::cout << "All AccessObserver tests passed." << std::endl;
    return 0;
}
