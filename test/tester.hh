#pragma once


#include "chrono"

class timer;

constexpr size_t TEST_THREAD_COUNT = 32;   // thread count
constexpr size_t TEST_BLOCK_COUNT  = 4096; // allocate count
constexpr size_t TEST_BLOCK_SIZE   = 256;  // byte: allocate unit
constexpr size_t TEST_BLOCK_ALIGN  = 16;   // byte
constexpr size_t TEST_LOOP_COUNT   = 10;   // for average

void main_threadSafetyTest();

// C++17 use pmr
void main_performanceTest();

class timer {
public:
    timer() {
        reset();
    }

    void reset() {
        start = std::chrono::steady_clock::now();
    }

    void stop() {
        end = std::chrono::steady_clock::now();
    }

    float microseconds() const noexcept {
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    float milliseconds() const noexcept {
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

    float seconds() const noexcept {
        return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
    }

private:
    std::chrono::time_point<std::chrono::steady_clock> start, end;
};

#include "tester.inl"