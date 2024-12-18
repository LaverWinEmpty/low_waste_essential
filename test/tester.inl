#include "tester.hh"

#include "thread"
#include "atomic"
#include "vector"
#include "iostream"
#include "memory_resource"
#include "../pool/pool.hh"

#define LINE() printf("=======================================\n");

void* memory[TEST_BLOCK_COUNT];

enum {
    // pmr pool
    E_STD_ALLOC,
    E_STD_FREE,

    // my pool
    E_LEW_ALLOC,
    E_LEW_FREE,

    E_COUNT
};

const char* const testKind[E_COUNT] = {
    "std allocate",
    "std free",
    "my allocate",
    "my free"
};

float times[/* enum */E_COUNT][TEST_LOOP_COUNT];

void main_threadSafetyTest() {

}

void main_performanceTest() {
    timer t;
    
    std::pmr::synchronized_pool_resource stdPmrPool;
    
    // use default
    auto& lwePool = lwe::mem::pool::statics<TEST_BLOCK_SIZE, lwe::config::DEF_CACHE, TEST_BLOCK_ALIGN>();

    printf("config");
    std::cout << "Test Count: "  << TEST_LOOP_COUNT  << std::endl;
    std::cout << "Block Count: " << TEST_BLOCK_COUNT << std::endl;
    std::cout << "Block Size: "  << TEST_BLOCK_SIZE  << std::endl;
    std::cout << "Block Align: " << TEST_BLOCK_ALIGN << std::endl;


    for(size_t i = 0; i < TEST_LOOP_COUNT; ++i) {
        ////////////////////////////////////////////////////////////////////////        
        /// std allocate
        ////////////////////////////////////////////////////////////////////////
        t.reset();
        for(size_t i = 0; i < TEST_BLOCK_COUNT; ++i) {
            memory[i] = stdPmrPool.allocate(TEST_BLOCK_SIZE, TEST_BLOCK_ALIGN);
        }
        t.stop();
        times[E_STD_ALLOC][i] = t.microseconds();


        ////////////////////////////////////////////////////////////////////////
        /// std free
        ////////////////////////////////////////////////////////////////////////
        t.reset();
        for(size_t i = 0; i < TEST_BLOCK_COUNT; ++i) {
            stdPmrPool.deallocate(memory[i], TEST_BLOCK_ALIGN);
        }
        t.stop();
        times[E_STD_FREE][i] = t.microseconds();


        ////////////////////////////////////////////////////////////////////////
        /// my allocate
        ////////////////////////////////////////////////////////////////////////
        t.reset();
        for(size_t i = 0; i < TEST_BLOCK_COUNT; ++i) {
            memory[i] = lwePool.construct();
        }
        t.stop();
        times[E_LEW_ALLOC][i] = t.microseconds();


        ////////////////////////////////////////////////////////////////////////
        /// my free
        ////////////////////////////////////////////////////////////////////////
        t.reset();
        for(size_t i = 0; i < TEST_BLOCK_COUNT; ++i) {
            lwePool.destruct(memory[i]);
        }
        t.stop();
        times[E_LEW_FREE][i] = t.microseconds();
    }

    float average[E_COUNT] = { 0 };
    for(size_t i = 0; i < E_COUNT; ++i) {
        for(size_t j = 0; j < TEST_LOOP_COUNT; ++j) {
            average[i] += times[i][j];
        }
        average[i] /= TEST_LOOP_COUNT;
    }

    for (size_t i = 0; i < TEST_LOOP_COUNT; ++i) {
        LINE();
        printf("%lld work\n", i + 1);

        for (int kind = 0; kind < E_COUNT; ++kind) {
            std::cout << testKind[kind] << ":\t" << times[kind][i] << "us\n";
        }
        LINE();
    }

    LINE();
    printf("average\n");
    for(size_t i = 0; i < E_COUNT; ++i) {
        std::cout << testKind[i] << ":\t" << average[i] << "us\n";
    }
    LINE();

}