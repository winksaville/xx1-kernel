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

#include <ac_inet_link.h>

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_receptor.h>
#include <ac_thread.h>
#include <ac_test.h>

ac_uint test_inet_link_impl(void) {
  ac_uint error = AC_FALSE;

  return error;
}

int main(void) {
  ac_uint error = AC_FALSE;

  ac_thread_init(3);
  AcReceptor_init(256);
  AcCompMgr* cm = AcCompMgr_init(3, 10, 0);
  error |= AC_TEST(cm != AC_NULL);

  AcInetLink_init(cm);

  if (!error) {
    error |= test_inet_link_impl();
  }

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
