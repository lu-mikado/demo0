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
 
/*  ( Reference to some ideas of VBA-M source code and NO $GBA document\
     https://github.com/visualboyadvance-m/visualboyadvance-m/blob/master/src/gba/Flash.cpp) */
#ifndef FLASH_INL
#define FLASH_INL 1

finline
void flash_init (struct flash *flash,  SAVE_BACKUP backup_id) {
  flash->backup_id = backup_id;
  flash->cmd_loc = 0;
  flash->_64k_bankId = 0;
  flash->bank = flash->memory;
  flash->status = FLASH_FREE;

  switch (backup_id) {
  case SAVE_BACKUP_FLASH_SST39LF_VF512:
    flash->rd_waitstate = 2;
    flash->wr_waitstate = 3;
    flash->man = 0xbf;
    flash->dev = 0xd4;
    break;
  case SAVE_BACKUP_FLASH_AT29LV512:
    flash->rd_waitstate = 8;
    flash->wr_waitstate = 8;
    flash->man = 0x1f;
    flash->dev = 0x3d;
    break;
  case SAVE_BACKUP_FLASH_SANYO:
    flash->rd_waitstate = 12; /* FIXME: ??? */
    flash->wr_waitstate = 14;/* FIXME: ??? */
    flash->man = 0x62;
    flash->dev = 0x13;
    break;
  case SAVE_BACKUP_FLASH_MACRONIX_128K:
    flash->rd_waitstate = 6; /* FIXME: ??? */
    flash->wr_waitstate =15;/* FIXME: ??? */
    flash->man = 0xc2;
    flash->dev = 0x09;
    break;
  case SAVE_BACKUP_FLASH_MACRONIX_64K:
    flash->rd_waitstate = 3;
    flash->wr_waitstate = 8;
    flash->man = 0xc2;
    flash->dev = 0x1c;
    break;
  case SAVE_BACKUP_FLASH_PANASONIC:
    flash->rd_waitstate = 2;
    flash->wr_waitstate = 4;
    flash->man = 0x32;
    flash->dev = 0x1b;
    break;
  default:
    assert (0);
    break;
  }
#define FLASH_RESET_VALUE 0
  memset  (& flash->memory[0], FLASH_RESET_VALUE, sizeof (flash->memory));
}

dinline 
int32_t flash_read (struct flash *flash,  uint32_t address, uint8_t *const memory) {
  
  address &= 0xFFFF;
  flash->cmd_loc = 0;

  if (flash->status == FLASH_GET_DEVICE_HASH) {
    if (address == 0)
      *memory = flash->man;
    else if (address == 1)
      *memory = flash->dev;
    else 
      *memory = 0;
  } else if (flash->status == FLASH_ERASE_WRITE_SECTOR_ONLY_ATMEL) {
    flash->status = FLASH_FREE;
    *memory =  flash->bank[address];
  } else 
  *memory =  flash->bank[address];
  return flash->rd_waitstate;
}

dinline 
int32_t flash_write (struct flash *flash,  uint32_t address, uint8_t value) {

  address &= 0xFFFF;
  if (flash->status == FLASH_ERASE_WRITE_SECTOR_ONLY_ATMEL
    || flash->status == FLASH_WRITE_ONE_BYTE
    || flash->status == FLASH_BANK_SWITCH)
  {
    if (flash->status == FLASH_BANK_SWITCH
      && address == 0
      && (flash->backup_id == SAVE_BACKUP_FLASH_SANYO || flash->backup_id == SAVE_BACKUP_FLASH_MACRONIX_128K))
    {
      /* switch 64K bank */
      flash->bank = & flash->memory[(value & 1) * 0x10000];
      flash->status = FLASH_FREE;
    }
    else if (flash->status == FLASH_WRITE_ONE_BYTE
      || (flash->status == FLASH_ERASE_WRITE_SECTOR_ONLY_ATMEL) )
    {
      flash->bank[address] = value;
      if (flash->status == FLASH_ERASE_WRITE_SECTOR_ONLY_ATMEL)
        flash->block_cnt ++;
    }
    
    if (flash->status == FLASH_WRITE_ONE_BYTE)
      flash->status = FLASH_FREE;
    
    if (flash->status == FLASH_ERASE_WRITE_SECTOR_ONLY_ATMEL
      && (flash->block_cnt == 128 || (address & 0x7F) == 0x7F)) /* sector's 127 byte block last write ?? */
    {
      flash->block_cnt = 0;
      flash->status = FLASH_FREE;
    }
  }
  else 
  {
    /* ext command for macronix */
    if (address == 0x5555
      && value == 0xF0
      && flash->cmd_loc == 0
      && (  flash->backup_id == SAVE_BACKUP_FLASH_MACRONIX_128K || 
                   flash->backup_id == SAVE_BACKUP_FLASH_MACRONIX_64K)
      && flash->status == FLASH_FREE)

    {
      /* end timeout command */
      flash->status = FLASH_FREE;
    }

    /* fisrt comannd byte */
    if (address == 0x5555
      && value == 0xAA)
    {
      /* first write or adv command*/
      if (flash->cmd_loc == 0
        && flash->status == FLASH_ERASE_COMMAND_ENTRY)
      {
        /* adv command */
      }
      else  
      {
        /* reset command */
        flash->status = FLASH_FREE;
      }
      flash->cmd_loc = 1;
    }
    /* second command byte */
    else if (address == 0x2AAA
      && value == 0x55
      && flash->cmd_loc == 1)
    {
      flash->cmd_loc = 2;
    }
    /* final byte, and get command id */
    else if (address == 0x5555
      && flash->cmd_loc == 2)
    {
      switch (value) {
      case 0x90: /* enter id mode */
        flash->status = FLASH_GET_DEVICE_HASH;
        break;
      case 0xF0: /* end id mode */
        flash->status = FLASH_FREE;
        break;
      case 0x80: /* erase base command entry */
        flash->status = FLASH_ERASE_COMMAND_ENTRY;
        break;
      case 0x10: /* erase entrie chip (or bank ??... emm..) */
        assert (flash->status == FLASH_ERASE_COMMAND_ENTRY);
#if 1
        assert (flash->bank != null);
        memset (flash->bank, 0xFF, 0x10000);
#else 
        memset (flash->memory, 0xFF, 0x20000);
#endif 
        break;
      case 0xA0: /* write single byte  or write 128 bytes page(128 only for atmel)*/
        if (flash->backup_id != SAVE_BACKUP_FLASH_AT29LV512)
          flash->status = FLASH_WRITE_ONE_BYTE;
        else 
          flash->status = FLASH_ERASE_WRITE_SECTOR_ONLY_ATMEL;
        flash->block_cnt = 0;
        break;
      case 0xB0: /* switch 64k bank */
        flash->status = FLASH_BANK_SWITCH;
        break;
      default:
        FORCE_BREAK ();
        break;
      }
      flash->cmd_loc = 0;
    }
    /* final byte - level 2 for device (except atmel)  */
    else if (flash->cmd_loc == 2 /* is last byte ?? */
      && flash->backup_id != SAVE_BACKUP_FLASH_AT29LV512 /* not atmel device ?? */
      && flash->status == FLASH_ERASE_COMMAND_ENTRY /* is FLASH_ERASE_COMMAND_ENTRY ?? */
      && (address & 0x0FFF) == 0) /* is 4k page start address ?? */
    {
      /* erase - 4KB region (except atmel) */
      memset (& flash->bank[address & 0xF000], 0xFF, 0x1000);
      
      flash->status = FLASH_FREE;
      flash->cmd_loc = 0;
    }
  }
  return flash->wr_waitstate;
}

#endif 