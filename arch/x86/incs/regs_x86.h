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

#ifndef REGS_X86_H
#define REGS_X86_H

#include <ac_inttypes.h>
#include <descriptors_x86.h>

/** Set the GDT register from desc_ptr */
static __inline__ void set_gdt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("lgdt %0" :: "m" (*p));
}

/** Get the GDT register to desc_ptr */
static __inline__ void get_gdt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("sgdt %0" : "=m" (*p));
}

/** Set the LDT register from desc_ptr */
static __inline__ void set_ldt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("lldt %0" :: "m" (*p));
}

/** Get the LDT register to desc_ptr */
static __inline__ void get_ldt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("sldt %0" : "=m" (*p));
}

/** Set the IDT register from desc_ptr */
static __inline__ void set_idt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("lidt %0" :: "m" (*p));
}

/** Get the IDT register to desc_ptr */
static __inline__ void get_idt(descriptor_ptr* desc_ptr) {
  ac_u16* p = (ac_u16*)&desc_ptr->limit;
  __asm__ volatile("sidt %0" : "=m" (*p));
}

/** Set Task Register which is a 16 bit selector into the GDT */
static __inline__ void set_tr(ac_u16 selector) {
  __asm__ volatile("ltr %0;" :: "r" (selector));
}

/** Get Task Register which is a 16 bit selector into the GDT */
static __inline__ ac_u16 get_tr(void) {
  ac_u16 tr;
  __asm__ volatile("str %0;" : "=r" (tr));
  return tr;
}

/** Set data segment register */
static __inline__ void set_ds(ac_u16 ds) {
  __asm__ volatile("movw %0, %%ds;" :: "r" (ds));
}

/** Get data segment register */
static __inline__ ac_u16 get_ds(void) {
  ac_u16 ds;
  __asm__ volatile("movw %%ds, %0;" : "=r" (ds));
  return ds;
}

/** Set stack segment register */
static __inline__ void set_ss(ac_u16 ss) {
  __asm__ volatile("movw %0, %%ss;" :: "r" (ss));
}

/** Get stack segment register */
static __inline__ ac_u16 get_ss(void) {
  ac_u16 ss;
  __asm__ volatile("movw %%ss, %0;" : "=r" (ss));
  return ss;
}

/** Set extra segment register */
static __inline__ void set_es(ac_u16 es) {
  __asm__ volatile("movw %0, %%es;" :: "r" (es));
}

/** Get extra segment register */
static __inline__ ac_u16 get_es(void) {
  ac_u16 es;
  __asm__ volatile("movw %%es, %0;" : "=r" (es));
  return es;
}

/** get stack pointer */
static __inline__ ac_uptr get_sp(void) {
  ac_uptr sp;
#ifdef CPU_X86_64
  __asm__ volatile("movq %%rsp, %0;" : "=r" (sp));
#else /* CPU_X86_32 */
  __asm__ volatile("movl %%esp, %0;" : "=r" (sp));
#endif
  return sp;
}

/** get msr */
static __inline__ ac_u64 get_msr(ac_u32 msr) {
  ac_u32 lo;
  ac_u32 hi;
  __asm__ volatile("movl  %0, %%ecx\n" :: "g"(msr));
  __asm__ volatile("rdmsr\n" : "=d"(hi), "=a"(lo));
  return (ac_u64)hi << 32 | (ac_u64)lo;
}

/** interrupt instruction */
static __inline__ void intr(ac_u8 num) {
  __asm__ volatile("int %0;" :: "i"(num));
}

/** interrupt return instruction */
static __inline__ void iret(void) {
#ifdef CPU_X86_64
  __asm__ volatile("iretq");
#else /* CPU_X86_32 */
  __asm__ volatile("iret");
#endif
}


/** hlt, halt instruction */
static __inline void hlt(void) {
  __asm__ volatile("hlt");
}

/** cli, disable interrupts */
static __inline void cli(void) {
  __asm__ volatile("cli");
}

/** sti, enable interrupts */
static __inline void sti(void) {
  __asm__ volatile("sti");
}

#endif