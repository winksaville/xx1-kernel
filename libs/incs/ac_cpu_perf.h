/*
 * Copyright 2015 Wink Saville
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

#ifndef SADIE_LIBS_AC_CPU_PERF_INCS_AC_CPU_PERF_H
#define SADIE_LIBS_AC_CPU_PERF_INCS_AC_CPU_PERF_H

#include <ac_inttypes.h>

/**
 * Initialize module
 */
void ac_cpu_perf_init(void);

/**
 * Zero counters and enable divider
 */
void ac_cpu_perf_zero_counters(ac_bool enable_divider);

/**
 * Frequency of ac_cpu_perf_cycle_counter in cycles per second.
 */
ac_u64 ac_cpu_perf_cycle_counter_freq();

#include <ac_cpu_perf_impl.h>

#endif
