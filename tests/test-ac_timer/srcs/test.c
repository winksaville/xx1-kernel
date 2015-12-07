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

#include <ac_timer.h>

#include <ac_test.h>
#if defined(VersatilePB)
#include <ac_interrupts.h>
#endif

ac_bool test_ac_timer() {
#if defined(Posix)
  ac_printf("test_ac_timer: no tests for Posix\n");
  return AC_FALSE;
#elif defined(VersatilePB)
  ac_bool error = AC_FALSE;

  ac_u32 count = ac_timer_get_count();
  ac_printf("test_ac_timer: count=%x(%u)\n", count, count);

  for (ac_u32 i = 0; i < count; i++) {
    ac_u32 value = ac_timer_rd_value(i);
    ac_u32 load = ac_timer_rd_load(i);
    ac_u32 control = ac_timer_rd_control(i);
    ac_u32 ris = ac_timer_rd_ris(i);
    ac_u32 mis = ac_timer_rd_mis(i);
    ac_u32 bgload = ac_timer_rd_bgload(i);
    ac_printf("test_ac_timer: value=0x%x(%u)\n", value, value);
    ac_printf("test_ac_timer: load=0x%x(%u)\n", load, load);
    ac_printf("test_ac_timer: control=0x%x(%u)\n", control, control);
    ac_printf("test_ac_timer: ris=0x%x(%u)\n", ris, ris);
    ac_printf("test_ac_timer: mis=0x%x(%u)\n", mis, mis);
    ac_printf("test_ac_timer: bgload=0x%x(%u)\n", bgload, bgload);
  }

  ac_u32 cur_value = 0;
  ac_timer_free_running(1);
  for (ac_u32 i = 0; i < 20; i++) {
    cur_value = ac_timer_rd_free_running(1);
    ac_printf("test_ac_timer: new free_running_value=0x%x(%u)\n",
          cur_value, cur_value);
  }

  cur_value = 0;
  ac_timer_one_shot(2, 1000);
  for (ac_u32 i = 0; i < 20; i++) {
    cur_value = ac_timer_rd_value(2);
    ac_u32 timer_ris = ac_timer_rd_ris(2);
    ac_u32 timer_mis = ac_timer_rd_mis(2);
    ac_u32 irq_status = ac_interrupts_rd_irq_status();
    ac_u32 fiq_status = ac_interrupts_rd_fiq_status();
    ac_u32 ris_status = ac_interrupts_rd_ris_status();
    ac_u32 int_select = ac_interrupts_rd_int_select();
    ac_u32 int_enable = ac_interrupts_rd_int_enable();
    ac_printf("test_ac_timer: new cur_value=0x%x(%u) timer_ris=%x(%x) timer_mis=%x(%x)\n",
        cur_value, cur_value, timer_ris, timer_ris, timer_mis, timer_mis);
    ac_printf("test_ac_timer: irq_status=0x%x(%u) fiq_status=%x(%x) ris_status=%x(%x)\n",
        irq_status, irq_status, fiq_status, fiq_status, ris_status, ris_status);
    ac_printf("test_ac_timer: int_select=0x%x(%u) int_enable=%x(%x)\n",
        int_select, int_select, int_enable, int_enable);
  }

  //cur_value = 0;
  //ac_timer_periodic(0, 1000);
  //for (ac_u32 i = 0; i < 10000; i++) {
  //  cur_value = ac_timer_rd_value(0);
  //  //ac_printf("test_ac_timer: new cur_value=0x%x(%u)\n",
  //  //    cur_value, cur_value);
  //}


  return error;
#else
  ac_printf("test_ac_timer: Unknown Platform\n");
  return AC_FALSE;
#endif
}

int main(void) {
  if (test_ac_timer()) {
      // Failed
      ac_printf("ERR\n");
      return 1;
  } else {
      // Succeeded
      ac_printf("OK\n");
      return 0;
  }
}

