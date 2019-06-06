//
// Created by Administrator on 2019/6/5.
//
#ifndef JNI_TEST_WRAPPER_LOG_H

#define JNI_TEST_WRAPPER_LOG_H

#include <android/log.h>

#define LOG_LEVEL_VERBOSE 1
#define LOG_LEVEL_DEBUG 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_WARNING 4
#define LOG_LEVEL_ERROR 5
#define LOG_LEVEL_FATAL 6
#define LOG_LEVEL_SILENT 7

#ifndef LOG_TAG
#define LOG_TAG __FILE__
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#endif

#define LOG_NOOP (void) 0
#define LOG_PRINT(level, fmt, ...) \
    __android_log_print(level, LOG_TAG, "(%s:%u) %s: " fmt, \
    __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__)

#if LOG_LEVEL_VERBOSE >= LOG_LEVEL
#define LOG_VERBOSE(fmt, ...) LOG_PRINT(ANDROID_LOG_VERBOSE, fmt, ##__VA_ARGS__)
#else
#define LOG_VERBOSE(...) LOG_NOOP
#endif

#if LOG_LEVEL_DEBUG >= LOG_LEVEL
#define LOG_DEBUG(fmt, ...) LOG_PRINT(ANDROID_LOG_DEBUG,fmt,##__VA_ARGS__)
#else
#define LOG_DEBUG(...) LOG_NOOP
#endif

#if LOG_LEVEL_INFO >= LOG_LEVEL
#define LOG_INFO(fmt, ...) LOG_PRINT(ANDROID_LOG_INFO,fmt,##__VA_ARGS__)
#else
#define LOG_INFO(...) LOG_NOOP
#endif

#if LOG_LEVEL_WARNING >= LOG_LEVEL
#define LOG_WARNING(fmt, ...) LOG_PRINT(ANDROID_LOG_WARNING,fmt,##__VA_ARGS__)
#else
#define LOG_WARNING(...) LOG_NOOP
#endif

#if LOG_LEVEL_ERROR >= LOG_LEVEL
#define LOG_ERROR(fmt, ...) LOG_PRINT(ANDROID_LOG_ERROR, fmt, ##__VA_ARGS__)
#else
#define LOG_ERROR(...) LOG_NOOP
#endif

#if LOG_LEVEL_FATAL >= LOG_LEVEL
#define LOG_FATAL(fmt, ...) LOG_PRINT(ANDROID_LOG_FATAL,fmt,##__VA_ARGS__)
#else
#define LOG_FATAL(...)LOG_NOOP
#endif

#if LOG_LEVEL_SILENT >= LOG_LEVEL
#define LOG_ASSERT(expression, fmt, ...) \
    if(!(expression)){\
    __android_log_assert(#expression,LOG_TAG,fmt,##__VA_ARGS__);\
    }
#else
#define LOG_ASSERT(...) LOG_NOOP
#endif

#endif //JNI_TEST_WRAPPER_LOG_H
