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

/**
 *  A simple linker script for kelvin core.
 *
 * Memory layout:
 *
 *  TCM_ORIGIN          --->   +=====================+
 *                             |                     |
 *                             |       .text         |
 *                             +---------------------|
 *                             |       .crt          |
 *                             +---------------------+
 *                             |      .rodata        |
 *                             +---------------------+
 *                             |    .init_array      |
 *                             +---------------------+
 *                             |       .data         |
 *                             +---------------------+
 *                             |       .bss          |
 *                             +---------------------+
 *                             |       .heap         |
 *                             |  (All unclamied     |
 *                             |       memory)       |
 *                             |                     |
 *  (TCM_END - 4096     --->   +---------------------+
 *     - model_output          |       .stack        |
 *     - stack)                +---------------------+
 *                             |   .model_output     |
 *  TCM_END - 4096      --->   +=====================+
 *                             |   .model_input      |
 +                             | (remainder of page) |
 *                             +---------------------+
 *  output_header (64B) --->   |.model_output_header |
 *  TCM_END             --->   +=====================+
 **/

TCM_SIZE = 0x400000;
STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : DEFINED(__stack_size__) ? __stack_size__ : 0x1000000;
__model_size__ = 830763008;

/*
 * This defines all of memory as 2x segments with the last page used for an
 * output header at a fixed location and the remainder of the page used for
 * input parameters. ML runtimes either copy input data from this region
 * (e.g. TfLite) or reference the data in the region directly (IREE).
 *
 * The "TCM segment" is sized as the physical memory size (TCM_SIZE) - 1x
 * page so that offsets from the end of "TCM" can be expressed as:
 *    ORIGIN(TCM) + LENGTH(TCM) - <offset>
 */
MEMORY {
  TCM(rwx): ORIGIN = 0, LENGTH = TCM_SIZE
  DRAM(rw): ORIGIN = 0x80000000+__model_size__, LENGTH = 0x80000000-__model_size__
}

ENTRY(_start)

SECTIONS {
  . = ORIGIN(TCM);
  .text : ALIGN(4) {
    *(._init)
    *(.text)
    *(.text.*)
    . = ALIGN(4);
    _ret = .;
    . = ALIGN(4);
    __text_end__ = .;
  } > TCM

  .init.array : ALIGN(4) {
    __init_array_start__ = .;
    *(.init_array)
    *(.init_array.*)
    . = ALIGN(4);
    __init_array_end__ = .;
  } > TCM

  .rodata : ALIGN(4) {
    *(.srodata)
    *(.srodata.*)
    *(.rodata)
    *(.rodata.*)
    . = ALIGN(4);
  } > TCM

  .data : ALIGN(4) {
    __data_start__ = .;
    /**
    * This will get loaded into `gp`, and the linker will use that register for
    * accessing data within [-2048,2047] of `__global_pointer$`.
    *
    * This is much cheaper (for small data) than materializing the
    * address and loading from that (which will take one extra instruction).
    */
    _global_pointer = . + 0x800;
    *(.sdata)
    *(.sdata.*)
    *(.data)
    *(.data.*)
    /* Align on 256 width. */
    . = ALIGN(256);
    __data_end__ = .;
  } > TCM

  .bss : ALIGN(4) {
    __bss_start__ = .;
    *(.sbss)
    *(.sbss.*)
    *(.bss)
    *(.bss.*)
    __bss_end__ = .;
  } > TCM

   /* All the unclaimed memory is used in heap */
  .heap (NOLOAD) : {
    . = ALIGN(64);
    __heap_start__ = .;
    . = ORIGIN(DRAM) + LENGTH(DRAM) - STACK_SIZE;
    . = ALIGN(64);
    __heap_end__ = .;
  } > DRAM

  .stack (NOLOAD) : {
    __stack_start__ = .;
    . += STACK_SIZE;
    . = ALIGN(64);
    __stack_end__ = .;
  } > DRAM
}

