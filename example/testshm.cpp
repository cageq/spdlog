#include "spdlog/spdlog.h"
#include "spdlog/sinks/shm_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <iostream>
#include <chrono>
#include "fastlog.h"

using namespace std::chrono;
    FastWriter  fastWriter; 

int main(int argc, char *argv[])
{

    fastWriter.open("./fast.log", 3096000000); 
    high_resolution_clock::time_point t5 = high_resolution_clock::now();

    for (uint32_t i = 0; i < 1000000; i++)
    {
//        fastWriter.format("hello {} \n", i );
//        fastWriter.format("hello {} \n", i );
    }

    high_resolution_clock::time_point t6 = high_resolution_clock::now();
    fastWriter.close(); 
    duration<double, std::ratio<1, 1000000>> fastDura = duration_cast<duration<double, std::ratio<1, 1000000>>>(t6 - t5);
    std::cout << fastDura.count() << " microseconds" << std::endl;




    auto shmLogger = spdlog::shm_logger_st("shmlog0", "shm_example.log");
    shmLogger->set_level(spdlog::level::trace);
    
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (uint32_t i = 0; i < 1000000; i++)
    {
        shmLogger->info("hello {} ", i );
        shmLogger->error("hello {}", i );
    }

    high_resolution_clock::time_point t2 = high_resolution_clock::now();

    duration<double, std::ratio<1, 1000000>> shmDura = duration_cast<duration<double, std::ratio<1, 1000000>>>(t2 - t1);
    std::cout << shmDura.count() << " microseconds" << std::endl;

    auto fileLogger = spdlog::rotating_logger_st("filelog", "file_example.log", 1024 * 1024 * 1024, 10);
    fileLogger->set_level(spdlog::level::trace);
    high_resolution_clock::time_point t3 = high_resolution_clock::now();
    for (uint32_t i = 0; i < 1000000; i++)
    {

        fileLogger->info("hello {} ", i );
        fileLogger->error("hello {}", i );
    }
    high_resolution_clock::time_point t4 = high_resolution_clock::now();

    duration<double, std::ratio<1, 1000000>> fileDura = duration_cast<duration<double, std::ratio<1, 1000000>>>(t4 - t3);
    std::cout << fileDura.count() << " microseconds" << std::endl;

    return 0;
}
