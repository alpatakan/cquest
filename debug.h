#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>

#define log_helper(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)

// A4
#undef log_helper
#define LOG_FILE "/tmp/jobs_log"

static FILE* debug_stream = NULL;
static inline FILE* debug_file(void) {
#ifdef LOG_STDOUT
  return stdout;
#endif

  if (!debug_stream) {
    debug_stream = fopen(LOG_FILE, "a+");
  }
  return debug_stream;
}

#define log_debug(stream, fmt, ...)                                     \
  fprintf((stream), DEBUG_ID " [%s:%d]: " fmt "\n", __func__, __LINE__, \
          ##__VA_ARGS__)
#define log_helper(fmt, ...) log_debug(debug_file(), fmt, ##__VA_ARGS__)
// A4 end
#endif /* __DEBUG_H__ */