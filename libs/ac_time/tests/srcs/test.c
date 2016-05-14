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

#define NDEBUG

#include <ac_time.h>

#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_string.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_tsc.h>

ac_bool test_time(void) {
  ac_bool error = AC_FALSE;
  ac_u8 buff[64];
  ac_u64 f = ac_tsc_freq();
  ac_u64 ticks;

  ticks = 0;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(0, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(0, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f / 10;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f/10, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f/10, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = (f / 2) - 1;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str((f/2)-1, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str((f/2)-1, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f / 2;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f/2, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f/2, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m0.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f + (f / 10);
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/10), 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/10), 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/10), 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.10s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = f + (f / 2) - 100;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-100, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-100, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-100, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-100, 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.4999", (char*)buff, 14) == 0);

  ticks = f + (f / 2) - 10;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-10, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-10, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-10, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-10, 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.4999", (char*)buff, 14) == 0);

  ticks = f + (f / 2) - 1;
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-1, 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-1, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-1, 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2)-1, 9)=%s\n", buff);
  error |= AC_TEST((ac_strncmp("0y0d0h0m1.5000", (char*)buff, 14) == 0)
               | (ac_strncmp("0y0d0h0m1.4999", (char*)buff, 14) == 0));


  ticks = f + (f / 2);
  ac_ticks_to_duration_str(ticks, LEADING_0, 0, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2), 0)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m2s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2), 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 2, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2), 2)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.50s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 9, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2), 9)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.500000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, LEADING_0, 10, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(f + (f/2), 10)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h0m1.5000000000s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  // Test Years, Days, Hours, Seconds with LEADING_0 and NO_LEADING_0
  ticks = AC_SECS_PER_YEAR * f;
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(1y, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1y0d0h0m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = (AC_SECS_PER_DAY * f) + (AC_SECS_PER_MIN * f);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(1d0h1m, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y1d0h1m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, NO_LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(1d0h1m, NO_LEADING_0, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1d0h1m0.0s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  ticks = (AC_SECS_PER_MIN * f) + (2 * f) + (f / 10);
  ac_ticks_to_duration_str(ticks, LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(1m2.1s, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("0y0d0h1m2.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);
  ac_ticks_to_duration_str(ticks, NO_LEADING_0, 1, buff, AC_ARRAY_COUNT(buff));
  ac_printf("ac_ticks_to_duration_str(1m2.1s, NO_LEADING_0, 1)=%s\n", buff);
  error |= AC_TEST(ac_strncmp("1m2.1s", (char*)buff, AC_ARRAY_COUNT(buff)) == 0);

  return error;
}


int main(void) {
  ac_bool error = AC_FALSE;

  error |= test_time();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}