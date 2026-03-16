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
 
/* FIXME: The reading and writing information of EEPROM may be inaccurate. 
    There are a few things I don't understand when I read the NO $GBA document.
     ( Reference to some ideas of VBA-M source code \
     https://github.com/visualboyadvance-m/visualboyadvance-m/blob/master/src/gba/EEprom.cpp) */
#ifndef EEPROM_INL
#define EEPROM_INL 1

finline
void eeprom_init (struct eeprom *eeprom,  EEPROM_CAPACITY cap) {
  eeprom->address = 0;
  eeprom->agb = null;
  eeprom->bank = null;
  eeprom->bit = 0;
  eeprom->byte = 0;
  eeprom->capacity = cap;
  eeprom->status = EEPROM_FREE;

  if (cap == EEPROM_CAPACITY_512B) {
    eeprom->r_stop = 9;
  } else if (cap == EEPROM_CAPACITY_8K) {
    eeprom->r_stop = 17;
  } else {
    assert (0);
  }
  memset (& eeprom->packet[0], 0, sizeof (eeprom->packet));
#define EEPROM_RESET_VALUE 0
  memset  (& eeprom->memory[0], EEPROM_RESET_VALUE, sizeof (eeprom->memory));
}

finline 
uint16_t eeprom_read (struct eeprom *eeprom,  uint32_t address) {
  
  switch (eeprom->status) {
  case  EEPROM_READ_ACK:
    /* for read data messages, 
            the first four bits are useless data */
    if (++eeprom->bit >= 5) {
      const int calc=  eeprom->bit - 5;
      const int byte= calc >> 3;
      const int mod = calc & 7 ^ 7; /* MSB first */
      const int bit = eeprom->bank[byte] >> mod & 1;
      if (eeprom->bit == 68) {
        eeprom->status = EEPROM_FREE;
        eeprom->bit = 0;
      } 
      return bit;
    } else 
      return 0;
  default:
    return 1;
  }
  return 0;
}

finline 
void eeprom_write (struct eeprom *eeprom,  uint32_t address, uint16_t value) {
  
  switch (eeprom->status) {
  case EEPROM_READ_ACK:
    DEBUG_BREAK ();
    break;
  case EEPROM_FREE:
    assert (eeprom->bit <= 1);
    eeprom->cmd[eeprom->bit] = (uint8_t) value;
    if (eeprom->bit++ == 1) {
      if ( ( (*(uint16_t *) & eeprom->cmd[0]) & 0x0101) == 0x0101) {
        eeprom->status = EEPROM_SET_READ_ADDRESS;
        eeprom->_trunc = eeprom->r_stop - 1;
      } else {
        eeprom->status = EEPROM_SET_WRITE_ADDRESS;
        eeprom->_trunc = eeprom->r_stop - 2;
      }
    }
    break;
  case EEPROM_SET_READ_ADDRESS:
  case EEPROM_SET_WRITE_ADDRESS:
    /* 6 or 14 bit + end code */
    if (eeprom->bit == eeprom->_trunc) {
      if (eeprom->status == EEPROM_SET_WRITE_ADDRESS) {
        /* write next bit without buffer bit (e.g. read's 0) */
        eeprom->packet[eeprom->bit>> 3] <<= 1;
        eeprom->packet[eeprom->bit>> 3] |= value & 1;
        eeprom->status = EEPROM_WRITE_ACK;
      } else eeprom->status = EEPROM_READ_ACK;

      /* final, get memory access bank */
      if (eeprom->capacity == EEPROM_CAPACITY_8K)
        eeprom->bank_id = ((eeprom->packet[0] & 0x3F) * 256 | eeprom->packet[1]) & 0x3FF;   /* 14 bit only use low 10 bits */
      else 
        eeprom->bank_id = eeprom->packet[0] & 0x3F; /* 512 byte only have 64 banks */
      eeprom->address = eeprom->bank_id * 8;
      eeprom->bank = & eeprom->memory[eeprom->address];    
      eeprom->bit = 0;      
      break;
    } else {
      eeprom->packet[eeprom->bit>> 3] <<= 1;
      eeprom->packet[eeprom->bit>> 3] |= value & 1;
    }
    eeprom->bit++;
    break;
  case  EEPROM_WRITE_ACK:
    /* write 64 bit, 8 byte stream + 1 bit end code */
    if (eeprom->bit == 64) {
      eeprom->status = EEPROM_FREE;
      eeprom->bit = 0;
      break;
    } else {
      int byte = eeprom->bit >> 3;
      int mod = eeprom->bit & 7 ^ 7;
      if (value & 1)
        eeprom->bank[byte] |= 1 << mod;
      else 
        eeprom->bank[byte] &= ~(1 << mod);
    }
    eeprom->bit++;
    break;
  }
}

#endif 