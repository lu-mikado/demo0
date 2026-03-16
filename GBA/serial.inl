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
 
#ifndef SERIAL_INL
#define SERIAL_INL 1

finline
void sio_tick (struct serial *sio, int32_t delta) {

}
finline 
void sio_init_static (struct serial *sio) {
  memset (sio, 0x00, sizeof (struct serial)); 
}

finline 
uint16_t sio_read (struct serial *sio, uint32_t addr) {
  return 0;
}

finline 
void sio_write (struct serial *sio, uint32_t addr, uint16_t value) {
}

finline 
void sio_reset (struct serial *sio) {

}

#endif