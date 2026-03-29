#include <windows.h>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    const int workers_count = 5;
    const int queue1_tasks = 3;
    const int queue2_tasks = 2;
    const int iterations_per_worker = 5;

    HANDLE queue1_sem = CreateSemaphore(nullptr, queue1_tasks, queue1_tasks, nullptr);
    HANDLE queue2_sem = CreateSemaphore(nullptr, queue2_tasks, queue2_tasks, nullptr);

    if (queue1_sem == nullptr || queue2_sem == nullptr) {
        std::cerr << "Не удалось создать семафоры\n";
        return 1;
    }

    std::mutex out_mtx;
    std::vector<std::thread> workers;

    for (int worker_id = 1; worker_id <= workers_count; ++worker_id) {
        workers.emplace_back([&, worker_id]() {
            for (int i = 1; i <= iterations_per_worker; ++i) {
                HANDLE taken_queue = nullptr;
                int queue_id = 0;

                if (WaitForSingleObject(queue1_sem, 0) == WAIT_OBJECT_0) {
                    taken_queue = queue1_sem;
                    queue_id = 1;
                } else {
                    WaitForSingleObject(queue2_sem, INFINITE);
                    taken_queue = queue2_sem;
                    queue_id = 2;
                }

                {
                    std::lock_guard<std::mutex> lock(out_mtx);
                    std::cout << "Рабочий " << worker_id
                              << " взял задачу из очереди " << queue_id << '\n';
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(140 + worker_id * 30));

                ReleaseSemaphore(taken_queue, 1, nullptr);

                {
                    std::lock_guard<std::mutex> lock(out_mtx);
                    std::cout << "Рабочий " << worker_id
                              << " вернул задачу в очередь " << queue_id << '\n';
                }
            }
        });
    }

    for (auto& worker : workers) {
        worker.join();
    }

    CloseHandle(queue1_sem);
    CloseHandle(queue2_sem);

    std::cout << "\nОбработка очередей завершена\n";
    return 0;
}
