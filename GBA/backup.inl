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
 
/*   kable rt_clock_support;
  kable crc32_support;
  SAVE_BACKUP backup_id;
  char title[64 + 1];
  uint32_t crc32; 
  const char *comment;
}; */

/* TODO: E, J, A C device crc done, RTC infos */
struct backup_infos backup_data [] = {
  { false, SAVE_BACKUP_EEPROM_512B, "CHOBITS-MMV1", "Chobits for Game Boy Advance - Atashi Dake no Hito (Japan)" },
  { false, SAVE_BACKUP_EEPROM_512B, "DRAGONBALLAA", "NULL" },
  
  /* Nintendo - Zelda */
  { false, SAVE_BACKUP_EEPROM_8K, "GBAZELDA MC", "1758 - The Legend of Zelda - The Minish Cap (E)(Independent)" },
  
  /* Nintendo - Golden Sun */
  /* Nintendo - Pokemon */

  
  { false, SAVE_BACKUP_FLASH_SANYO, "POKEMON FIRE", "Pokemon - Fire Red Version (U) (V1.1)" },
  { false, SAVE_BACKUP_FLASH_SANYO, "POKEMON EMER", "Pokemon - Emerald Version (U)" },
  { false, SAVE_BACKUP_FLASH_SANYO, "POKEMON RUBY", "Pokemon - Ruby Version (U) (V1.1)" },
  { false, SAVE_BACKUP_FLASH_SANYO, "POKE DUNGEON", "Pokemon - Mystery Dungeon Red Rescue Team" }, /* FIXME: No tests */
  /* Nintendo - Fire Emblem */
  /* Nintendo - Mario */
  
  
  
  
  /* Square Enix - Final Fantasy */
  
  
  /* Konami - Castlevania */
  
  /* Capcom - Gyakuten Saiban */
  /* Capcom - Rockman */
  /* Capcom - Street Fighter */
  
  /* SNK (MMV ?) The King Of Fighters */
  
  
  
  
  
  /* NBGI - Super Robot Taisen */
  { false, SAVE_BACKUP_FLASH_SST39LF_VF512, "SRWJ", "Super Robot Taisen J - Japan" },
  { false, SAVE_BACKUP_FLASH_SST39LF_VF512, "SRWD", "Super Robot Taisen D - Japan" },
  { false, SAVE_BACKUP_FLASH_SST39LF_VF512, "SRWR", "Super Robot Taisen R - Japan" },
  { false, SAVE_BACKUP_FLASH_SST39LF_VF512, "SRWA", "Super Robot Taisen A - Japan" },
  { false, SAVE_BACKUP_FLASH_SST39LF_VF512, "SRWOG", "Super Robot Taisen Original Generation - Japan" },
  { false, SAVE_BACKUP_FLASH_SST39LF_VF512, "SRWOG2", "Super Robot Taisen Original Generation 2 - Japan" }
};