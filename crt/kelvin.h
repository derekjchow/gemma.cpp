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
// Kelvin helper header

#ifndef CRT_KELVIN_H_
#define CRT_KELVIN_H_

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define __volatile_always__ volatile

// Helper macros for Intrinsics definitions.
#define ARGS_F_A(FN, A0) FN " " #A0 "\n"
#define ARGS_F_A_A(FN, A0, A1) FN " " #A0 ", " #A1 "\n"
#define ARGS_F_A_A_A(FN, A0, A1, A2) FN " " #A0 ", " #A1 ", " #A2 "\n"
#define ARGS_F_A_A_A_A(FN, A0, A1, A2, A3) \
  FN " " #A0 ", " #A1 ", " #A2 ", " #A3 "\n"

#include "crt/kelvin_intrinsics.h"

#define vm0 v0
#define vm1 v4
#define vm2 v8
#define vm3 v12
#define vm4 v16
#define vm5 v20
#define vm6 v24
#define vm7 v28
#define vm8 v32
#define vm9 v36
#define vm10 v40
#define vm11 v44
#define vm12 v48
#define vm13 v52
#define vm14 v56
#define vm15 v60

struct vconv_u8_t {
  uint32_t mode : 2;    // 1:0
  uint32_t start : 5;   // 6:2
  uint32_t stop : 5;    // 11:7
  int32_t sbias1 : 9;   // 20:12
  uint32_t sdata1 : 1;  // 21
  int32_t sbias2 : 9;   // 30:22
  uint32_t sdata2 : 1;  // 31
};
static_assert(sizeof(struct vconv_u8_t) == 4);

struct vdwconv_u8_t {
  uint32_t mode:2;      // 1:0
  uint32_t sparsity:2;  // 3:2
  uint32_t regbase:4;   // 7:4
  uint32_t rsvd:4;      // 11:8
  int32_t sbias1:9;    // 20:12
  uint32_t sdata1:1;    // 21
  int32_t sbias2:9;    // 30:22
  uint32_t sdata2:1;    // 31
};
static_assert(sizeof(struct vdwconv_u8_t) == 4);

#endif  // CRT_KELVIN_H_
