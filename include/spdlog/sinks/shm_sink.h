#pragma once
#include <cstdio>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <memory>

#include <spdlog/details/null_mutex.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/synchronous_factory.h>
#include "shm_buffer.h"
#include <mutex>

#define DEFAULT_PAGE_SIZE 4096
#define MMAP_BLOCK_SIZE 1024 * 1024 * 8

namespace spdlog {
namespace sinks {

template<typename Mutex>
class shm_sink : public base_sink<Mutex>
{
    public:

    fmt::shm_buffer log_buffer_; 

    shm_sink(const filename_t &filename = "spdlog.log", bool truncate = false, const file_event_handlers &event_handlers = {}):log_buffer_(filename)
    {
        
    }
    ~shm_sink(){
      
    }
protected:
      

    void close()
    {
      
    } 


    void sink_it_(const details::log_msg &msg) override
    { 
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, log_buffer_);
        
    }
    void flush_() override {
        log_buffer_.flush(); 
    }
 
};

using shm_sink_mt = shm_sink<details::null_mutex>;
using shm_sink_st = shm_sink<details::null_mutex>;

} // namespace sinks

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> shm_logger_mt(const std::string &logger_name, const std::string &log_file)
{
    auto shm_logger = Factory::template create<sinks::shm_sink_mt>(logger_name, log_file);
    shm_logger->set_level(level::off);
    return shm_logger;
}

template<typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> shm_logger_st(const std::string &logger_name, const std::string &log_file)
{
    auto shm_logger = Factory::template create<sinks::shm_sink_st>(logger_name, log_file);
    shm_logger->set_level(level::off);
    return shm_logger;
}

 
} // namespace spdlog
