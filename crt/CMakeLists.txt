# Copyright 2023 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


enable_language(ASM)


include_directories(BEFORE SYSTEM $ENV{ROOTDIR}/sw/kelvin)

# Build Kelvin
add_library(kelvin INTERFACE)
add_library(kelvin_intrinsic STATIC)
target_sources(kelvin_intrinsic
    PRIVATE
      crt.S
      kelvin_start.S
      kelvin_gloss.cc
)

target_include_directories(kelvin_intrinsic PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(kelvin
INTERFACE
  kelvin_intrinsic
)

target_include_directories(kelvin INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_options(kelvin
    INTERFACE
      -Wl,--whole-archive ${CMAKE_CURRENT_BINARY_DIR}/libkelvin_intrinsic.a -Wl,--no-whole-archive,--no-warn-rwx-segments
)
