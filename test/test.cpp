#include "test.hh"

#include "mutex"
#include "thread"
#include "vector"
#include "iostream"
#include "memory_resource"
#include "../pool/pool.hh"

#include "boost/pool/singleton_pool.hpp"

std::vector<std::thread>             gThreads;
std::pmr::synchronized_pool_resource gStdPmrPool;

int main() {
    printf("config\n");
    std::cout << "Test Count: " << TEST_LOOP_COUNT << std::endl;
    std::cout << "Chunk Count: " << TEST_CHUNK_COUNT << std::endl;
    std::cout << "Chunk Size: " << TEST_CHUNK_SIZE << std::endl;
    std::cout << "Chunk Align: " << TEST_CHUNK_ALIGN << std::endl;
    std::cout << "Thread Count:" << TEST_THREAD_COUNT << std::endl;

    gThreads.reserve(TEST_THREAD_COUNT);

    for(gLoop = 0; gLoop < TEST_LOOP_COUNT; ++gLoop) {
        gKind = 0;

        printf("\nTest: %lld\n", gLoop + 1);

        LINE();
        printf("c malloc / free\n");
        thread(allocSTD, allocSTD);
        LINE();

        LINE();
        printf("boost::pool\n");
        thread(allocBoost, freeBoost);
        LINE();

        LINE();
        printf("std::pmr\n");
        thread(allocPMR, allocPMR);
        LINE();

        LINE();
        printf("custom\n");
        thread(allocLWE, freeLWE);
        LINE();
    }

    // clac average
    float average[E_COUNT] = { 0 };
    for(size_t i = 0; i < E_COUNT; ++i) {
        for(size_t j = 0; j < TEST_LOOP_COUNT; ++j) {
            average[i] += gTimes[i][j];
        }
        average[i] /= TEST_LOOP_COUNT;
    }

    // print times
    for(size_t i = 0; i < TEST_LOOP_COUNT; ++i) {
        LINE();
        printf("%lld work\n", i + 1);

        for(int kind = 0; kind < E_COUNT; ++kind) {
            std::cout << G_KIND_STRING[kind] << ":\t" << gTimes[kind][i] << "\tus\n";
        }
        LINE();
    }

    LINE();
    printf("average\n");
    for(size_t i = 0; i < E_COUNT; ++i) {
        std::cout << G_KIND_STRING[i] << ":\t" << average[i] << "\tus\n";
    }
    LINE();

    printf("\nNOTE\n");
    LINE();
    printf("- custom pool pre-init to full usage: %s\n", TEST_POOL_PRE_INIT ? "true" : "false");
    printf("- boost pool does not support custom alignment.\n");
    printf("- environment is: ");
#ifdef NDEBUG
    printf("Rlease\n");
#else
    printf("Debug\n");
#endif

    return 0;
}

size_t getNextIndex() {
    static std::mutex mtx;
    mtx.lock();
    size_t idx = gIndex++;
    mtx.unlock();
    return idx;
}

void allocSTD(size_t idx) {
    gMem[idx] = malloc(TEST_CHUNK_SIZE);
}

void freeSTD(size_t idx) {
    free(gMem[idx]);
}

void allocBoost(size_t idx) {
    gMem[idx] = boost::singleton_pool<void*, TEST_CHUNK_SIZE>::malloc();
}

void freeBoost(size_t idx) {
    boost::singleton_pool<void*, TEST_CHUNK_SIZE>::free(gMem[idx]);
}

void allocPMR(size_t idx) {
    gMem[idx] = gStdPmrPool.allocate(TEST_CHUNK_SIZE, TEST_CHUNK_ALIGN);
}

void freePMR(size_t idx) {
    gStdPmrPool.deallocate(gMem[idx], TEST_CHUNK_ALIGN);
}

void allocLWE(size_t idx) {
    auto& pool = lwe::mem::pool::statics<TEST_CHUNK_SIZE, TEST_CHUNK_ALIGN, POOL_INIT_COUNT>();
    gMem[idx]  = pool.construct();
}

void freeLWE(size_t idx) {
    auto& pool = lwe::mem::pool::statics<TEST_CHUNK_SIZE, TEST_CHUNK_ALIGN, POOL_INIT_COUNT>();
    pool.destruct(gMem[idx]);
}

void thread(func allocator, func deallocator) {
    memset(gMem, 0, TEST_CHUNK_COUNT);
    gIndex = 0;
    gTimer.reset();
    for(size_t idx = getNextIndex(); idx < TEST_CHUNK_COUNT; idx = getNextIndex()) {
        allocator(idx);
    }
    gTimer.stop();
    gTimes[gKind++][gLoop] = gTimer.microseconds();
    checkAlloc();

    gIndex = 0;
    gTimer.reset();
    for(size_t idx = getNextIndex(); idx < TEST_CHUNK_COUNT; idx = getNextIndex()) {
        deallocator(idx);
        gMem[idx] = nullptr;
    }
    gTimer.stop();
    gTimes[gKind++][gLoop] = gTimer.microseconds();
    checkFree();
}

void checkAlloc() {
    for(size_t i = 0; i < TEST_CHUNK_COUNT; ++i) {
        if(gMem[i] == nullptr) {
            std::cerr << "malloc: detected thread problem" << std::endl;
            return;
        }
    }
    std::cout << "malloc OK: not detected thread problem" << std::endl;
}

void checkFree() {
    for(size_t i = 0; i < TEST_CHUNK_COUNT; ++i) {
        if(gMem[i] != nullptr) {
            std::cerr << "free: detected thread problem" << std::endl;
            return;
        }
    }
    std::cout << "free OK: not detected thread problem" << std::endl;
}