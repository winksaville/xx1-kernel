/*
 * Copyright 2016 Wink Saville
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

#ifndef DESCRIPTORS_X86_64_PRINT_H
#define DESCRIPTORS_X86_64_PRINT_H

#include "ac_inttypes.h"
#include "interrupts_x86.h"
#include "descriptors_x86.h"

void print_tss_desc(char *str, TssDesc* desc);

void print_seg_desc(char *str, SegDesc* desc);

void print_desc_table(char *str, DescPtr dp);

#endif
