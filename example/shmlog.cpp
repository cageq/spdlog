#include "spdlog/spdlog.h"
#include "spdlog/sinks/shm_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <iostream>
#include <chrono>
#include "spdlog/sinks/shm_buffer.h"
#include "fastlog.h"

using namespace std::chrono;
    FastWriter  fastWriter; 

int main(int argc, char *argv[])
{
 


    auto shmLogger = spdlog::shm_logger_st("shmlog0", "shm.log");
    shmLogger->set_level(spdlog::level::trace);
    
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (uint32_t i = 0; i < 10; i++)
    {
        shmLogger->info("hello {} welcome {}  ", i  , i / 899);
        shmLogger->error("hello {}", i );
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    duration<double, std::ratio<1, 1000000>> shmDura = duration_cast<duration<double, std::ratio<1, 1000000>>>(t2 - t1);
    std::cout << shmDura.count() << " microseconds" << std::endl;
 

    return 0;
}
