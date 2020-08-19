#pragma once
#include <stdarg.h>

#ifndef LIBRARY_NAME
#define LIBRARY_NAME "Unknown"
#endif

#define infol(fmt, ...)                                          \
  do {                                                               \
    fprintf(stderr,                                                  \
            "[I] "                                                   \
            "[%s] [%s(%d)] [%s] : " fmt,                             \
            LIBRARY_NAME, __FILE__, __LINE__, __FUNCTION__); \
  } while (0)
#define errorl(fmt, ...)                                         \
  do {                                                               \
    fprintf(stderr,                                                  \
            "\033[31m[E] "                                           \
            "[%s] [%s(%d)] [%s] : " fmt "\033[0m",                   \
            LIBRARY_NAME, __FILE__, __LINE__, __FUNCTION__); \
  } while (0)