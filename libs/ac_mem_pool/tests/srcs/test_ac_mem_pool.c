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

//#define NDEBUG

#include <ac_mem_pool.h>
#include <ac_mem_pool_dbg.h>
#include <ac_mem_pool_internal.h>
#include <ac_mem_pool/tests/incs/test.h>

#include <ac_printf.h>
#include <ac_debug_printf.h>
#include <ac_memset.h>
#include <ac_mpsc_fifo_dbg.h>
#include <ac_receptor.h>
#include <ac_test.h>
#include <ac_time.h>
#include <ac_thread.h>

extern AcMpscFifo* AcMemPool_get_fifo(AcMemPool* pool);

ac_bool simple_mem_pool_test() {
  ac_bool error = AC_FALSE;
  ac_debug_printf("simple_mem_pool_test:+\n");

  ac_u8* data;
  AcMemPool* mp;
  AcMem* mem;
  AcMem* mem2;

  AcMemPoolCountSize mpcs[1] = {
    { .count = 1, .data_size = 1 },
  };

  // Test BAD_PARAMS
  ac_debug_printf("simple_mem_pool_test: create a pool with NO AcMem's\n");
  error |= AC_TEST(AcMemPool_alloc(0, mpcs, &mp) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mp == AC_NULL);
  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mpcs), AC_NULL, &mp) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mp == AC_NULL);
  error |= AC_TEST(AcMemPool_alloc(AC_ARRAY_COUNT(mpcs), mpcs, AC_NULL) == AC_STATUS_BAD_PARAM);
  ac_debug_printf("\n");

  // Test requesting an AcMem from a AC_NULL pool returns AC_NULL
  ac_debug_printf("simple_mem_pool_test: bad params for AcMem_get_ac_mem\n");
  mem = (AcMem*)1;
  error |= AC_TEST(AcMemPool_get_ac_mem(AC_NULL, 1, &mem) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mem == AC_NULL);
  mem = (AcMem*)1;
  error |= AC_TEST(AcMemPool_get_ac_mem(AC_NULL, 0, &mem) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(mem == AC_NULL);
  error |= AC_TEST(AcMemPool_get_ac_mem((AcMemPool*)1, 1, AC_NULL) == AC_STATUS_BAD_PARAM);
  ac_debug_printf("\n");

  ac_debug_printf("simple_mem_pool_test: bad params for AcMem_get_mem\n");
  data = (void*)1;
  error |= AC_TEST(AcMemPool_get_mem(AC_NULL, 1, (void**)&data) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(data == AC_NULL);
  data = (void*)1;
  error |= AC_TEST(AcMemPool_get_mem(AC_NULL, 0, (void**)&data) == AC_STATUS_BAD_PARAM);
  error |= AC_TEST(data == AC_NULL);
  error |= AC_TEST(AcMemPool_get_mem((AcMemPool*)1, 1, AC_NULL) == AC_STATUS_BAD_PARAM);
  ac_debug_printf("\n");

  ac_debug_printf("simple_mem_pool_test: AC_NULL passed to AcMem_free, AcMem_ret_ac_mem and AcMem_ret_mem\n");
  AcMemPool_free(AC_NULL);
  AcMemPool_ret_ac_mem(AC_NULL);
  AcMemPool_ret_mem(AC_NULL);
  ac_debug_printf("\n");

  // Testing creating a pool with single size
  ac_debug_printf("simple_mem_pool_test: pool with one size data_size=%d\n",
      mpcs[0].data_size);
  error |= AC_TEST(AcMemPool_alloc(1, mpcs, &mp) == AC_STATUS_OK);
  error |= AC_TEST(mp != AC_NULL);
  AcMemPool_debug_print("simple_mem_pool_test: pool after creation, should have one entry:", mp);
  ac_debug_printf("\n");

  // Test getting an AcMem
  ac_debug_printf("simple_mem_pool_test: first get ac_mem, expecting != AC_NULL\n");
  error |= AC_TEST(AcMemPool_get_ac_mem(mp, 1, &mem) == AC_STATUS_OK);
  error |= AC_TEST(mem != AC_NULL);
  error |= AC_TEST(mem->hdr.user_size == 1);
  error |= AC_TEST(mem->data[0] == 0);
  AcMemPool_debug_print("simple_mem_pool_test: pool after first get, should be empty:", mp);
  ac_debug_printf("\n");

  // Test a second get fails
  ac_debug_printf("simple_mem_pool_test: second get ac_mem, expecting == AC_NULL\n");
  error |= AC_TEST(AcMemPool_get_ac_mem(mp, 1, &mem2) == AC_STATUS_NOT_AVAILABLE);
  error |= AC_TEST(mem2 == AC_NULL);
  AcMemPool_debug_print("simple_mem_pool_test: pool after second get:", mp);
  ac_debug_printf("\n");

  // Modify data and return the AcMem
  ac_debug_printf("simple_mem_pool_test: ret ac_mem, expecting == AC_NULL\n");
  mem->data[0] = 1;
  error |= AC_TEST(mem->data[0] == 1);
  AcMemPool_ret_ac_mem(mem);
  AcMemPool_debug_print("simple_mem_pool_test: pool after return, should have one entry:", mp);
  ac_debug_printf("\n");

  // Request it back and verify that data[0] is 0 and its not the same as mem
  ac_debug_printf("simple_mem_pool_test: re-getting ac_mem, expecting != AC_NULL\n");
  error |= AC_TEST(AcMemPool_get_ac_mem(mp, 1, &mem2) == AC_STATUS_OK);
  error |= AC_TEST(mem2 != AC_NULL);
  error |= AC_TEST(mem2 != mem);
  error |= AC_TEST(mem2->hdr.user_size == 1);
  error |= AC_TEST(mem2->data[0] == 0);
  AcMemPool_debug_print("simple_mem_pool_test: pool after re-getting, should be empty:", mp);
  ac_debug_printf("\n");

  ac_debug_printf("simple_mem_pool_test:-\n");
  return error;
}

/**
 * main
 */
int main(void) {
  ac_bool error = AC_FALSE;

  ac_thread_init(10);
  AcReceptor_init(256);
  AcTime_init();

  ac_debug_printf("sizeof(AcMem)=%d\n", sizeof(AcMem));

  error |= simple_mem_pool_test();
  //error |= simple_mem_pool_test(1);
  //error |= simple_mem_pool_test(127);
  //error |= test_mem_pool_multiple_threads(1, 1, 0);
  //error |= test_mem_pool_multiple_threads(1, 10, 1);
  //error |= test_mem_pool_multiple_threads(10, 1, 48);

  if (!error) {
    ac_printf("OK\n");
  }

  return error;
}
