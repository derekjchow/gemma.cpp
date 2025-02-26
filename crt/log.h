/*
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Kelvin logging helper header

#ifndef CRT_LOG_H_
#define CRT_LOG_H_

#include <stdio.h>

#define LOG_MAX_SZ 256

static inline void kelvin_simprint(const char *_string) {
  __asm__ volatile("flog %0 \n\t" : : "r"(_string));
}

#define SIMLOG(fmt, ...)                                 \
  do {                                                   \
    char tmp_log_msg[LOG_MAX_SZ];                        \
    snprintf(tmp_log_msg, LOG_MAX_SZ, fmt, __VA_ARGS__); \
    kelvin_simprint(tmp_log_msg);                        \
  } while (0)

#define LOG_ERROR(msg, args...) SIMLOG("%s |" msg "\n", "ERROR", ##args)
#define LOG_WARN(msg, args...) SIMLOG("%s |" msg "\n", "WARN", ##args)
#define LOG_INFO(msg, args...) SIMLOG("%s |" msg "\n", "INFO", ##args)
#define LOG_DEBUG(msg, args...) SIMLOG("%s |" msg "\n", "DEBUG", ##args)
#define LOG_NOISY(msg, args...) SIMLOG("%s |" msg "\n", "NOISY", ##args)

#endif  // CRT_LOG_H_
