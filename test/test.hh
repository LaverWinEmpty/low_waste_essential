#pragma once

#include "chrono"
#include "../pool/config.hh"

#define TEST_POOL_PRE_INIT 1

#define LINE() printf("==============================================================================\n");

//======================================================================================================================
// config
//======================================================================================================================
constexpr size_t TEST_THREAD_COUNT = 16;      // thread count
constexpr size_t TEST_CHUNK_COUNT  = 1000000; // allocate count
constexpr size_t TEST_CHUNK_SIZE   = 256;     // byte: allocate unit
constexpr size_t TEST_CHUNK_ALIGN  = 16;      // byte
constexpr size_t TEST_LOOP_COUNT   = 10;      // for average

#if TEST_POOL_PRE_INIT
constexpr size_t POOL_INIT_COUNT = TEST_CHUNK_COUNT;
#else
constexpr size_t POOL_INIT_SIZE = lwe::config::DEF_CACHE;
#endif

// test order
enum {
    E_C_ALLOC, // malloc
    E_C_FREE,  // free
    E_BOOST_ALLOC,
    E_BOOST_FREE,
    E_PMR_ALLOC,
    E_PMR_FREE,
    E_LEW_ALLOC,
    E_LEW_FREE,
    E_COUNT
};

// clang-format off
// enum to string
const char* const G_KIND_STRING[E_COUNT] = {
    "c std malloc",
    "c std free",
    "boost allocate",
    "boost free",
    "pmr allocate",
    "pmr free",
    "my allocate",
    "my free"
};
// clang-format on

//======================================================================================================================
// functions
//======================================================================================================================
using func = void (*)(size_t);

// main
int main();

// thread main
void thread(func alloc, func free);

// alloc
void allocSTD(size_t idx); // ::malloc
void allocBoost(size_t idx);
void allocPMR(size_t idx);
void allocLWE(size_t idx);

// free
void allocSTD(size_t idx); // ::free
void freeBoost(size_t idx);
void freePMR(size_t idx);
void freeLWE(size_t idx);

// get next index using mutex
size_t getNextIndex();

// thread check
void checkAlloc();
void checkFree();

//======================================================================================================================
// temp timer
//======================================================================================================================
class timer {
public:
    timer() { reset(); }
    void  reset() { start = std::chrono::steady_clock::now(); }
    void  stop() { end = std::chrono::steady_clock::now(); }
    float microseconds() const { return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count(); }
    float milliseconds() const { return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count(); }
    float seconds() const { return std::chrono::duration_cast<std::chrono::seconds>(end - start).count(); }

private:
    std::chrono::time_point<std::chrono::steady_clock> start, end;
};

//======================================================================================================================
// globals
//======================================================================================================================
void*  gMem[TEST_CHUNK_COUNT];
float  gTimes[/* enum */ E_COUNT][TEST_LOOP_COUNT];
size_t gIndex = 0;
size_t gKind  = 0;
size_t gLoop  = 0;
timer  gTimer;