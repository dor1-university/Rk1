#include <windows.h>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

struct Zone {
    std::atomic<int> stock{0};
    std::atomic<int> checks_done{0};
    std::atomic<int> refills_done{0};
    std::mutex mtx;
    std::condition_variable cv;
    bool need_refill = false;
    bool checked = false;
};

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    const int zones_count = 2;
    const int min_stock = 5;
    const int refill_amount = 7;
    const int cycles = 8;

    std::vector<Zone> zones(zones_count);
    zones[0].stock.store(4);
    zones[1].stock.store(9);

    std::thread checker([&]() {
        for (int step = 0; step < cycles; ++step) {
            int zone_id = step % zones_count;
            Zone& zone = zones[zone_id];

            std::unique_lock<std::mutex> lock(zone.mtx);
            int current = zone.stock.load();
            zone.checks_done.fetch_add(1);
            zone.need_refill = current < min_stock;
            zone.checked = true;

            std::cout << "Проверка: зона " << zone_id + 1
                      << ", остаток = " << current;
            if (zone.need_refill) {
                std::cout << " -> нужно пополнение";
            } else {
                std::cout << " -> пополнение не требуется";
            }
            std::cout << '\n';

            zone.cv.notify_one();
            zone.cv.wait(lock, [&]() { return !zone.checked; });
            lock.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }
    });

    std::thread filler([&]() {
        for (int step = 0; step < cycles; ++step) {
            int zone_id = step % zones_count;
            Zone& zone = zones[zone_id];

            std::unique_lock<std::mutex> lock(zone.mtx);
            zone.cv.wait(lock, [&]() { return zone.checked; });

            if (zone.need_refill) {
                int before = zone.stock.load();
                int after = before + refill_amount;
                while (!zone.stock.compare_exchange_weak(before, after)) {
                    after = before + refill_amount;
                }
                zone.refills_done.fetch_add(1);
                std::cout << "Пополнение: зона " << zone_id + 1
                          << ", было " << before
                          << ", стало " << after << '\n';
            } else {
                int current = zone.stock.load();
                std::cout << "Пополнение: зона " << zone_id + 1
                          << " пропущено, остаток = " << current << '\n';
            }

            zone.checked = false;
            lock.unlock();
            zone.cv.notify_one();

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    checker.join();
    filler.join();

    std::cout << "\nИтоговое состояние склада:\n";
    for (int i = 0; i < zones_count; ++i) {
        std::cout << "Зона " << i + 1
                  << ": остаток = " << zones[i].stock.load()
                  << ", проверок = " << zones[i].checks_done.load()
                  << ", пополнений = " << zones[i].refills_done.load()
                  << '\n';
    }

    return 0;
}
