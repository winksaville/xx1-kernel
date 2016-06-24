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

#include <ac_mem_pool.h>

#include <ac_assert.h>
#include <ac_memmgr.h>
#include <ac_msg_pool.h>
#include <ac_mpscfifo.h>
#include <ac_msg.h>
#include <ac_debug_printf.h>

// The memory buffer and its message for storing in the msg_pool
// msg->arg1 == AcMemBuff*
typedef struct AcMemBuff {
  AcMsg* msg;
  ac_u8  mem[];
} AcMemBuff;

// Pool msg pool where each msg->arg1 points to a AcMemBuff
typedef struct AcMemBuffPool {
  AcMemBuffCountSize bcs;       // Max Count of mem buffers and size of each
  AcMsgPool* msg_pool;          // Pool of messages with msg->arg1 == AcMemBuff*
} AcMemBuffPool;

// Array of AcMemBuffPool one for each different buffer size
typedef struct AcMemPool {
  ac_u32 count;                        // Number of elements in mem_buff_pool_array
  AcMemBuffPool mem_buff_pool_array[]; // Array of AcMemBuffPool
} AcMemPool;

/**
 * Allocate a AcMemPool
 */
static AcMemPool* AcMemPool_alloc(ac_u32 count) {
  AcMemPool* pool = ac_calloc(1, sizeof(AcMemPool) + (count * sizeof(AcMemBuffPool)));
  if (pool != AC_NULL) {
    pool->count = count;
  }
}

/**
 * Free a AcMemPool
 */
static AcMemPool* AcMemPool_free(AcMemPool* pool) {
  ac_free(pool);
}

/**
 * Get memory from a pool
 *
 * @param pool is a previously created mem pool
 * @param size is the number of bytes of memory to get
 *
 * @return a ponter to memory or AC_NULL if none available
 */
void* AcMem_get(AcMemPool* pool, ac_u32 size) {
  ac_debug_printf("AcMem_get:+pool=%p\n", pool);

  void* mem = AC_NULL;

  if (pool == AC_NULL) {
    ac_debug_printf("AcMem_get: err pool=AC_NULL\n");
    mem = AC_NULL;
    goto done;
  }

  for (ac_u32 i = 0; i < pool->count; i++) {
    AcMemBuffPool* mem_buff_pool = &pool->mem_buff_pool_array[i];

    // Find first fit
    if (mem_buff_pool->bcs.size >= size) {
      AcMsg* msg = AcMsg_get(mem_buff_pool->msg_pool);
      AcMemBuff* mem_buff = (AcMemBuff*)msg->arg1;
      if ((mem_buff == AC_NULL) || (mem_buff->msg != msg)) {
        // Should NEVER happen, memory is corrupted!!!
        // TODO: ASSERT??
        ac_debug_printf("AcMem_get: err pool=%p "
            "(mem_buff=%p == AC_NULL) || mem_buff->msg != msg=%p)\n:",
            pool, mem_buff, mem_buff == AC_NULL ? AC_NULL : mem_buff->msg, msg);
        mem = AC_NULL;
        goto done;
      }
      mem = &mem_buff->mem;
      goto done;
    }
  }

done:
  ac_debug_printf("AcMem_get:-pool=%p mem=%p\n", pool, mem);
  return mem;
}

/**
 * Ret the memory to its pool. The mem must have been previously
 * created from a pool
 *
 * @param mem is a pointer to memory previously returned by AcMem_get
 */
void AcMem_ret(void* mem) {
  ac_debug_printf("AcMem_ret:+mem=%p\n", mem);

  if (mem == AC_NULL) {
    ac_debug_printf("AcMem_ret: err pool=? mem == AC_NULL\n");
    goto done;
  }

  AcMemBuff* mem_buff = (AcMemBuff*)(mem - sizeof(AcMsg*));
  AcMsg* msg = mem_buff->msg;
  if ((msg == AC_NULL) || ((ac_uptr)msg->arg1 != (ac_uptr)mem_buff)) {
    // Should NEVER happen, memory is corrupted!!!
    // TODO: ASSERT??
    ac_debug_printf("AcMem_ret: err mem=%p mem_buff=%p "
        "(mem_buff->msg=%p == AC_NULL) || (mem_buff->msg->arg1=%p != mem=%p)\n",
        mem, mem_buff, msg, msg != AC_NULL ? msg->arg1 : AC_NULL, mem);
    goto done;
  }

  AcMsg_ret(msg);

done:
  ac_debug_printf("AcMem_ret:-mem=%p\n", mem);
}

/**
 * Create a memory pool
 *
 * @params count is number of memory buffers for this pool
 *
 * @return AC_NULL if no pool is created.
 */
AcMemPool* AcMemPool_create(ac_u32 count, AcMemBuffCountSize* bcs) {
  ac_debug_printf("AcMemPool_create:+count=%d bcs=%p\n", count, bcs);

  ac_bool error;
  AcMemPool* pool = AC_NULL;

  // This fifo is used to save the AcMsg that point at the AcMemBuff
  ac_mpscfifo tmp_fifo;
  ac_mpscfifo_init(&tmp_fifo);
  

  if (count == 0) {
    ac_debug_printf("AcMemPool_create: err count is 0 return pool=AC_NULL\n");
    error = AC_TRUE;
    goto done;
  }

  // Allocate the AcMemPool
  pool = AcMemPool_alloc(count);
  ac_debug_printf("AcMemPool_create:+pool=%p\n", pool);
  if (pool == AC_NULL) {
    error = AC_TRUE;
    goto done;
  }

  // Allocate all of msg_pools which hold a AcMemBuff
  for (ac_u32 i = 0; i < pool->count; i++) {
    AcMemBuffPool* mem_buff_pool = &pool->mem_buff_pool_array[i];
    mem_buff_pool->bcs = bcs[i];

    // Allocate a AcMsgPool to hold the memory buffers
    AcMsgPool* msg_pool = AcMsgPool_create(mem_buff_pool->bcs.max_count);
    if (msg_pool == AC_NULL) {
      ac_debug_printf("AcMemPool_create: err creating pool[%d]\n", i);
      error = AC_TRUE;
      goto done;
    }
    
    // Allocate the memory buffers placing it on the tmp_fifo until
    // they are all initialized then we'll move the back to the pool
    for (ac_u32 m = 0; m < mem_buff_pool->bcs.max_count; m++) {
      // Allocate the AcMemBuff and its memory buffer that follows it
      AcMemBuff* mem_buff = ac_calloc(1, sizeof(AcMemBuff) + mem_buff_pool->bcs.size);
      if (mem_buff == AC_NULL) {
        // TODO: ASSERT Out of memory??
        ac_debug_printf("AcMemPool_create: err creating buffer[%d][%d]\n", i, m);
        error = AC_TRUE;
        goto done;
      }

      // Get a message from the pool
      AcMsg* msg = AcMsg_get(msg_pool);
      if (msg == AC_NULL) {
        // Should NEVER happen, memory is corrupted!!!
        // TODO: ASSERT??
        ac_debug_printf("AcMemPool_create: err msg AC_NULL initializing, Should not happen!\n");
        error = AC_TRUE;
        goto done;
      }

      // Initialize msg->arg1 to point to the mem_buff and
      // initialize mem_buff->msg to be the message.
      msg->arg1 = (ac_u64)mem_buff;
      mem_buff->msg = msg;

      // Add the msg to the tmp_fifo until all are initialzied.
      ac_mpscfifo_add_msg(&tmp_fifo, msg);
    }

    // Return the msgs which point to the mem_buff to its mem_buff_pool
    AcMsg* msg;
    while ((msg = ac_mpscfifo_rmv_msg(&tmp_fifo)) != AC_NULL) {
      AcMsg_ret(msg);
    }
  }

  error = AC_TRUE;

done:
  ac_debug_printf("AcMemPool_create:-count=%d bcs=%p pool=%p\n", count, bcs, pool);

  if (error) {
    // Return any msgs that are left on the tmp_fifo
    AcMsg* msg;
    while ((msg = ac_mpscfifo_rmv_msg(&tmp_fifo)) != AC_NULL) {
      ac_free((void*)msg->arg1);
      AcMsg_ret(msg);
    }

    if (pool != AC_NULL) {
      for (ac_u32 i = 0; i < count; i++) {
        AcMsgPool* msg_pool = pool->mem_buff_pool_array[i].msg_pool;
        // Free the AcMemBuff & its memory buffer
        while ((msg = AcMsg_get(msg_pool)) != AC_NULL) {
          if (msg->arg1 != 0) {
            ac_free((void*)msg->arg1);
            msg->arg1 = (ac_u64)AC_NULL;
          }
          ac_mpscfifo_add_msg(&tmp_fifo, msg);
        }
        // Return the msg back to its pool
        while ((msg = ac_mpscfifo_rmv_msg(&tmp_fifo)) != AC_NULL) {
          AcMsg_ret(msg);
        }
      }

      AcMemPool_free(pool);
      pool = AC_NULL;
    }
  }

  ac_mpscfifo_deinit(&tmp_fifo);

  return pool;
}