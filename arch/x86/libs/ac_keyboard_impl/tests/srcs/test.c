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

#include <ac_keyboard_impl.h>

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_test.h>

ac_uint test_keyboard(void) {
  ac_printf("test_keyboard:+\n");
  ac_uint error = AC_FALSE;

#if defined(pc_x86_64)
#endif

  ac_printf("test_keyboard:-error=%d\n", error);
  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;

  ac_keyboard_init();

  if (!error) {
    error |= test_keyboard();
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}