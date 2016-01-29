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

#include <ac_poweroff.h>

extern void main(void);
extern void ac_init(void);

__attribute__ ((__noreturn__))
void ac_startup(void) {
  ac_init();
  main();
  ac_poweroff();

  /*
   * If poweroff didn't work, execute an undefined instruction.
   * This is particularly helpful as this will cause qemu to
   * terminate if we pass it the -no-reboot option
   */
  while (1) {
    __asm__ ("ud2");
  }
}