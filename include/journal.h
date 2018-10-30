#pragma once

#include <cstdio>
#include <ctime>
#include <mutex>

#define LOG_LEVEL_CRITICAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_MESSAGE 4
#define LOG_LEVEL_INFO 5
#define LOG_LEVEL_DEBUG 6

#define DEFAULT_LOG_LEVEL 6

#ifdef JOURNAL_C_FORMAT

#define LOG(fp, prefix, postfix, tag, ...) do { \
    auto __tv = std::time(NULL); \
    char timestamp_str[100]; \
    std::strftime(timestamp_str, sizeof(timestamp_str), "%F %T", std::localtime(&__tv)); \
    fprintf(fp, "%s %s: [%s] ", timestamp_str, prefix, tag); \
    fprintf(fp, __VA_ARGS__); \
    fputs(postfix, fp); \
    fflush(fp); \
    } while(0)

#else

#include <format.h>

#define LOG(fp, prefix, postfix, tag, ...) do { \
    auto __tv = std::time(NULL); \
    char timestamp_str[100]; \
    std::strftime(timestamp_str, sizeof(timestamp_str), "%F %T", std::localtime(&__tv)); \
    const auto sstr = xfmt::format(__VA_ARGS__); \
    fprintf(fp, "%s %s: [%s] ", timestamp_str, prefix, tag); \
    fputs(sstr.c_str(), fp); \
    fputs(postfix, fp); \
    fflush(fp); \
    } while(0)


#endif

#define LOG_CRITICAL(tag, ...) if (log_level >= LOG_LEVEL_CRITICAL) LOG(stderr, "\x1b[39;41;1mC", "\x1b[0m\n", tag, __VA_ARGS__)
#define LOG_ERROR(tag, ...) if (log_level >= LOG_LEVEL_ERROR) LOG(stderr, "\x1b[31;1mE", "\x1b[0m\n", tag, __VA_ARGS__)
#define LOG_WARNING(tag, ...) if (log_level >= LOG_LEVEL_WARNING) LOG(stdout, "\x1b[33;1mW", "\x1b[0m\n", tag, __VA_ARGS__)
#define LOG_MESSAGE(tag, ...) if (log_level >= LOG_LEVEL_MESSAGE) LOG(stdout, "\x1b[32mM", "\x1b[0m\n", tag, __VA_ARGS__)
#define LOG_INFO(tag, ...) if (log_level >= LOG_LEVEL_INFO) LOG(stdout, "I", "\x1b[0m\n", tag, __VA_ARGS__)
#define LOG_DEBUG(tag, ...) if (log_level >= LOG_LEVEL_DEBUG) LOG(stdout, "\x1b[36mD", "\x1b[0m\n", tag, __VA_ARGS__)

#define LOG_DEBUG_CHECKPOINT( tag ) LOG_DEBUG( tag, "%1", __FUNCTION__ )
#define LOG_DEBUG_CHECKPOINT_ONCE( tag ) do { \
    const auto this_fn = __FUNCTION__; \
    static std::once_flag __FUNCTION__##once_flag;\
    std::call_once( __FUNCTION__##once_flag, [this_fn] () { LOG_DEBUG( tag, "%1", this_fn ); } ); \
    ; \
    } while( 0 )

extern volatile int log_level;
