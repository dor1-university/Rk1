#include <windows.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    const int stages_count = 4;
    const int products_count = 6;

    std::mutex mtx;
    std::condition_variable cv;

    int current_stage = 0;
    int current_product = 1;
    bool finished = false;

    std::atomic<int> total_stage_runs{0};
    std::vector<std::atomic<int>> stage_done(stages_count);
    for (auto& counter : stage_done) {
        counter.store(0);
    }

    std::vector<std::thread> stage_threads;
    for (int stage_id = 0; stage_id < stages_count; ++stage_id) {
        stage_threads.emplace_back([&, stage_id]() {
            while (true) {
                int product_for_work = 0;

                {
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [&]() {
                        return finished || (current_stage == stage_id);
                    });

                    if (finished) {
                        break;
                    }

                    product_for_work = current_product;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(120 + stage_id * 60));

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    std::cout << "Этап " << stage_id + 1
                              << ": обработан товар " << product_for_work << '\n';

                    stage_done[stage_id].fetch_add(1);
                    total_stage_runs.fetch_add(1);

                    if (stage_id == stages_count - 1) {
                        ++current_product;
                        if (current_product > products_count) {
                            finished = true;
                        }
                        current_stage = 0;
                    } else {
                        ++current_stage;
                    }
                }

                cv.notify_all();
            }
        });
    }

    cv.notify_all();

    for (auto& worker : stage_threads) {
        worker.join();
    }

    std::cout << "\nИтоги конвейера:\n";
    for (int i = 0; i < stages_count; ++i) {
        std::cout << "Этап " << i + 1
                  << ": выполнений = " << stage_done[i].load() << '\n';
    }
    std::cout << "Всего операций этапов: " << total_stage_runs.load() << '\n';

    return 0;
}
