/* Copyripht 2019 moecmks (agalis01@outlook.com)
   This file is part of ArchBoyAdvance.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */
 
#ifndef TIMER_INL
#define TIMER_INL 1

#define TIMER0_RELOAD_COUNTER 0x100 
#define TIMER1_RELOAD_COUNTER 0x104
#define TIMER2_RELOAD_COUNTER 0x108
#define TIMER3_RELOAD_COUNTER 0x10C
#define TIMER0_CONTROL 0x102
#define TIMER1_CONTROL  0x106
#define TIMER2_CONTROL  0x10A
#define TIMER3_CONTROL  0x10E

static
void timer_write_ctl (struct timer *const timer, const uint32_t channel, const uint16_t value) {
  
  struct timer0 *const tm = & timer->component[channel];
  const uint32_t freq_lut[4] = { 1, 64, 256, 1024 };
  if (value & 0x80 && !(tm->control.blk & 0x80)) {
    /* open pulse, init status **/
    tm->counter.blk = tm->reload.blk;
    tm->phase = 0;
    tm->trigger = freq_lut[value & 3];
    tm->reload_adjust = tm->reload.blk;
    
    if ((value & 0x84) == 0x80)
      timer->igniter.item[channel] = 1;
    if ((value & 0x84) == 0x84)
      timer->igniter.item[channel] = 2;
    tm->overflow = tm->trigger * (0x10000 - tm->reload.blk);
  }
  if ((value & 0x03) != (tm->control.blk & 0x03)) {
    /* freq adjust pulse */
    tm->trigger = freq_lut[value & 3];
    tm->phase = 0;
    tm->reload_adjust = tm->reload.blk;
    tm->overflow = tm->trigger * (0x10000 - tm->reload.blk);
  }
  tm->control.blk = value;
  if ((tm->control.blk & 0x80) == 0)
    timer->igniter.item[channel] = 0;
}

static
uint16_t timer_adjust_counter (struct timer *const timer, const uint32_t channel) {
  
  struct timer0 *const tm = & timer->component[channel];
  if (tm->control.blk & 4)
    return tm->counter.blk;
  else 
    return tm->reload_adjust + tm->phase / tm->trigger;
}

finline
void timer_reset (struct timer *const timer) {

  timer->component[0].trigger = 1; 
  timer->component[1].trigger = 1;
  timer->component[2].trigger = 1; 
  timer->component[3].trigger = 1;

  timer->component[0].control.blk = 0; 
  timer->component[1].control.blk = 0;
  timer->component[2].control.blk = 0; 
  timer->component[3].control.blk = 0;

  timer->igniter.item[0] = 0;
  timer->igniter.item[1] = 0;
  timer->igniter.item[2] = 0;
  timer->igniter.item[3] = 0;
}

#endif