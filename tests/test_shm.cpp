/*
 * This content is released under the MIT License as specified in https://raw.githubusercontent.com/gabime/spdlog/master/LICENSE
 */
#include "includes.h"
#include "spdlog/sinks/shm_sink.h"

TEST_CASE("shm_mt", "[shm]")
{
    auto l = spdlog::shm_logger_mt("test", "shmlog.log");
    l->set_pattern("%+");
    l->set_level(spdlog::level::trace);
    l->trace("Test shm loggint mt");
    spdlog::drop_all();
}
 
