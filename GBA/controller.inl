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
 
#ifndef CONTROLLER_INL
#define CONTROLLER_INL 1

finline
void controller_update (struct controller *controller) {
  
  uint16_t input;

   /* update keyinfos by host drv */
  input = controller->input.blk;
  controller->agb->platform_io.controller_drv (controller->agb, & controller->input);

  if (controller->ctl.blk & 0x4000) {
    int cnt;
    for (cnt =0; cnt != 10; cnt++) {
      int mask = 1 << cnt;
      /* check input bit selection */
      if (controller->ctl.blk & mask) {
        if (controller->ctl.blk & 0x8000) {
          /* interrupt - and mode */
          if (controller->input.blk & mask)
            return ;
            /* If it is a pulse signal rather than an indicator level, 
              this may not be possible on Windows, 
              which can only support the simultaneous pressing of four or so keybit pulses.
            */
       #ifdef GBA_CONTROLLER_INTERRUPT_PUSLE_GEN
          else if (!(input & mask))
            return ;
         #endif 
        } else {
          /* interrupt - or mode */
          if (!(controller->input.blk & mask)) {
       #ifdef GBA_CONTROLLER_INTERRUPT_PUSLE_GEN
            if (input & mask) 
              break;
       #else 
           break;
       #endif            
          }
        }
      }  
    }
    /* Check Interrupt */
    if (controller->ctl.blk & 0x3FF) {
      controller->agb->arm7.ifs.blk |= IRQ_REQUEST_KEYPAD;
      /* check resume halt */
      check_halt (controller->agb, IRQ_REQUEST_KEYPAD);
      /* check irq breakpoint */
      agb_irq_breakpoint (controller->agb, IRQ_REQUEST_KEYPAD);
      /* check resume stop */
      check_stop (controller->agb, IRQ_REQUEST_KEYPAD);
    }
  }
}

finline 
void controller_init_static (struct controller *controller) {
  memset (controller, 0x00, sizeof (struct controller)); 
}

finline 
uint16_t controller_read (struct controller *controller, uint32_t addr) {
  switch (addr & 0x3FF) {
  case 0x130: return controller->input.blk; break;
  case 0x132: return controller->ctl.blk; break;
  default: DEBUG_BREAK ();
  }
  return 0;
}

finline 
void controller_write (struct controller *controller, uint32_t addr, uint16_t value) {
  switch (addr & 0x3FF) {
  case 0x132: controller->ctl.blk = value; break;
  default: DEBUG_BREAK ();
  }
}

finline 
void controller_write_byte (struct controller *controller, uint32_t addr, uint8_t value) {
  switch (addr & 0x3FF) {
  case 0x132: controller->ctl.blk8[0] = value; break;
  case 0x133: controller->ctl.blk8[1] = value; break;
  default: DEBUG_BREAK ();
  }
}

finline
void controller_reset (struct controller *controller) {

  controller->input.blk = 0x3FF;
  controller->ctl.blk = 0;
  controller->phase = 0;
  controller->trigger = 0xFFFFFF;
}


#endif