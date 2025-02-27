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

#ifndef CRT_PRINTF_TRAITS_H_
#define CRT_PRINTF_TRAITS_H_

#include <cstdint>

// Intentionally left empty, so that
// types without an implementation will cause
// a compile failure.
template <typename T>
struct PrintfTraits {
};

template <>
struct PrintfTraits<int8_t> {
  static constexpr const char* kFmt = "%d";
  static constexpr const char* kFmtHex = "0x%02x";
};

template <>
struct PrintfTraits<int16_t> {
  static constexpr const char* kFmt = "%d";
  static constexpr const char* kFmtHex = "0x%04x";
};

template <>
struct PrintfTraits<int32_t> {
  static constexpr const char* kFmt = "%ld";
  static constexpr const char* kFmtHex = "0x%lx";
};

template <>
struct PrintfTraits<uint8_t> {
  static constexpr const char* kFmt = "%u";
  static constexpr const char* kFmtHex = "0x%02x";
};

template <>
struct PrintfTraits<uint16_t> {
  static constexpr const char* kFmt = "%u";
  static constexpr const char* kFmtHex = "0x%04x";
};

template <>
struct PrintfTraits<uint32_t> {
  static constexpr const char* kFmt = "%lu";
  static constexpr const char* kFmtHex = "0x%lx";
};

#endif  // CRT_PRINTF_TRAITS_H_
