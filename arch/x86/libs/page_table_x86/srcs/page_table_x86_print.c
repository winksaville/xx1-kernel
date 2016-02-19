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

#include <page_table_x86_print.h>

#include <ac_inttypes.h>
#include <ac_printf.h>

void print_cr3_nrml_paging_fields(char* str, ac_uint cr3) {
  union cr3_paging_fields_u reg = { .raw = cr3 };

  ac_printf("%s: 0x%x\n", str, reg.raw);
  ac_printf(" pwt=%d\n", reg.nrml_paging_fields.pwt);
  ac_printf(" pcd=%d\n", reg.nrml_paging_fields.pcd);
  ac_printf(" page_directory_base=0x%x\n", reg.nrml_paging_fields.page_directory_base);
}

void print_cr3_pae_paging_fields(char* str, ac_uint cr3) {
  union cr3_paging_fields_u reg = { .raw = cr3 };

  ac_printf("%s: 0x%p\n", str, reg.raw);
  ac_printf(" page_directory_base=0x%x\n", reg.pae_paging_fields.page_directory_base);
}

void print_cr3_pcide_paging_fields(char* str, ac_uint cr3) {
  union cr3_paging_fields_u reg = { .raw = cr3 };

  ac_printf("%s: 0x%p\n", str, reg.raw);
  ac_printf(" page_directory_base=0x%x\n", reg.pae_paging_fields.page_directory_base);
}

void print_pml4e_fields(char* str, ac_u64 val) {
  union pml4e_fields_u reg = { .raw = val };

  ac_printf("%s: 0x%llx\n", str, reg.raw);
  ac_printf(" p=%d\n", reg.fields.p);
  ac_printf(" rw=%d\n", reg.fields.rw);
  ac_printf(" us=%d\n", reg.fields.us);
  ac_printf(" pwt=%d\n", reg.fields.pwt);
  ac_printf(" pcd=%d\n", reg.fields.pcd);
  ac_printf(" a=%d\n", reg.fields.a);
  ac_printf(" reserved_0=0x%llx\n", reg.fields.reserved_0);
  ac_printf(" phy_addr=0x%llx\n", reg.fields.phy_addr);
  ac_printf(" xd=%d\n", reg.fields.xd);
}

/**
 * Print the page table whose top most directory is
 * table and the mode is one of the three modes supported
 * by x86 cpus.
 */
void print_page_table(union cr3_paging_fields_u cr3u, enum page_mode mode) {
  ac_printf("page directory addr=0x%p mode=%d\n",
      get_page_directory_addr(cr3u, mode), mode);

}
