#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>
#include <atomic>

std::queue<std::string> messageQueue;
std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> finished(false);

int t1 = 3; // максимальний інтервал генерації
int t2 = 5; // максимальний інтервал обробки

std::random_device rd;
std::mt19937 gen(rd());

void producer(int count) {
    std::uniform_int_distribution<> dist(1, t1);

    for (int i = 1; i <= count; ++i) {
        int sleepTime = dist(gen);
        std::this_thread::sleep_for(std::chrono::seconds(sleepTime));

        std::string message = "Повідомлення #" + std::to_string(i);

        {
            std::lock_guard<std::mutex> lock(mtx);
            messageQueue.push(message);
            std::cout << "[Згенеровано] " << message << std::endl;
        }

        cv.notify_one();
    }

    finished = true;
    cv.notify_one();
}

void consumer() {
    std::uniform_int_distribution<> dist(1, t2);

    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [] {
            return !messageQueue.empty() || finished.load();
        });

        if (messageQueue.empty() && finished)
            break;

        std::string message = messageQueue.front();
        messageQueue.pop();
        lock.unlock();

        int sleepTime = dist(gen);
        std::this_thread::sleep_for(std::chrono::seconds(sleepTime));

        std::cout << "    [Оброблено] " << message << std::endl;
    }
}

int main() {
    int messageCount = 10;

    std::thread prod(producer, messageCount);
    std::thread cons(consumer);

    prod.join();
    cons.join();

    std::cout << "\nВсі повідомлення оброблено.\n";
    return 0;
}
