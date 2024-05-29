#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <chrono>
#include <random>

std::mutex mtx; // Blokada dla pisarzy
std::mutex read_mtx; // Blokada dla dostępu do licznika czytelników
std::condition_variable cv_writer; // Zmienna warunkowa dla pisarzy
std::condition_variable cv_reader; // Zmienna warunkowa dla czytelników
int reader_count = 0; // Licznik czytelników
int read_operations_since_last_write = 0; // Licznik odczytów od ostatniego zapisu
bool data_ready = false; // Flaga wskazująca, czy dane są gotowe do odczytu
int data = 0; // Wspólne dane

void reader(int id) {
    while (true) {
        std::unique_lock<std::mutex> lk(read_mtx);
        cv_reader.wait(lk, [] {return data_ready && read_operations_since_last_write < 3; });

        // Symulacja odczytu
        std::cout << "Czytelnik " << id << " odczytuje wartość: " << data << std::endl;
        read_operations_since_last_write++;
        if (read_operations_since_last_write >= 3) {
            cv_writer.notify_one(); // Informuj pisarza, jeśli wykonano 3 odczyty
        }

        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Symulacja pracy czytelnika
    }
}

void writer() {
    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(1, 100);

    while (true) {
        std::unique_lock<std::mutex> lk(mtx);
        cv_writer.wait(lk, [] {return read_operations_since_last_write >= 3 || !data_ready; });

        // Symulacja zapisu
        data = distribution(generator); // Losowanie nowej wartości
        std::cout << "Pisarz zapisuje wartość: " << data << std::endl;
        data_ready = true;
        read_operations_since_last_write = 0; // Reset licznika odczytów

        lk.unlock();
        cv_reader.notify_all(); // Informuj czytelników o nowych danych
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Symulacja pracy pisarza
    }
}

int main() {
    std::vector<std::thread> threads;

    // Tworzenie wątków czytelników
    for (int i = 0; i < 5; ++i) {
        threads.push_back(std::thread(reader, i));
    }

    // Tworzenie wątku pisarza
    threads.push_back(std::thread(writer));

    // Oczekiwanie na zakończenie wątków (w praktyce, ten kod nigdy się nie zakończy)
    for (auto& t : threads) {
        t.join();
    }

    return 0;
}

