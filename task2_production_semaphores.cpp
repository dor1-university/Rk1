#include <windows.h>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    const int batches = 6;
    HANDLE sem_loaded = CreateSemaphore(nullptr, 0, batches, nullptr);
    HANDLE sem_processed = CreateSemaphore(nullptr, 0, batches, nullptr);

    if (sem_loaded == nullptr || sem_processed == nullptr) {
        std::cerr << "Не удалось создать семафоры\n";
        return 1;
    }

    std::thread loader([&]() {
        for (int i = 1; i <= batches; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(220));
            std::cout << "Загрузка: партия " << i << " готова\n";
            ReleaseSemaphore(sem_loaded, 1, nullptr);
        }
    });

    std::thread processor([&]() {
        for (int i = 1; i <= batches; ++i) {
            WaitForSingleObject(sem_loaded, INFINITE);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            std::cout << "Обработка: партия " << i << " готова\n";
            ReleaseSemaphore(sem_processed, 1, nullptr);
        }
    });

    std::thread packer([&]() {
        for (int i = 1; i <= batches; ++i) {
            WaitForSingleObject(sem_processed, INFINITE);
            std::this_thread::sleep_for(std::chrono::milliseconds(180));
            std::cout << "Упаковка: партия " << i << " завершена\n";
        }
    });

    loader.join();
    processor.join();
    packer.join();

    CloseHandle(sem_loaded);
    CloseHandle(sem_processed);

    std::cout << "\nВсе этапы производства выполнены\n";
    return 0;
}
