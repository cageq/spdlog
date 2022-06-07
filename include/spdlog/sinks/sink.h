// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <spdlog/details/log_msg.h>
#include <spdlog/formatter.h>

namespace spdlog {

namespace sinks {
class SPDLOG_API sink
{
public:
    virtual ~sink() = default;
    virtual void log(const details::log_msg &msg) = 0;
    virtual void flush() = 0;
    virtual void set_pattern(const std::string &pattern) = 0;
    virtual void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) = 0;

    template <class ... Args> 
    void format_log(const details::log_msg &msg,  bool log_enabled, bool traceback_enabled, string_view_t fmt,   Args ... args ){
        if (wbegin()){
            auto rst = fmt::format_to_n(wbegin(), wend()- wbegin(), fmt, args...); 
            advance(rst); 
        }else {
            memory_buf_t buf;
        #ifdef SPDLOG_USE_STD_FORMAT
                fmt_lib::vformat_to(std::back_inserter(buf), fmt, fmt_lib::make_format_args(std::forward<Args>(args)...));
        #else
                // seems that fmt::detail::vformat_to(buf, ...) is ~20ns faster than fmt::vformat_to(std::back_inserter(buf),..)
                fmt::detail::vformat_to(buf, fmt, fmt::make_format_args(std::forward<Args>(args)...));
        #endif
           
            log_it_(msg, true, true);
        }        
    }
    virtual char * wbegin(){
        return nullptr;
    }
    virtual char * wend(){
        return nullptr;
    }
    virtual void advance(size_t n){

    }

    void set_level(level::level_enum log_level);
    level::level_enum level() const;
    bool should_log(level::level_enum msg_level) const;

protected:
    // sink log level - default is all
    level_t level_{level::trace};
};

} // namespace sinks
} // namespace spdlog

#ifdef SPDLOG_HEADER_ONLY
#    include "sink-inl.h"
#endif
