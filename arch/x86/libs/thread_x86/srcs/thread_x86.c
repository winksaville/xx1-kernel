/*
 * copyright 2015 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

#include <thread_x86.h>

#include <interrupts_x86.h>
#include <interrupts_x86_print.h>
#include <native_x86.h>

#include <ac_assert.h>
#include <ac_inttypes.h>
#include <ac_memmgr.h>
#include <ac_putchar.h>

//#define NDEBUG
#include <ac_debug_printf.h>

#define RESCHEDULE_ISR 0xfe

#define STATIC

// Forward declaractions
typedef struct tcb_x86 tcb_x86;
STATIC void* entry_trampoline(void* param) __attribute__ ((noreturn));
STATIC tcb_x86* next_tcb(tcb_x86* pcur_tcb);
tcb_x86* thread_scheduler(ac_u8* sp, ac_u16 ss, ac_u8* bp);

typedef struct tcb_x86 {
  ac_u32 thread_id;
  struct tcb_x86* pnext_tcb;
  void*(*entry)(void*);
  void* entry_arg;
  ac_u8* pstack;
  ac_u8* sp;
  ac_u16 ss;
  ac_u8* bp;
} tcb_x86;

typedef struct {
  ac_u32 max_count;
  tcb_x86 tcbs[];
} ac_threads;

#define DEFAULT_FLAGS 0x00200246

#define AC_THREAD_ID_EMPTY (ac_u32)-1
#define AC_THREAD_ID_STARTING (ac_u32)-2
#define AC_THREAD_ID_ZOMBIE (ac_u32)-3

/**
 * The idle thread
 */
STATIC ac_u8 idle_stack[AC_THREAD_STACK_MIN] __attribute__ ((aligned(64)));
STATIC tcb_x86 idle_tcb;

/**
 * The main thread
 */
STATIC tcb_x86 main_tcb;

/**
 * A NON-EMPTY circular linked list of tcbs to run,
 * it always contains at least the idle tcb, if nohting else.
 */
STATIC tcb_x86* pready;

/**
 * The list of threads which are waiting for some
 * type of event??
 */
STATIC tcb_x86* pwaiting_list;

/**
 * All of the threads
 */
STATIC ac_threads* pthreads;

struct saved_regs {
  ac_u64 rax;
  ac_u64 rdx;
  ac_u64 rcx;
  ac_u64 rbx;
  ac_u64 rsi;
  ac_u64 rdi;
  ac_u64 r8;
  ac_u64 r9;
  ac_u64 r10;
  ac_u64 r11;
  ac_u64 r12;
  ac_u64 r13;
  //ac_u64 r14;
  ac_u64 rbp;
};

struct full_stack_frame {
  union {
    struct saved_regs regs;
    ac_u64 regs_array[13];
  };
  struct intr_frame iret_frame;
} __attribute__ ((__packed__));

#define FULL_STACK_FRAME_SIZE 18 * sizeof(ac_u64)
ac_static_assert(sizeof(struct full_stack_frame) == FULL_STACK_FRAME_SIZE,
    "inital_stack_frame is not " AC_XSTR(FULL_STACK_FRAME_SIZE) " bytes in size");

/**
 * Print full stack frame
 */
STATIC void print_full_stack_frame(char* str, struct full_stack_frame* fsf) {
  if (str != AC_NULL) {
    ac_printf("%s:\n", str);
  }
  ac_printf(" rax=0x%lx\n", fsf->regs.rax);
  ac_printf(" rdx=0x%lx\n", fsf->regs.rdx);
  ac_printf(" rcx=0x%lx\n", fsf->regs.rcx);
  ac_printf(" rbx=0x%lx\n", fsf->regs.rbx);
  ac_printf(" rsi=0x%lx\n", fsf->regs.rsi);
  ac_printf(" rdi=0x%lx\n", fsf->regs.rdi);
  ac_printf(" r8=0x%lx\n", fsf->regs.r8);
  ac_printf(" r9=0x%lx\n", fsf->regs.r9);
  ac_printf(" r10=0x%lx\n", fsf->regs.r10);
  ac_printf(" r11=0x%lx\n", fsf->regs.r11);
  ac_printf(" r12=0x%lx\n", fsf->regs.r12);
  ac_printf(" r13=0x%lx\n", fsf->regs.r13);
  //ac_printf(" r14=0x%lx\n", fsf->regs.r14);
  ac_printf(" rbp=0x%lx\n", fsf->regs.rbp);
  print_intr_frame(AC_NULL, &fsf->iret_frame);
}

/**
 * Initialize the stack frame assuming no stack switching
 */
STATIC void init_stack_frame(ac_u8* pstack, ac_uptr stack_size, ac_uptr flags,
    void* (*entry)(void*), void* entry_arg, ac_u8** psp, ac_u16 *pss, ac_u8** pbp) {
  ac_u8* tos = pstack + stack_size;
  struct full_stack_frame* sf =
    (struct full_stack_frame*)(tos - sizeof(struct full_stack_frame));

  ac_printf("thread_x86 init_stack_frame: pstack=0x%p stack_size=0x%x, sf=0x%p flags=0x%x entry=0x%p entry_arg=0x%p\n",
     pstack, stack_size, sf, flags, entry, entry_arg);

  ac_static_assert(sizeof(void*) == sizeof(ac_uptr),
      "Assumption that void* is sizeof ac_uptr is false");

  // Be sure stack is on a 16 byte boundry
  ac_assert(((ac_uptr)pstack & 0xf) == 0);
  ac_assert(((ac_uptr)stack_size & 0xf) == 0);

  // fill the stack with its address for debugging
  ac_uptr* pfill = (ac_uptr*)pstack;
  for (ac_uint i = 0; i < stack_size / sizeof(ac_uptr); i++) {
    pfill[i] = (ac_uptr)&pfill[i];
  }

  print_full_stack_frame("thread_x86 init_stack_frame before init", sf);

  for (ac_uint i = 0; i < AC_ARRAY_COUNT(sf->regs_array); i++) {
    sf->regs_array[i] = i;
  }
  sf->regs.rbp = (ac_u64)&sf->regs.rbp;
  sf->regs.rdi = (ac_u64)entry_arg;
  sf->iret_frame.ip = (ac_uptr)entry;
  sf->iret_frame.cs = 0;
  sf->iret_frame.flags = flags;

  sf->iret_frame.sp = (ac_uptr)(tos);
  sf->iret_frame.ss = 0;

  print_full_stack_frame("thread_x86 init_stack_frame after init", sf);

  *psp = (ac_u8*)sf;
  *pss = sf->iret_frame.ss;
  *pbp = (ac_u8*)&sf->regs.rbp;
}

/**
 * Reschedule the CPU to the next ready thread. CAREFUL, this is
 * invoked by ac_init/srcs/ac_exception_irq_wrapper.S as such the
 * stack frame pointed to by sp must be identical.
 *
 * Interrupts will be disabled as the scheduler will be invoked
 */
INTERRUPT_HANDLER
void reschedule_isr(struct intr_frame* frame) {
  struct intr_frame* isf;
  struct full_stack_frame* fsf;
  ac_u8* new_bp;
  ac_u8* new_sp;

  isf = (struct intr_frame*)((ac_u8*)get_bp() + 8);
  fsf = (struct full_stack_frame*)((ac_u8*)get_bp() - (sizeof(struct saved_regs) - 8));

  ac_debug_printf("thread_x86 reschedule_isr:+cur pready=%p thread_id=%d size_sr=%d(0x%x)\n",
      pready, pready->thread_id, sizeof(struct saved_regs), sizeof(struct saved_regs));

  ac_debug_printf("thread_x86 reschedule_isr: sp=%p bp=%p &bp=%p &rax=%p isf=%p fsf=%p\n",
      get_sp(), get_bp(), &fsf->regs.rbp, &fsf->regs.rax, isf, fsf);
  print_full_stack_frame("thread_x86 reschedule_isr", fsf);

  tcb_x86 *ptcb = thread_scheduler((ac_u8*)get_sp(), get_ss(), (ac_u8*)get_bp());
  new_sp = ptcb->sp;
  new_bp = ptcb->bp;

  ac_debug_printf("thread_x86 reschedule_isr: new_sp=%p new_bp=%p\n", new_sp, new_bp);

  ac_debug_printf("thread_x86 reschedule_isr: new pready=%p thread_id=%d entry=%p entry_arg=%p\n",
      pready, pready->thread_id, pready->entry, pready->entry_arg);

  //set_sp(new_sp); // This is effectively a NOP because bp is used to reset the stack
  //set_bp(new_bp); // Switching stacks in an ISR seems to cause an #GP 13 (general protection fault)

  //fsf->iret_frame.ip = (ac_uptr)entry_trampoline; //pready->entry;
  //fsf->iret_frame.ip = (ac_uptr)pready->entry;
  //fsf->regs.rdi = (ac_uptr)pready;

  //isf = (struct intr_frame*)((ac_u8*)get_bp() + 8);
  //fsf = (struct full_stack_frame*)((ac_u8*)get_bp() - (sizeof(struct saved_regs) - 8));

  //ac_debug_printf("thread_x86 reschedule_isr: sp=%p bp=%p &bp=%p &rax=%p isf=%p fsf=%p\n",
  //    get_sp(), get_bp(), &fsf->regs.rbp, &fsf->regs.rax, isf, fsf);
  //print_full_stack_frame("thread_x86 reschedule_isr", fsf);
}



/**
 * A routine that calls the actual thread entry
 *
 * @param param is a pointer to the tcb_x86.
 *
 * @return AC_NULL
 */
STATIC void* entry_trampoline(void* param) {
  // Invoke the entry point
  tcb_x86* ptcb = (tcb_x86*)param;
  ac_printf("ptcb=%p\n", ptcb);
  ptcb->entry(ptcb->entry_arg);

  // Mark as zombie
  ac_u32* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, AC_THREAD_ID_ZOMBIE, __ATOMIC_RELEASE);

  intr(RESCHEDULE_ISR);

  while (AC_TRUE);
}

/**
 * Remove any zombie threads recoverying the stack
 * and the tcb.
 */
STATIC void remove_zombies(void) {
  tcb_x86* pzombie = AC_NULL;

  // Loop through the list of ready tcb removing
  // ZOMBIE entries. We'll start at idle_tcb as
  // the tail since its guaranteed to be on the
  // list and not a ZOMBIE.
  tcb_x86* ptail = &idle_tcb;
  tcb_x86* phead = idle_tcb.pnext_tcb;
  while (phead != &idle_tcb) {
    cli();
    if (phead->thread_id == AC_THREAD_ID_ZOMBIE) {
      // phead is a ZOMBIE, remove it from the
      // list advancing the head past it.
      pzombie = phead;
      phead = phead->pnext_tcb;
      ptail->pnext_tcb = phead;
    } else {
      // Not a ZOMBIE advance head and tail
      ptail = phead;
      phead = phead->pnext_tcb;
    }
    sti();

    if (pzombie != AC_NULL) {
      // Remove the ZOMBIE
      if (pzombie->pstack != AC_NULL) {
        ac_free(pzombie->pstack);
      }
      ac_u32* pthread_id = &pzombie->thread_id;
      __atomic_store_n(pthread_id, AC_THREAD_ID_EMPTY, __ATOMIC_RELEASE);
      pzombie = AC_NULL;
    }
  }
}

/**
 * Add an initialized tcb following pcur
 * * @param tcb to add to the ready list */
STATIC void add_after(tcb_x86* pcur, tcb_x86* pnew) {
  cli();
  pnew->pnext_tcb = pcur->pnext_tcb;
  pcur->pnext_tcb = pnew;
  sti();
}

/**
 * Idle routine, invoked if no other thread is ready.
 * It must never exit.
 */
STATIC void* idle(void* param) {
  while (AC_TRUE) {
    remove_zombies();
    // TODO: Goto a lower power state
  }
  return AC_NULL;
}

#define STATIC

/**
 * Initialize tcb it with the entry and entry_arg.
 *
 * @param entry routine to call to start thread
 * @param entry_arg is the argument to pass to entry
 *
 * Return AC_NULL if an error, i.e. nono available
 */
STATIC void tcb_init(tcb_x86* ptcb, ac_u32 thread_id, ac_u8* pstack,
    void*(*entry)(void*), void* entry_arg) {
  ptcb->entry = entry;
  ptcb->entry_arg = entry_arg;
  ptcb->pstack = pstack;
  ac_u32* pthread_id = &ptcb->thread_id;
  __atomic_store_n(pthread_id, thread_id, __ATOMIC_RELEASE);
}

/**
 * Get a tcb and initialize it with the entry and entry_arg.
 *
 * @param entry routine to call to start thread
 * @param entry_arg is the argument to pass to entry
 *
 * Return AC_NULL if an error, i.e. nono available
 */
STATIC tcb_x86* get_tcb(void*(*entry)(void*), void* entry_arg) {
  // Find an tcb_x86
  tcb_x86* ptcb;
  ac_bool found = AC_FALSE;
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    ptcb = &pthreads->tcbs[i];

    ac_u32 empty = AC_THREAD_ID_EMPTY;
    ac_u32* pthread_id = &ptcb->thread_id;
    ac_bool ok = __atomic_compare_exchange_n(pthread_id, &empty,
        AC_THREAD_ID_STARTING, AC_TRUE, __ATOMIC_RELEASE, __ATOMIC_ACQUIRE);
    if (ok) {
      tcb_init(ptcb, i, AC_NULL, entry, entry_arg);
      found = AC_TRUE;
      break;
    }
  }

  if (!found) {
    ptcb = AC_NULL;
  }
  return ptcb;
}


/**
 * Create a thread with a stack, and the initial flags
 *
 * If stack_size is <= 0 a "default" stack size will be used.
 *
 * If stack_size is > 0 it must be at AC_THREAD_MIN_STACK_SIZE
 * otherwise an error is returned and the thread is not created.
 *
 * Return 0 on success !0 if an error.
 */
STATIC tcb_x86* thread_create(ac_size_t stack_size, ac_uptr flags,
    void*(*entry)(void*), void* entry_arg) {
  tcb_x86* ptcb = AC_NULL;
  ac_u8* pstack = AC_NULL;
  int error = 0;

  // Allocate a stack
  if (stack_size <= 0) {
    stack_size = AC_THREAD_STACK_MIN;
  }
  if (stack_size < AC_THREAD_STACK_MIN) {
    error = 1;
    goto done;
  }
  stack_size = (stack_size + 0xff) & ~0xff;

  pstack = ac_malloc(stack_size);
  if (pstack == AC_NULL) {
    error = 1; // TODO: add AC_STATUS_OOM
    goto done;
  }
  ac_assert(((ac_uptr)pstack & 0xf) == 0);

  // Get the tcb and initialize the stack frame
  ptcb = get_tcb(entry, entry_arg);
  if (ptcb == AC_NULL) {
    error = 1; // TODO: add AC_STATUS_TO_MANY_THREADS
    goto done;
  }
  ptcb->pstack = pstack;
  init_stack_frame(pstack, stack_size, flags, entry_trampoline, ptcb,
      &ptcb->sp, &ptcb->ss, &ptcb->bp);

  // Add this after pready
  add_after(pready, ptcb);

done:
  if (error != 0) {
    if (pstack != AC_NULL) {
      ac_free(pstack);
    }
  }

  ac_debug_printf("thread_x86 thread_create: pstack=0x%p stack_size=0x%x tos=0x%p\n",
      pstack, stack_size, pstack + stack_size);
  ac_debug_printf("thread_x86 thread_create:-ptcb=0x%p thread_id=%d pready=0x%p next=0x%p\n",
      ptcb, ptcb->thread_id, pready, next_tcb(pready));

  return ptcb;
}

/**
 * Next tcb
 */
STATIC tcb_x86* next_tcb(tcb_x86* pcur_tcb) {
  // Next thread
  tcb_x86* pnext = pcur_tcb->pnext_tcb;

  // Skip any ZOMBIE threads it is ASSUMED the list
  // has at least one non ZOMBIE thread, the idle thread,
  // so this is guarranteed to not be an endless loop.
  ac_u32* pthread_id = &pnext->thread_id;
  ac_u32 thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  while(thread_id == AC_THREAD_ID_ZOMBIE) {
    // Skip the ZOMBIE
    pnext = pnext->pnext_tcb;
    pthread_id = &pnext->thread_id;
    thread_id = __atomic_load_n(pthread_id, __ATOMIC_ACQUIRE);
  }

  return pnext;
}

/**
 * Thread scheduler, for internal only, ASSUMES interrupts are DISABLED!
 *
 * @param pcur_sp is the stack of the current thread
 * @return the stack of the next thread to run
 */
tcb_x86* thread_scheduler(ac_u8* sp, ac_u16 ss, ac_u8* bp) {
  // Save the current thread stack pointer
  pready->sp = sp;
  pready->ss = ss;
  pready->bp = bp;

  // Get the next tcb
  pready = next_tcb(pready);
  if (pready == &idle_tcb) {
    // Skip idle once, if idle is the only thread
    // then we'll execute this time.
    pready = next_tcb(pready);
  }

  return pready;
}

/**
 * The current thread yeilds the CPU to the next
 * ready thread.
 */
void ac_thread_yield(void) {
  // Setup the stack frame exected by ac_reschedule
  // and branch to ac_reschedule
  __asm__ volatile(
      "mov $1, %rax;\n"
      "mov $2, %rbx;\n"
      "mov $3, %rsi;\n"
      "mov $4, %rdi;\n"
      "mov $5, %rbp;\n"
  );
  intr(RESCHEDULE_ISR);
}

/**
 * Early initialization of this module
 */
void ac_thread_early_init() {
  // Initialize reschedule isr
  set_intr_handler(RESCHEDULE_ISR, reschedule_isr);

  // Initialize the idle_stack
  ac_uptr idle_flags = DEFAULT_FLAGS;
  init_stack_frame(idle_stack, sizeof(idle_stack), idle_flags, idle, AC_NULL,
      &idle_tcb.sp, &idle_tcb.ss, &idle_tcb.bp);

  // Get the main tcb, no stack is needed because its
  // the one currently being used.
  tcb_init(&main_tcb, 0, AC_NULL, AC_NULL, AC_NULL);
  tcb_init(&idle_tcb, 1, idle_stack, idle, AC_NULL);

  // Empty waiting list
  pwaiting_list = AC_NULL;

  // Add main as the only tcb on pready to start
  main_tcb.pnext_tcb = &main_tcb;
  pready = &main_tcb;

  // Add idle after that
  add_after(pready, &idle_tcb);

  // TODO: Start a periodic timer for scheduling
  // ac_start_periodic_timer(1000); // Timer for scheduling

  ac_debug_printf("thread_x86 ac_thread_early_init:- pready=%p next=%p main_tcb=%p idle_tcb=%p\n",
      pready, next_tcb(pready), &main_tcb, &idle_tcb);
}

/**
 * Initialize this module
 */
void ac_thread_init(ac_u32 max_threads) {
  ac_assert(max_threads > 0);

  // Create array of the threads
  ac_u32 size = sizeof(ac_threads) + (max_threads * sizeof(tcb_x86));
  pthreads = ac_malloc(size);
  ac_assert(pthreads != AC_NULL);

  pthreads->max_count = max_threads;
  for (ac_u32 i = 0; i < pthreads->max_count; i++) {
    pthreads->tcbs[i].thread_id = AC_THREAD_ID_EMPTY;
    pthreads->tcbs[i].entry = AC_NULL;
    pthreads->tcbs[i].entry_arg = AC_NULL;
    pthreads->tcbs[i].pnext_tcb = AC_NULL;
    pthreads->tcbs[i].pstack = AC_NULL;
    pthreads->tcbs[i].sp = AC_NULL;
  }
  // Maybe early, but we don't know a good value?
  main_tcb.thread_id = max_threads;
  idle_tcb.thread_id = main_tcb.thread_id + 1;
}

/**
 * Create a thread and i:qnvoke the entry_p:w
 * oint passing
 * the parameter entry_arg.
 *
 * @param stack_size is <= 0 a "default" stack size will be used.
 *        If stack_size is > 0 it must be at AC_THREAD_MIN_STACK_SIZE
 *        otherwise an error is returned and the thread is not created.
 *
 * @return 0 on success !0 if an error.
 */
ac_u32 ac_thread_create(ac_size_t stack_size,
    void*(*entry)(void*), void* entry_arg) {
  tcb_x86* ptcb = thread_create(stack_size, DEFAULT_FLAGS, entry, entry_arg);
  return ptcb == AC_NULL ? 1 : 0;
}
