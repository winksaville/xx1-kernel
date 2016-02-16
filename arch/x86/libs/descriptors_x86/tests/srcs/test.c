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

#include <descriptors_x86.h>
#include <descriptors_x86_print.h>

#include <ac_printf.h>
#include <ac_test.h>

ac_bool test_gdt_ldt(void) {
  desc_ptr dp1;
  desc_ptr dp2;
  ac_bool error = AC_FALSE;

  // Get current GDT and verifiy we can write it
  // and read back the same value. Not a great test
  // but in the short term anything else would be
  // fatal.
  get_gdt(&dp1);
  set_gdt(&dp1);
  get_gdt(&dp2);

  error |= AC_TEST_EM(dp1.limit == dp2.limit,
      "Unable to get/set/get GDT register desc_ptr.limit");

  error |= AC_TEST_EM(dp1.sd == dp2.sd,
      "Unable to get/set/get GDT register desc_ptr.sd");

  // Get current LDT and verifiy we can write it
  // and read back the same value. Not a great test
  // but in the short term anything else would be
  // fatal.
  get_ldt(&dp1);
  set_ldt(&dp1);
  get_ldt(&dp2);

  error |= AC_TEST_EM(dp1.limit == dp2.limit,
      "Unable to get/set/get LDT register desc_ptr.limit");

  error |= AC_TEST_EM(dp1.sd == dp2.sd,
      "Unable to get/set/get LDT register desc_ptr.sd");

  return error;
}

int main(void) {
  ac_bool error = AC_FALSE;

  desc_ptr dp;
  get_gdt(&dp);
  print_desc_table("gdt", dp);

  error |= test_gdt_ldt();

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
