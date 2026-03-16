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

/* GPU Graphics Explanation 
 * The GPU of GBA is very complex. 
 * I'll try to explain something, if you can understand it.
 *
 * The display memory of GBA is 96KB, the first 64KB is used for map scrolling, 
 * and the latter 32K has been used for sprite rendering (OAM). 
 *
 * GBA has four scroll (Bg0~Bg3) renderers and behaves differently in different modes
 * There are six modes for map scanning 
 *
 * Mode-0 Alive BG : Bg0 Bg1 Bg2 Bg3 - 2D Mapper Base NES/CGB Tile extend (4Bit/8Bit Switchable)  
 * Mode-1 Alive BG : Bg0 Bg1 Bg2(2 is AffineBG), Bg3 unused 
 * Mode-2 Alive BG : Bg2 Bg3(2, 3 is AffineBG) Bg0, Bg1 unused 
 * Mode-3 240x160 pixels, 32768 colors 75KB (Only Bg2 alive) 
 * Mode-4 240x160 pixels, 255 colors 2 frame (Only Bg2 alive) 
 * Mode-5 160x128 pixels, 32768 colors 2 frame (Only Bg2 alive) 

 * Imaging principle of map
 * Maps can display four types of pixels 
 * PT 0: 16 PAL (Mode-0~Mode-2)
 * PT 1: Common 256 PAL (Mode-0~Mode-2)
 * PT 2: rgb15 pixel (Only for Mode-3 and Mode-5)
 * PT 3: Special 256 PAL (Mode-4)
 
 * The map and the sprite each have a readable and writable rgb15 palette indexing 256 colors palette 
 * color format : d15                  d14       ....          d0 
 *                unused               bbbbb   ggggg rrrrr
 *
 
 * PT0, PT1 Render (Not Affine Mode) 
 * In this mode, the minimum operating unit of a graph is an 8*8 set of small tile pixels, 
 * not a single pixel, as we often call it "Tile".
 *
 * a Tile := 8*8 Pixels set 
 * This Tile takes 32 bytes in 4Bit palette mode
 * 64 bytes under 8Bit palette
 *
 *                             Pos 0  .........                Pos 7
 * A 8Bit Mode's Tile Decode:= Line 0 Start:= Byte0  ... End Byte7
                                 .
                                 .
                                 .
                               Line 4 Start:= Byte32  ... End Byte39 
                                 .
                                 .
                               Line 7 Start:= Byte56  ... End Byte63
                               
                               Pos 0  .........                Pos 7
 * A 4Bit Mode's Tile Decode:= Line 0 Start:= Byte0  ... End Byte3 (Per Byte's Low4Bit for left pos, other 4bit for right pos, 
                                 .                                     e.g. Byte 3, low4Bit for pos 6 high_4bit for pos7 )
                                 .
                                 .
                                 .
                               Line 4 Start:= Byte16  ... End Byte19
                                 .
                                 .
                               Line 7 Start:= Byte29  ... End Byte31
                       
 * For 8Bit Mode, Directly indexed to the color value of the 15-bit palette, (No. 0 palette is always transparent, Even in 4bit mode)
 * Fot 4Bit Mode, The palette color that starts with base_16 (4Bit nums) mul 16 add 16-bit index is indexed 
 * 4Bit Mode addressing e.g. base_16:= 5 4bit:= 3 result pixel := bg_palette[5*16+3] 
 *                  bg_palette[base_16]'s Pixel is always transparent (This base_16 will be explained in the next stage.)
   
 * the display memory has something called "Tile Id/attr table".
 * We simply call it "NameTable" (Same as NES)
 *
 * NameTable a unit has two bytes
 *
 * NameTable's 2 Bytes attr (Not Affine Mode)
 *
 *     0-9   Tile ID    
 *      10    Horizontal Flip 
 *      11    Vertical Flip   
 *      12-15 Palette Number  base_16 4bit for 4bit mode render (8bit mode unused, Ignore It!)
 *
 * The GPU can address an 8*8 pixel tile block based on the Tile ID number inside the naming table, 
 * and then do a little flip according to the set of the other bit domains, 
 * or the programmer can rewrite base_16 to switch 16 bit palette slots (if some flickering effect is made), if it is in 4bit mode. 

 * the display memory has something called "Character Table".
 * We simply call it "ChrTable" (Same as NES)
 *
 * This is where the 8bit/4bit palette index is stored.
 *
 * for 8bit mode addressing char index := base_char_address + tid *64 (Per 8bit mode tile block have 64bytes for 8*8 pixel) 
 * for 4bit mode addressing char index := base_char_address + tid *32 (Per 4bit mode tile block have 32bytes for 8*8 pixel)  
 * base_char_address are provided by GPU registers

 * Map Scroll and Nametable 
 * The unit size of a map in each map rendering channel is 256*256 display Pixels 
 * Maximum four units can be set (512*512 Pixels), only for 4 mode Selectable 
 
 * Type 0 256 *256 pixels (Single Screen Mapper), 32*32 Tile (2K NameTable Bytes) 
   Type 1 512 *256 pixels (Horizontal Double Screen), 64*32 Tile (4K NameTable Bytes) 
   Type 2 256 *512 pixels (Vertical Double Screen), 32*64 Tile (4K NameTable Bytes) 
   Type 3 512 *512 pixels (Quadruple Screen), 64*64 Tile (8K NameTable Bytes) 
    
   For Type 0 Scroll Exceed 256, Backup to pos 0 (both Horizontal Scroll, Vertical Scroll)
    
   For Type 1 Vertical Scroll Exceed 256, Backup to pos 0
               Horizontal Scroll Exceed 512, Backup to pos 0
               
   For Type 2 Horizontal Scroll Exceed 256, Backup to pos 0
               Vertical Scroll Exceed 512, Backup to pos 0 
               
   For Type 3 Horizontal Scroll Exceed 512, Backup to pos 0
               Vertical Scroll Exceed 512, Backup to pos 0   
    
   Address Mapper := BaseAddress + Unit ID *2K (Base addresses are provided by GPU registers)
    
   Type 0: Only 1 Unit 
   Type 1: 2 Units, left unit id:= 0 right unit id:= 1 
   Type 2: 2 Units, top unit id:= 0 bottom unit id:= 1 
    
   Type 3: 4 Units, left-top unit id:= 0 right-top unit id:= 1  
                     left-bottom unit id:= 2 right-bottom unit id:= 3
  
 * Sprite 
 * All 32K memory used is a character table 
 * Another 1K memory is used to describe the properties of the Sprites. 
 * Whether in 4bit/8bit mode, the Sprite's Tile * ID addressing is only * 32 
 * For the 8 bit palette, his lsb will be ignored (& - 2) 
  
 * e.g. 8bit mode 
        char_base := 0x10000 
        tile id:= 126
        char address := 0x10000 + 126*32
   
   
 * In addition, Sprite's other feature is that many tiles can be glued together to 
    form a large collection of Tiles (that is, the complete console game character pixels).

    Tile id always represents the set's first Tile in the upper left corner
      
      Horizontal increase := 8Bit +2, 4Bit +1
      
      The remaining Tiles's ID can be addressed in two ways
      
      1: Fixed Pitch Mapper Addressing. 
         Pitch Always 32 in 4bit/8bit mode .
         
         e.g. 0 1 2 3 4 5 6 7 line 1
              32 33 34 35 ..39 line 2 (this is 4bit mode, for 8bit Next incremental addressing in each horizontal direction+2)
              
         e.g. Tile Block Set := 2*4 Tiles (16 *64 Pixels)
              Tile Id := 252
              8Bit mode 
              
              Pos (1, 2)'s ID := 2*32 (Y Pos 2 , Mul Pitch) 
                           +     1*2  (X Pos 1 , * 2 [8Bit per +2])
                           +     252  (Base Tile ID) 
                                             
         e.g. Tile Block Set := 4*4 Tiles (16 *64 Pixels)
              Tile Id := 177
              4Bit mode 
              
              Pos (3, 3)'s ID := 3*32 (Y Pos 3 , Mul Pitch) 
                           +     3*1  (X Pos 1 , * 1 [4Bit per +1])
                           +     177  (Base Tile ID)                                
         
      2: Linear  Mapper Addressing. 
         
         Even on the next line, TileID is only +2, +1 each time.
         
         e.g. 1 2 3 4 5 6 7 line 1
              8 9 10 ......15 line 2 (this is 4bit mode, for 8bit Next incremental addressing in each horizontal direction+2)
  
         e.g. Tile Block Set := 4*8 Tiles (16 *64 Pixels)
              Tile Id := 66
              8Bit mode 
              
              Pos (0, 1)'s ID := 2*8 (Y Pos 1 , To Next line, 8+1*2) 
                           +     0  (X Pos 0 nodone)
                           +     66  (Base Tile ID)          
      

      * Sprite Affine 
      * The direction of the coordinate vector of the fairy affine is 
      *                                 
                       Y Negtive
                      /|\ 
                      
                       |
                       |
    X Negtive <-  -----|-----  -> X Postive 
                       |\
                       | \_____________ Origin (0, 0) is Sprite's central coordinate
                      
                      \|/
                       Y Postive
                    
    For affine, Sprite only needs these four parameters              
    The direction of expansion of vectors increases from the center to the outside
    
    PA - dx Incremental position vector for each horizontal pixel (X)
    PB - dmx Incrementally incremental position vectors for each switch to the next row of pixels (X)
    PC - dy Incremental position vector for each horizontal pixel (Y)
    PD - dmy Incrementally incremental position vectors for each switch to the next row of pixels (Y)

    e.g. dx := 1
         dmy := 1
         dy := 0
         dmx := 0 <-------------------- Standard render 
         
    e.g. dx := -1
         dmy := 1
         dy := 0
         dmx := 0 <-------------------- Horizontal Flip render         
         
    e.g. dx := 1
         dmy :=-1
         dy := 0
         dmx := 0 <-------------------- Vertical Flip render       
         
    (Note that the last 8 bits of the parameters in GBA are all non-integers (used to simulate decimals))
    
    The matrix expression is as follows (from Nintendo's official programming manual)
    
    0: Center pos 
    1: Origin Pos
    2: New Pos
    a: x direction pixel stretch 
    b: y direction pixel stretch 
    
    | x2 |     | PA PB |   |x1 - x0|   |x0|
    |    | :=  |       | * |       | + |  |
    | y2 |     | PC PD |   |y1 - y0|   |y0|
 
          1
    PA := cos(Angle)
          a
 
          1
    PB := sin(Angle)
          a
     
         -1
    PC :=  sin(Angle)
          b 
    
          1
    PD :=  cos(Angle)
          b 

    Pixels that rotate beyond their original coordinate range are clipped.
    
     * Double Sprite 
     * X and Y are double canvas size
     * Note that this adds only the area of the canvas and does not magnify the original sprite pixels.
     * If in Double Sprite mode, position is unchanged, the coordinates of the center point will be added half the size of the X and Y vectors.
    
     * e.g. 
    
      dx := 1
      dmy := 1
      dy := 0
      dmx := 0 <-------------------- Standard render 

      pos x:= 128 
      pos y:= 128  

      sprite size := 64, 64 

      middle point := 160, 160 
      canvas range x:= 128~192
      canvas range y:= 128~192   

      double it ! 

      pos x:= 128 
      pos y:= 128  

      sprite size := 64, 64 
      middle point := 192, 192 
      canvas range x:= 128~256
      canvas range y:= 128~256

      BUG:
      In Double Sprite Mode 

      If the sprite is in the scanning line 128-160,
      This part of the pixel sprite will not be displayed, of course, 
      rest pixel of the this a sprite is normal displayed.
     
      
     */

     /* drawsolid status and pri
         winobj -> winout, winobj draw 
         win0-> winout, winobj, win1, win0 
         win1-> winout, winobj, win1,
         winout-> only winout.

         pri 0 win0 
         pri 1 win1 
         pri 2 winobj 
         pri 3 winout 
      */

      /* TODO: mosaic and more mode */









#ifndef GPU_INL
#define GPU_INL 1

/* TODO: OAM 128*128 Pixel BUG, (for Bios Boot dra3w logo)*/
/* TODO: Imm Write Pixel when pixel is backdrop */
struct rasterizer_caps {
  uintptr_t order; 
  uintptr_t sub_order;
  uintptr_t chanId;
  uintptr_t chr_base;
  uintptr_t opca; /* 0: normal filter 1: alpha 2: bright_inc 3: bright_dec */
  uintptr_t winId;
  uint16_t solid;
  uintptr_t interp_yvec;
  uintptr_t interp_xvec;
  kable interp_x;
  kable interp_y;
  struct gpu_channel *chan;
  struct gpu *gpu;
  uintptr_t ntbank[4];
};

finline void 
bg_mosaic (struct rasterizer_caps *rtc, uint16_t attr_mask) {
  if (rtc->chan->ctl.blk >> 6 & 1) {
    uintptr_t xvec = rtc->gpu->mosaic.blk & 15;
    uintptr_t yvec = rtc->gpu->mosaic.blk >> 4 & 15;
  }
}

finline void 
sp_mosaic (uint16_t attr_mask, uint16_t x_pos, uint16_t vec) {
}

finline 
void nt_bank_settings (struct rasterizer_caps *rtc) {
  uintptr_t nt_base =  (rtc->chan->ctl.blk >> 8 & 31) * 0x800;
  switch (rtc->chan->ctl.blk & 0xC000) {
  case 0x0000:
    /* Single nametable 256 * 256 */
    rtc->ntbank[0] = 0x0000;
    rtc->ntbank[1] = 0x0000;
    rtc->ntbank[2] = 0x0000;
    rtc->ntbank[3] = 0x0000;
    break;
  case 0x4000:
    /* Double nametable 512 * 256 (hori) */
    rtc->ntbank[0] = 0x0000;
    rtc->ntbank[1] = 0x0800;
    rtc->ntbank[2] = 0x0000;
    rtc->ntbank[3] = 0x0800;
    break;
  case 0x8000:
    /* Double nametable 256 * 512 (vert) */
    rtc->ntbank[0] = 0x0000;
    rtc->ntbank[1] = 0x0000;
    rtc->ntbank[2] = 0x0800;
    rtc->ntbank[3] = 0x0800;
    break;
  case 0xC000:
    /* Quad nametable 512 * 512 (four screen) */
    rtc->ntbank[0] = 0x0000;
    rtc->ntbank[1] = 0x0800;
    rtc->ntbank[2] = 0x1000;
    rtc->ntbank[3] = 0x1800;
    break;
  default:
    DEBUG_BREAK ();
  }
  rtc->ntbank[0] += nt_base;
  rtc->ntbank[1] += nt_base;
  rtc->ntbank[2] += nt_base;
  rtc->ntbank[3] += nt_base;
}

finline
uint16_t GetAttrWord (struct rasterizer_caps *rtc, uintptr_t tile_x, uintptr_t tile_y) {
  uintptr_t hori_bank = tile_x >> 5 & 1;
  uintptr_t vert_bank = tile_y >> 5 & 1;
  uintptr_t offx_page = tile_x & 31;
  uintptr_t offy_page = tile_y & 31;
  uintptr_t addr_base =  rtc->ntbank[(vert_bank<<1)+hori_bank];
  uintptr_t addr_total = addr_base + (offy_page << 6) + (offx_page << 1);
  return * (uint16_t *) &rtc->gpu->vram[addr_total & 0xFFFF];
}

finline
void *GetBG_Chr16 (struct rasterizer_caps *rtc, uint16_t attrWord, uintptr_t offsetY) {
  uintptr_t chrAddr = (rtc->chr_base + (attrWord & 1023) *32);
  chrAddr += offsetY <<  2;
  return & rtc->gpu->vram[chrAddr&0xFFFF];
}

finline
void *GetBG_Chr256 (struct rasterizer_caps *rtc, uint16_t attrWord, uintptr_t offsetY) {
  uintptr_t chrAddr = (rtc->chr_base + (attrWord & 1023) *64);
  chrAddr += offsetY <<  3;
  return & rtc->gpu->vram[chrAddr&0xFFFF];
}
finline
void *GetSP_Chr16 (struct rasterizer_caps *rtc, uint16_t attrWord, uintptr_t offsetY) {
  uintptr_t chrAddr = ((attrWord & 1023) *32);
  chrAddr += offsetY <<  2;
  return & rtc->gpu->vram[0x10000+chrAddr];
}
finline
void *GetSP_Chr256 (struct rasterizer_caps *rtc, uint16_t attrWord, uintptr_t offsetY) {
  uintptr_t chrAddr = ((attrWord & 1023) *32);
  chrAddr += offsetY <<  3;
  return & rtc->gpu->vram[0x10000+chrAddr];
}
finline 
uint16_t alpha16 (uint16_t tilePixel, uint16_t backdrop, uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map) {

  uint32_t u_out;
  uint32_t v_out;
  uint32_t   out;
  
  if (((back_attr & bld_map) == 0x00) || ((back_attr & SOLID_CHAN_FULL_MASK) == 0))
    return tilePixel; /* TODO: BackDrop pixel ??*/

  u_out = tilePixel | tilePixel << 16;
  u_out&= 0x3E07C1F;
  u_out*= coeff1;
  v_out = backdrop | backdrop << 16;
  v_out&= 0x3E07C1F;
  v_out*= coeff2;
  /* 0000 0011 1110 0000 0111 1100 0001 1111*/
  out = v_out +u_out >> 4;
  /* Check pixel saturation */
#if 0
  if (out & 0x20)
    out |= 0x1F; /* mod:1 */
  if (out & 0x8000)
    out |= 0x7C00;/* mod:4 */
  if (out & 0x4000000)/* mod:2 */
    out |=   0x3E00000;
#else 
    out |= 0x4008020 - (out >> 5 & 0x200401);
#endif 
  out&=    0x3E07C1F;
  return out | out >> 16;
}
finline 
uint16_t brightness_inc16 (uint16_t tilePixel, uint16_t backdrop, uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map) {
  uint32_t u_out;
  uint32_t v_out;

  u_out = tilePixel | tilePixel << 16;
  u_out&= 0x3E07C1F;
  v_out = u_out ^ 0x3E07C1F;
  v_out*= coeff1;
  v_out += u_out << 4;
  v_out>>= 4;
  v_out&= 0x3E07C1F;
  return v_out | v_out >> 16;
}
finline 
uint16_t brightness_dec16 (uint16_t tilePixel, uint16_t backdrop, uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map) {
  int32_t out;
  out = tilePixel | tilePixel << 16;
  out&= 0x3E07C1F;
  out*= 16 - coeff1;
  out>>= 4; 
  out&= 0x3E07C1F;
  return out | out >> 16;
}
finline 
uint16_t pixcpy (uint16_t tilePixel, uint16_t backdrop, uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map) {
  return tilePixel;
}

finline 
void CommonBG_Render (struct rasterizer_caps *rtc,         
                            uint16_t ch_mode_mask,
                            uint16_t bld_map,
                            uint16_t coeff1, 
                               uint16_t coeff2,
                             kable (*solid) (uint16_t back_attr),
                              uint16_t (*filter) ( uint16_t tilePixel, 
                                                             uint16_t backdrop, 
                                                            uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map))
{
  const struct gpu *gpu= rtc->gpu;
  const struct gpu_channel *chan = rtc->chan;
           uint16_t *pal256 = (uint16_t *)& gpu->palette16_b[0];
           uint8_t *vram = & rtc->gpu->vram[0];

  uintptr_t blk_x;
  uintptr_t blk_y;
  intptr_t vapos;
  uint16_t *vptr;
  uint16_t *vattr;                               
  uintptr_t x_calc;
  uintptr_t y_calc;
  uintptr_t ymini_calc;                              
  uintptr_t cnt;
  uintptr_t id;
  uint16_t vcache_bank[2];
  uint16_t vchr_base;
  uint16_t vchr_base_rev;

  if (rtc->interp_y != false) {
    uintptr_t interp_mod;
    blk_x = chan->loopy_x_shadow.blk & 511;
    blk_y = chan->loopy_y_shadow.blk & 511;
    vapos = - (intptr_t)(blk_x & 7);
    vptr= & gpu->vbuf[rtc->gpu->vptr_pitch/2*gpu->line.blk + 8+vapos  ];     
    vattr = & gpu->vattr[8+vapos ];                                        
    x_calc = blk_x >> 3;
    y_calc = gpu->line.blk + blk_y & 511;
    interp_mod = y_calc % (rtc->interp_yvec + 1);
    if (interp_mod != 0)
      y_calc -= interp_mod;
    y_calc &= 511;
    ymini_calc = y_calc  & 7;     
    y_calc = y_calc >> 3; 
  } else {
    blk_x = chan->loopy_x_shadow.blk & 511;
    blk_y = chan->loopy_y_shadow.blk & 511;
    vapos = - (intptr_t)(blk_x & 7);
    vptr= & gpu->vbuf[rtc->gpu->vptr_pitch/2*gpu->line.blk + 8+vapos  ];     
    vattr = & gpu->vattr[8+vapos ];                                        
    x_calc = blk_x >> 3;
    y_calc = (gpu->line.blk + blk_y & 511) >> 3;
    ymini_calc = blk_y +  gpu->line.blk & 7;                                  
  }

  rtc->chr_base = (chan->ctl.blk >> 2 & 3) * 0x4000;
  nt_bank_settings (rtc);

  cnt = (y_calc >> 5 & 1) <<1;
  vcache_bank[0]= rtc->ntbank[cnt] + ((y_calc & 31) << 6);
  vcache_bank[1]= rtc->ntbank[cnt + 1]+ ((y_calc & 31) << 6);

  if ((rtc->chan->ctl.blk & 0x80) == 0) {
    vchr_base = rtc->chr_base + (ymini_calc << 2);
    vchr_base_rev = rtc->chr_base + (7 - ymini_calc << 2);
    for (cnt = 0; cnt != 248; cnt+= 8) {
      uint16_t attr;
      uint16_t *pal16;
      uint8_t *pix16;
      uint16_t *vpp;
      uint16_t *app;

      /* get  attr word */
      attr =* (uint16_t *) & vram [vcache_bank[x_calc >> 5 & 1] + ((x_calc & 31) << 1)& 0xFFFF];
      /* get palette bank */
      pal16 = & pal256[attr >> 12 << 4];
      /* get chr pixel ptr */
      if (attr & 0x800) 
        pix16 = & vram[vchr_base_rev + ((attr & 1023) << 5) & 0xFFFF];
      else 
        pix16 = & vram[vchr_base + ((attr & 1023) << 5)& 0xFFFF];
      vpp = & vptr[cnt];
      app = & vattr[cnt];
      x_calc++;

      if (attr & 0x400) {
#if 0
        for (id = 3; id!= (uintptr_t)-1; id--) {
          uintptr_t hi = pix16[id] & 15;
          uintptr_t lo = pix16[id] >> 4;
          uintptr_t pos = 3-id  << 1;
          
          if (solid (app[pos])) { 
            if (lo) {
              vpp[pos] = filter (pal16[lo], vpp[pos], coeff1, coeff2, app[pos], bld_map);  
              app[pos] = ch_mode_mask;
            }
          } 
          if (solid (app[pos+1])) { 
            if (hi) {
              vpp[pos+1] = filter (pal16[hi], vpp[pos+1], coeff1, coeff2, app[pos+1], bld_map); 
              app[pos+1] = ch_mode_mask;
            }
          } 
        }
#else 
#undef T8SU_GU_
#define T8SU_GU_(x, n)\
  { \
  uint8_t chr_mixer = pix16[x];\
  uint8_t lo = chr_mixer >> 4;\
  uint8_t hi = chr_mixer  &15;\
  \
  if (solid (app[n])) { \
    if (lo) {\
      vpp[n] = filter (pal16[lo], vpp[n], coeff1, coeff2, app[n], bld_map);  \
      app[n] = ch_mode_mask;\
    }\
  } \
  if (solid (app[n+1])) { \
    if (hi) {\
      vpp[n+1] = filter (pal16[hi], vpp[n+1], coeff1, coeff2, app[n+1], bld_map); \
      app[n+1] = ch_mode_mask;\
    }\
  } \
}
        T8SU_GU_(3, 0)
        T8SU_GU_(2, 2)
        T8SU_GU_(1, 4)
        T8SU_GU_(0, 6)

#endif 
      } else {
#if   0
        for (id = 0; id!= 4; id++) {
          uintptr_t lo = pix16[id] & 15;
          uintptr_t hi = pix16[id] >> 4;
          uintptr_t pos = id << 1;

          if (solid (app[pos])) { 
            if (lo) {
              vpp[pos] = filter (pal16[lo], vpp[pos], coeff1, coeff2, app[pos], bld_map);  
              app[pos] = ch_mode_mask;
            }
          } 
          if (solid (app[pos+1])) { 
            if (hi) {
              vpp[pos+1] = filter (pal16[hi], vpp[pos+1], coeff1, coeff2, app[pos+1], bld_map); 
              app[pos+1] = ch_mode_mask;
            }
          } 
        }

#else 
#undef T8SU_GU_
#define T8SU_GU_(x, n)\
  { \
  uint8_t chr_mixer = pix16[x];\
  uint8_t lo = chr_mixer & 15;\
  uint8_t hi = chr_mixer >> 4;\
  \
  if (solid (app[n])) { \
    if (lo) {\
      vpp[n] = filter (pal16[lo], vpp[n], coeff1, coeff2, app[n], bld_map);  \
      app[n] = ch_mode_mask;\
    }\
  } \
  if (solid (app[n+1])) { \
    if (hi) {\
      vpp[n+1] = filter (pal16[hi], vpp[n+1], coeff1, coeff2, app[n+1], bld_map); \
      app[n+1] = ch_mode_mask;\
    }\
  } \
}
        T8SU_GU_(0, 0)
        T8SU_GU_(1, 2)
        T8SU_GU_(2, 4)
        T8SU_GU_(3, 6)
#endif 
      }
    }
  } else {
    for (cnt = 0; cnt != 248; cnt+= 8) {
      uint16_t attr = GetAttrWord (rtc, x_calc++, y_calc);
      uint8_t *pix256 = (uint8_t *) GetBG_Chr256 (rtc, attr, (attr & 0x800) ? (7 - ymini_calc) : ymini_calc);
      uint16_t *vpp = & vptr[cnt];
      uint16_t *app = & vattr[cnt];

      if (attr & 0x400) {
        for (id = 7; id!= (uintptr_t)-1; id--) {
          uintptr_t pmix = pix256[id];
          uintptr_t pos = 7-id;
          if (solid (app[pos])) { 
            if (pmix) {
              vpp[pos] = filter (pal256[pmix], vpp[pos], coeff1, coeff2, app[pos], bld_map);  
              app[pos] = ch_mode_mask;
            }
          } 
        }
      } else {
        for (id = 0; id!= 8; id++) {
          uintptr_t pmix = pix256[id];
        
          if (solid (app[id])) { 
            if (pmix) {
              vpp[id] = filter (pal256[pmix], vpp[id], coeff1, coeff2, app[id], bld_map);  
              app[id] = ch_mode_mask;
            }
          } 
        }
      }
    }
  }
  /* FIXME: compared with standard GBA game consoles, mosaic edge processing has errors. */
  if (rtc->interp_x != false) {
    intptr_t mosaic_blk  = rtc->interp_xvec + 1;
    vptr= & gpu->vbuf[rtc->gpu->vptr_pitch/2*gpu->line.blk + 8  ];     
    vattr = & gpu->vattr[8 ];   

    for (cnt = 0; cnt != 240; cnt ++){
      int mod = cnt % mosaic_blk;
      if (mod != 0) {
        int pos = cnt - mod;
        if (vattr[pos] == ch_mode_mask)
          vptr[cnt] = vptr[pos];
      }
    }
  }
}

finline 
void CommonBG_Render_Rot (struct rasterizer_caps *rtc, 
                            uint16_t ch_mode_mask,
                            uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2,
                              kable (*solid) (uint16_t back_attr),
                              uint16_t (*filter) ( uint16_t tilePixel, 
                                                             uint16_t backdrop, 
                                                            uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map))
{
  struct gpu *gpu= rtc->gpu;
  struct gpu_channel *chan = rtc->chan;
  uintptr_t tile_x = chan->loopy_x_shadow.blk & 511;
  uintptr_t tile_y = chan->loopy_y_shadow.blk & 511;                             
  uint16_t *vattr_b = & gpu->vattr[ 8 ];                                          
  uint16_t *vptr_b= & gpu->vbuf[gpu->vptr_pitch/2*gpu->line.blk + 8 ];                   
  uintptr_t cnt;
  uintptr_t affi_bx;                              
  uintptr_t affi_by;                              
  uintptr_t affi_dx;                              
  uintptr_t affi_dy;                              
  uintptr_t affi_xp;                              
  uintptr_t affi_yp;          
  uintptr_t round_mask;                           
  uintptr_t affi_mask;                            
  uintptr_t affi_mask2;                                                                            
  uint8_t *chr_b = & gpu->vram[(chan->ctl.blk>> 2 & 3) * 0x4000];  
  uint8_t *te_b= & gpu->vram[0];                                
  uintptr_t te_ac = (chan->ctl .blk>> 8 & 31) * 0x800;              
  uint16_t *bg_pal = (uint16_t *)& gpu->palette16_b[0];                                                                                                                                                               
  uintptr_t te_sft;                                                         
  uintptr_t te_pi;                                                          
  uintptr_t te_blk_x;                 
  uintptr_t te_blk_y;               
  uintptr_t te_mini_x;              
  uintptr_t te_mini_y;            
  uintptr_t te_addr; 
  uintptr_t te_id;                                 
          
  rtc->chr_base = (rtc->chan->ctl.blk>> 2 & 3) * 0x4000;

  switch (rtc->chan->ctl.blk & 0xC000) {                                             
  case 0x0000:                                                              
    affi_mask = 128 << 8;                         
    round_mask = 128;                             
    te_sft = 4;                                   
    break;                                        
  case 0x4000:                                    
    affi_mask = 256 << 8;                         
    round_mask = 256;                             
    te_sft = 5;                                   
    break;                                        
  case 0x8000:                                    
    affi_mask = 512 << 8;                         
    round_mask = 512;                             
    te_sft = 6;                                   
    break;                                        
  case 0xC000:              
    affi_mask = 1024 << 8;  
    round_mask = 1024;      
    te_sft = 7;             
    break;                  
  }                         
  round_mask--;             
  affi_mask--;              
  affi_mask = ~affi_mask;   
  affi_mask2= affi_mask >> 8; 
                            
  affi_bx = rtc->chan->loopy_dmx.blk;
  affi_by = rtc->chan->loopy_dmy.blk;
  affi_dx = rtc->chan->dx_shadow.blk;       
  affi_dy = rtc->chan->dy_shadow.blk;       

  if (rtc->chan->ctl.blk & 0x2000) {
    /* 8Bit Wrapround */
    for (cnt = 0; cnt != 240; cnt++) {
                                                    
      affi_xp = (uintptr_t) affi_bx >> 8 & round_mask;            
      affi_yp = (uintptr_t) affi_by >> 8 & round_mask;       
      
      te_blk_x = affi_xp >> 3;                  
      te_blk_y = affi_yp >> 3;                  
      te_mini_x = affi_xp & 7;                  
      te_mini_y = affi_yp & 7;                  
      te_addr = te_ac + te_blk_x+ (te_blk_y << te_sft) & 0xFFFF;  
      te_id = te_b[te_addr];                                      
      te_pi = chr_b[(te_id<<6)+te_mini_x+(te_mini_y<<3)];   
                                                          
      if (solid (vattr_b[cnt]))  {                 
        if (te_pi) {
          vptr_b[cnt] = filter (bg_pal[te_pi], vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask;
        }
      }               
      affi_bx += affi_dx;
      affi_by += affi_dy;
    }
  } else {
    for (cnt = 0; cnt != 240; cnt++) {                 
     if (((affi_bx | affi_by) & affi_mask) == 0) {
    // if (1) {
        affi_xp = (uintptr_t) affi_bx >> 8 & round_mask;            
        affi_yp = (uintptr_t) affi_by >> 8 & round_mask;       
        
        te_blk_x = affi_xp >> 3;                  
        te_blk_y = affi_yp >> 3;                  
        te_mini_x = affi_xp & 7;                  
        te_mini_y = affi_yp & 7;                  
        te_addr = te_ac + te_blk_x+ (te_blk_y << te_sft) & 0xFFFF;  
        te_id = te_b[te_addr];                                      
        te_pi = chr_b[(te_id<<6)+te_mini_x+(te_mini_y<<3)];   
                                                            
        if (solid (vattr_b[cnt]))  {                 
          if (te_pi) {
            vptr_b[cnt] = filter (bg_pal[te_pi], vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);                                       
            vattr_b[cnt] = ch_mode_mask;
          }
        }   
      }
      affi_bx += affi_dx;
      affi_by += affi_dy;
    }
  }
}

finline 
void Mode3Bitmap8Full_BG_Render_Rot (struct rasterizer_caps *rtc, 
                            uint16_t ch_mode_mask,
                            uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2,
                              kable (*solid) (uint16_t back_attr),
                              uint16_t (*filter) ( uint16_t tilePixel, 
                                                             uint16_t backdrop, 
                                                            uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map))
{
  struct gpu *gpu= rtc->gpu;
  struct gpu_channel *chan = rtc->chan;                          
  uint16_t *vattr_b = & gpu->vattr[ 8 ];                                          
  uint16_t *vptr_b= & gpu->vbuf[gpu->vptr_pitch/2*gpu->line.blk + 8 ];                   
  uintptr_t cnt;
  intptr_t affi_bx;                              
  intptr_t affi_by;                              
  intptr_t affi_dx;                              
  intptr_t affi_dy;                              
  intptr_t affi_xp;                              
  intptr_t affi_yp;                              
  uint8_t *vram= & gpu->vram[0];                            
                    
  affi_bx = rtc->chan->loopy_dmx.sblk;
  affi_by = rtc->chan->loopy_dmy.sblk;
  affi_dx = rtc->chan->dx_shadow.sblk;       
  affi_dy = rtc->chan->dy_shadow.sblk;       

  if (gpu->palette16_b == & gpu->palette2[0]) {
    if (rtc->chan->ctl.blk & 0x2000) {
      /* 8Bit Wrapround */
      for (cnt = 0; cnt != 240; cnt++) {                                          
        if (solid (vattr_b[cnt]))  {    
          uint16_t pixel;
          affi_xp = affi_bx >> 8 % 240;
          affi_yp = affi_by >> 8 % 160;  
          if (affi_xp < 0)
            affi_xp = affi_xp + 240;
          if (affi_yp < 0)
             affi_yp = affi_yp + 160;
          pixel = * (uint16_t *)& vram[affi_yp*480+(affi_xp << 1)];
          pixel = pixel >> 10 & 31 
        |          pixel & 31 << 5 
        |          (pixel & 31) << 10;
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask;
        }               
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    } else {
      for (cnt = 0; cnt != 240; cnt++) {         
        intptr_t pos_x = affi_bx >> 8;
        intptr_t pos_y = affi_by >> 8;

        if (pos_x >= 0 
          && pos_x < 240
          && pos_y >= 0 
          && pos_y < 160)
        {
          uint16_t pixel= * (uint16_t *)& vram[pos_y*480+(pos_x << 1)];
          pixel = pixel >> 10 & 31 
        |          pixel & 31 << 5 
        |          (pixel & 31) << 10;
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask; 
        }
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    }
  } else {

    if (rtc->chan->ctl.blk & 0x2000) {
      /* 8Bit Wrapround */
      for (cnt = 0; cnt != 240; cnt++) {                                          
        if (solid (vattr_b[cnt]))  {    
          uint16_t pixel;
          affi_xp = affi_bx >> 8 % 240;
          affi_yp = affi_by >> 8 % 160;  
          if (affi_xp < 0)
            affi_xp = affi_xp + 240;
          if (affi_yp < 0)
             affi_yp = affi_yp + 160;
          pixel = * (uint16_t *)& vram[affi_yp*480+(affi_xp << 1)];
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask;
        }               
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    } else {
      for (cnt = 0; cnt != 240; cnt++) {         
        intptr_t pos_x = affi_bx >> 8;
        intptr_t pos_y = affi_by >> 8;

        if (pos_x >= 0 
          && pos_x < 240
          && pos_y >= 0 
          && pos_y < 160)
        {
          uint16_t pixel= * (uint16_t *)& vram[pos_y*480+(pos_x << 1)];
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask; 
        }
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    }
  }
}

finline 
void Mode4Bitmap4SwapBuffer_BG_Render_Rot (struct rasterizer_caps *rtc, 
                            uint16_t ch_mode_mask,
                            uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2,
                              kable (*solid) (uint16_t back_attr),
                              uint16_t (*filter) ( uint16_t tilePixel, 
                                                             uint16_t backdrop, 
                                                            uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map))
{
  struct gpu *gpu= rtc->gpu;
  struct gpu_channel *chan = rtc->chan;                          
  uint16_t *vattr_b = & gpu->vattr[ 8 ];                                          
  uint16_t *vptr_b= & gpu->vbuf[gpu->vptr_pitch/2*gpu->line.blk + 8 ];                   
  uintptr_t cnt;
  intptr_t affi_bx;                              
  intptr_t affi_by;                              
  intptr_t affi_dx;                              
  intptr_t affi_dy;                              
  intptr_t affi_xp;                              
  intptr_t affi_yp;                                                      
  uint16_t *pal256 = (uint16_t *)& gpu->palette16_b[0];
  uint8_t *frame_bank = & gpu->vram[(gpu->ctl.blk >> 4 & 1) ? 0xA000 : 0];   

  affi_bx = rtc->chan->loopy_dmx.sblk;
  affi_by = rtc->chan->loopy_dmy.sblk;
  affi_dx = rtc->chan->dx_shadow.sblk;       
  affi_dy = rtc->chan->dy_shadow.sblk;       

  if (rtc->chan->ctl.blk & 0x2000) {
    /* 8Bit Wrapround */
    for (cnt = 0; cnt != 240; cnt++) {                                          
      if (solid (vattr_b[cnt]))  {    
        uint8_t pal;
        affi_xp = affi_bx >> 8 % 240;
        affi_yp = affi_by >> 8 % 160;  
        if (affi_xp < 0)
          affi_xp = affi_xp + 240;
        if (affi_yp < 0)
            affi_yp = affi_yp + 160;
        pal = frame_bank[affi_yp*240+affi_xp];
        if (pal ) {
          vptr_b[cnt] = filter (pal256[pal], vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask;
        }
      }               
      affi_bx += affi_dx;
      affi_by += affi_dy;
    }
  } else {
    for (cnt = 0; cnt != 240; cnt++) {         
      intptr_t pos_x = affi_bx >> 8;
      intptr_t pos_y = affi_by >> 8;

      if (pos_x >= 0 
        && pos_x < 240
        && pos_y >= 0 
        && pos_y < 160)
      {
        uint8_t pal = frame_bank[pos_y*240+pos_x];
        if (pal ) {
          vptr_b[cnt] = filter (pal256[pal], vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask;
        }
      }
      affi_bx += affi_dx;
      affi_by += affi_dy;
    }
  }
}

finline 
void Mode5Bitmap8SwapBuffer_BG_Render_Rot (struct rasterizer_caps *rtc, 
                            uint16_t ch_mode_mask,
                            uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2,
                              kable (*solid) (uint16_t back_attr),
                              uint16_t (*filter) ( uint16_t tilePixel, 
                                                             uint16_t backdrop, 
                                                            uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map))
{
  struct gpu *gpu= rtc->gpu;
  struct gpu_channel *chan = rtc->chan;                          
  uint16_t *vattr_b = & gpu->vattr[ 8 ];                                          
  uint16_t *vptr_b= & gpu->vbuf[gpu->vptr_pitch/2*gpu->line.blk + 8 ];                   
  uintptr_t cnt;
  intptr_t affi_bx;                              
  intptr_t affi_by;                              
  intptr_t affi_dx;                              
  intptr_t affi_dy;                              
  intptr_t affi_xp;                              
  intptr_t affi_yp;                              
  uint8_t *vram= & gpu->vram[(gpu->ctl.blk >> 4 & 1) ? 0xA000 : 0];               
                    
  affi_bx = rtc->chan->loopy_dmx.sblk;
  affi_by = rtc->chan->loopy_dmy.sblk;
  affi_dx = rtc->chan->dx_shadow.sblk;       
  affi_dy = rtc->chan->dy_shadow.sblk;       

  if (gpu->palette16_b == & gpu->palette2[0]) {
    if (rtc->chan->ctl.blk & 0x2000) {
      /* 8Bit Wrapround */
      for (cnt = 0; cnt != 240; cnt++) {                                          
        if (solid (vattr_b[cnt]))  {    
          uint16_t pixel;
          affi_xp = affi_bx >> 8 % 160;
          affi_yp = affi_by >> 8 % 128;  
          if (affi_xp < 0)
            affi_xp = affi_xp + 160;
          if (affi_yp < 0)
             affi_yp = affi_yp + 128;
          pixel = * (uint16_t *)& vram[affi_yp*320+(affi_xp << 1)];
          pixel = pixel >> 10 & 31 
        |          pixel & 31 << 5 
        |          (pixel & 31) << 10;
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask;
        }               
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    } else {
      for (cnt = 0; cnt != 240; cnt++) {         
        intptr_t pos_x = affi_bx >> 8;
        intptr_t pos_y = affi_by >> 8;

        if (pos_x >= 0 
          && pos_x < 160
          && pos_y >= 0 
          && pos_y < 128)
        {
          uint16_t pixel= * (uint16_t *)& vram[pos_y*320+(pos_x << 1)];
          pixel = pixel >> 10 & 31 
        |          pixel & 31 << 5 
        |          (pixel & 31) << 10;
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask; 
        }
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    }
  } else {

    if (rtc->chan->ctl.blk & 0x2000) {
      /* 8Bit Wrapround */
      for (cnt = 0; cnt != 240; cnt++) {                                          
        if (solid (vattr_b[cnt]))  {    
          uint16_t pixel;
          affi_xp = affi_bx >> 8 % 160;
          affi_yp = affi_by >> 8 % 128;  
          if (affi_xp < 0)
            affi_xp = affi_xp + 160;
          if (affi_yp < 0)
             affi_yp = affi_yp + 128;
          pixel = * (uint16_t *)& vram[affi_yp*320+(affi_xp << 1)];
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask;
        }               
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    } else {
      for (cnt = 0; cnt != 240; cnt++) {         
        intptr_t pos_x = affi_bx >> 8;
        intptr_t pos_y = affi_by >> 8;

        if (pos_x >= 0 
          && pos_x < 160
          && pos_y >= 0 
          && pos_y < 128)
        {
          uint16_t pixel= * (uint16_t *)& vram[pos_y*320+(pos_x << 1)];
          vptr_b[cnt] = filter (pixel, vptr_b[cnt], coeff1, coeff2, vattr_b[cnt], bld_map);           
          vattr_b[cnt] = ch_mode_mask; 
        }
        affi_bx += affi_dx;
        affi_by += affi_dy;
      }
    }
  }
}

finline 
void CommonSP_Draw (
           struct rasterizer_caps *rtc,
             uint16_t ch_mode_mask,
             uint16_t bld_map,
     uint16_t *vptr_b,
     uint16_t *vattr_b,
     uint16_t *oam_entry,
     uint16_t *pal16,
     uint16_t *pal256,
     uint16_t coeff1, 
     uint16_t coeff2,
     uintptr_t tbid_t, 
     uintptr_t line,
     uintptr_t dim_x,
     uintptr_t dim_y,
     uintptr_t pos_x,
     uintptr_t pos_y,
     kable (*solid) (uint16_t back_attr),
    uint16_t (*filter) ( uint16_t tilePixel, 
                                   uint16_t backdrop, 
                                  uint16_t coeff1, uint16_t coeff2, uint16_t back_attr, uint16_t bld_map))
{
  intptr_t y_offset;
  intptr_t y_blockoffset;
  intptr_t y_miniffset;
  uintptr_t line2 = line;
  
  if ((pos_y + dim_y) >= 256)
    line2 += 256;
  if (line2 < pos_y) /* TODO : oam bug */
    return ;
  if (line2 >= (pos_y+dim_y))
    return ;
  if (pos_x >= 240 && (pos_x + dim_x) <= 512)
    return ;
  y_offset = line2 - pos_y;
  if (oam_entry[1] & 0x2000) 
    y_offset = dim_y - 1 - y_offset;
  else ;
  y_blockoffset = y_offset >> 3;
  y_miniffset = y_offset & 7;

  if (oam_entry[0] & 0x2000) {
    if (oam_entry[1] & 0x1000) {
      /* std horz-swap sprite */ 
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + ((tbid_t /* & 1022 */)) * 32 + (y_miniffset << 3)];
      uintptr_t x = pos_x + dim_x - 1;
      do 
      {
#undef OAMS_
#define OAMS_(n)\
      x &= 511;\
      if (solid (vattr_b[x])) {\
        uint8_t chr_value = chrb[n];\
        if (chr_value) {\
          vptr_b[x] =  filter (pal256[chr_value], vptr_b[x], coeff1, coeff2, vattr_b[x], bld_map);\
          vattr_b[x] = ch_mode_mask;\
        }\
      }\
      x--;

#undef OAMZ_
#define OAMZ_(n)\
      OAMS_(64*n+0)\
      OAMS_(64*n+1)\
      OAMS_(64*n+2)\
      OAMS_(64*n+3)\
      OAMS_(64*n+4)\
      OAMS_(64*n+5)\
      OAMS_(64*n+6)\
      OAMS_(64*n+7)\
      if (dim_x == 8* (n+1))\
        break;

#undef OAMZ__
#define OAMZ__(n)\
      OAMS_(64*n+0)\
      OAMS_(64*n+1)\
      OAMS_(64*n+2)\
      OAMS_(64*n+3)\
      OAMS_(64*n+4)\
      OAMS_(64*n+5)\
      OAMS_(64*n+6)\
      OAMS_(64*n+7)

        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    } else {
      /* std noswap sprite */
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + ((tbid_t /* & 1022 */)) * 32 + (y_miniffset << 3)];
      uintptr_t x = pos_x;

      do 
      {
#undef OAMS_
#define OAMS_(n)\
      x &= 511;\
      if (solid (vattr_b[x])) {\
        uint8_t chr_value = chrb[n];\
        if (chr_value) {\
          vptr_b[x] =  filter (pal256[chr_value], vptr_b[x], coeff1, coeff2, vattr_b[x], bld_map);\
          vattr_b[x] = ch_mode_mask;\
        }\
      }\
      x++;
        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    }
  } else {
    if (oam_entry[1] & 0x1000) {
      /* std horz-swap sprite */ 
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + (tbid_t & 1023) * 32 + (y_miniffset << 2)];
      uintptr_t x = pos_x + dim_x - 1;
      uint8_t chr_mixer;

      do 
      {
#undef OAMS_
#define OAMS_(n)\
      x &= 511;\
      chr_mixer = chrb[n];\
      if (solid (vattr_b[x])) {\
        uint8_t chr_lo = chr_mixer & 15; \
        if (chr_lo) {\
          vptr_b[x] =  filter (pal16[chr_lo], vptr_b[x], coeff1, coeff2, vattr_b[x], bld_map);\
          vattr_b[x] = ch_mode_mask;\
        }\
      }\
      x--; \
      x &= 511;\
      if (solid (vattr_b[x])) {\
        uint8_t chr_hi = chr_mixer >> 4; \
        if (chr_hi) {\
          vptr_b[x] =  filter (pal16[chr_hi], vptr_b[x], coeff1, coeff2, vattr_b[x], bld_map);\
          vattr_b[x] = ch_mode_mask;\
        }\
      }\
      x--;

#undef OAMZ_
#define OAMZ_(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)\
      if (dim_x == 8* (n+1))\
        break;

#undef OAMZ__
#define OAMZ__(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)

        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    } else {
      /* std noswap sprite */
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + (tbid_t & 1023) * 32 + (y_miniffset << 2)];
      uintptr_t x = pos_x;
      uint8_t chr_mixer;

      do 
      {
#undef OAMS_
#define OAMS_(n)\
      x &= 511;\
      chr_mixer = chrb[n];\
      if (solid (vattr_b[x])) {\
        uint8_t chr_lo = chr_mixer & 15; \
        if (chr_lo) {\
          vptr_b[x] =  filter (pal16[chr_lo], vptr_b[x], coeff1, coeff2, vattr_b[x], bld_map);\
          vattr_b[x] = ch_mode_mask;\
        }\
      }\
      x++; \
      x &= 511;\
      if (solid (vattr_b[x])) {\
        uint8_t chr_hi = chr_mixer >> 4; \
        if (chr_hi) {\
          vptr_b[x] =  filter (pal16[chr_hi], vptr_b[x], coeff1, coeff2, vattr_b[x], bld_map);\
          vattr_b[x] = ch_mode_mask;\
        }\
      }\
      x++;

#undef OAMZ_
#define OAMZ_(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)\
      if (dim_x == 8* (n+1))\
        break;

#undef OAMZ__
#define OAMZ__(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)

        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    }
  }
}

finline 
void CommonSP_Draw_Shadow (
           struct rasterizer_caps *rtc,
     uint16_t *vattr_b,
     uint16_t *oam_entry,
     uintptr_t tbid_t, 
     uintptr_t line,
     uintptr_t dim_x,
     uintptr_t dim_y,
     uintptr_t pos_x,
     uintptr_t pos_y)
{
  int y_offset;
  int y_blockoffset;
  int y_miniffset;
  unsigned int line2 = line;
  
  if ((pos_y + dim_y) >= 256)
    line2 += 256;
  if (line2 < pos_y) /* TODO : oam bug */
    return ;
  if (line2 >= (pos_y+dim_y))
    return ;
  if (pos_x >= 240 && (pos_x + dim_x) <= 512)
    return ;
  y_offset = line2 - pos_y;
  if (oam_entry[1] & 0x2000) 
    y_offset = dim_y - 1 - y_offset;
  else ;
  y_blockoffset = y_offset >> 3;
  y_miniffset = y_offset & 7;

  if (oam_entry[0] & 0x2000) {
    if (oam_entry[1] & 0x1000) {
      /* std horz-swap sprite */ 
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + ((tbid_t /* & 1022 */)) * 32 + (y_miniffset << 3)];
      uintptr_t x = pos_x + dim_x - 1;
      do 
      {
#undef OAMS_
#define OAMS_(n)\
      if (chrb[n]) \
        vattr_b[x & 511] |= SOLID_MODE_OBJWIN_MASK;\
      x--;

#undef OAMZ_
#define OAMZ_(n)\
      OAMS_(64*n+0)\
      OAMS_(64*n+1)\
      OAMS_(64*n+2)\
      OAMS_(64*n+3)\
      OAMS_(64*n+4)\
      OAMS_(64*n+5)\
      OAMS_(64*n+6)\
      OAMS_(64*n+7)\
      if (dim_x == 8* (n+1))\
        break;

#undef OAMZ__
#define OAMZ__(n)\
      OAMS_(64*n+0)\
      OAMS_(64*n+1)\
      OAMS_(64*n+2)\
      OAMS_(64*n+3)\
      OAMS_(64*n+4)\
      OAMS_(64*n+5)\
      OAMS_(64*n+6)\
      OAMS_(64*n+7)

        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    } else {
      /* std noswap sprite */
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + ((tbid_t /* & 1022 */)) * 32 + (y_miniffset << 3)];
      uintptr_t x = pos_x;

      do 
      {
#undef OAMS_
#define OAMS_(n)\
      if (chrb[n]) \
        vattr_b[x & 511] |= SOLID_MODE_OBJWIN_MASK;\
      x++;
        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    }
  } else {
    if (oam_entry[1] & 0x1000) {
      /* std horz-swap sprite */ 
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + (tbid_t & 1023) * 32 + (y_miniffset << 2)];
      uintptr_t x = pos_x + dim_x - 1;
      uint8_t chr_mixer;

      do 
      {
#undef OAMS_
#define OAMS_(n)\
      chr_mixer = chrb[n];\
      if (chr_mixer & 0x0F) \
        vattr_b[x & 511] |= SOLID_MODE_OBJWIN_MASK;\
      x--; \
      if (chr_mixer & 0xF0) \
        vattr_b[x & 511] |= SOLID_MODE_OBJWIN_MASK;\
      x--;

#undef OAMZ_
#define OAMZ_(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)\
      if (dim_x == 8* (n+1))\
        break;

#undef OAMZ__
#define OAMZ__(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)

        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    } else {
      /* std noswap sprite */
      const uint8_t *chrb = & rtc->gpu->vram [0x10000 + (tbid_t & 1023) * 32 + (y_miniffset << 2)];
      uintptr_t x = pos_x;
      uint8_t chr_mixer;

      do 
      {
#undef OAMS_
#define OAMS_(n)\
      chr_mixer = chrb[n];\
      if (chr_mixer & 0x0F) \
        vattr_b[x & 511] |= SOLID_MODE_OBJWIN_MASK;\
      x++; \
      if (chr_mixer & 0xF0) \
        vattr_b[x & 511] |= SOLID_MODE_OBJWIN_MASK;\
      x++;

#undef OAMZ_
#define OAMZ_(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)\
      if (dim_x == 8* (n+1))\
        break;

#undef OAMZ__
#define OAMZ__(n)\
      OAMS_(32*n+0)\
      OAMS_(32*n+1)\
      OAMS_(32*n+2)\
      OAMS_(32*n+3)

        OAMZ_(0)
        OAMZ_(1)
        OAMZ_(2)
        OAMZ_(3)
        OAMZ_(4)
        OAMZ_(5)
        OAMZ_(6)
        OAMZ__(7)

      } while (0);
    }
  }
}

finline 
void AffineMartixSP_Draw (
           struct rasterizer_caps *rtc,
             uint16_t ch_mode_mask,
             uint16_t bld_map,
             uint16_t *oamb, 
             uint16_t slot,
             uint16_t tile_pitch,
     uint16_t *vptr_b,
     uint16_t *vattr_b,
     uint16_t *pal16,
     uint16_t *pal256,
     uint16_t coeff1, 
     uint16_t coeff2,
     uintptr_t line,
     uintptr_t dim_x,
     uintptr_t dim_y,
     uintptr_t pos_x,
     uintptr_t pos_y,
     kable (*solid) (uint16_t back_attr),
        uint16_t (*filter) ( uint16_t tilePixel, 
                                        uint16_t backdrop, 
                                      uint16_t coeff1, uint16_t coeff2, uint16_t back_att, uint16_t bld_map))
{
  uint16_t *oam;
  uintptr_t virtual_pos_x;
  uintptr_t virtual_pos_y;
  uintptr_t virtual_dim_x;
  uintptr_t virtual_dim_y;
  uintptr_t tbid;
  intptr_t calc_left;
  intptr_t calc_right;
  intptr_t calc_top;
  intptr_t calc_bottom;
  intptr_t check_left;
  intptr_t check_right;
  intptr_t check_top;
  intptr_t check_bottom;
  intptr_t affine_bx;
  intptr_t affine_by;
  intptr_t affine_dx;
  intptr_t affine_dy;
  intptr_t affine_dmx;
  intptr_t affine_dmy;
  intptr_t cnt;
  int16_t *affbank;

  oam = & oamb[slot <<2];

  /* Get affine params */
  affbank = (int16_t *)& oamb[(oam[1] >> 9 & 31) << 4];
  affine_dx =affbank[3] ;
  affine_dmx =affbank[3+4] ;
  affine_dy =affbank[3+8] ;
  affine_dmy =affbank[3+12] ;

  if (line >= 160 && line <= 255)
    return ;

  if (oam[0] & 0x200) {
    /* TODO: Scanline 128-160 BUG. */
    virtual_pos_x = pos_x;
    virtual_pos_y = pos_y;
    virtual_dim_x = dim_x << 1;
    virtual_dim_y = dim_y << 1;
    check_left = pos_x + (dim_x >> 1);
    check_top = dim_y >> 1;
  } else {
    virtual_pos_x = pos_x;
    virtual_pos_y = pos_y;
    virtual_dim_x = dim_x;
    virtual_dim_y = dim_y;
    check_left = pos_x;
    check_top = 0;
  }
  virtual_pos_x &= 511;
  virtual_pos_y &= 255;

  if (virtual_pos_x >= 240 && (virtual_pos_x + virtual_dim_x) <= 512)
    return ;
  if (virtual_pos_y >= 160 && (virtual_pos_y + virtual_dim_y) <= 256)
    return ;

  tbid = oam[2] & 1023;
  calc_left = virtual_pos_x;
  calc_right = virtual_pos_x + virtual_dim_x;
  
  if ((virtual_pos_y + virtual_dim_y) > 256) {
    /* Cross screen bottom to up.*/
    uintptr_t phase = virtual_pos_y + virtual_dim_y - 1 & 255;
    if (!(line >= 0 && line <= phase))
      return ;
    /* Make affine guard negtive */
    calc_top = virtual_pos_y - 256;
    check_top = check_top + pos_y - 256;
    /* Get Y Offset in tile 
      x init vector always -dim_x/2 *affine param.
    */
  } else {
    if (line < virtual_pos_y) /* TODO : oam bug */
      return ;
    if (line > (virtual_pos_y + virtual_dim_y -1)) /* TODO : oam bug */
      return ;

    check_top += pos_y;
    calc_top = virtual_pos_y;
  }
  check_bottom = check_top+ dim_y;
  check_right = check_left + dim_x;
  calc_bottom = calc_top + virtual_dim_y;

  /* Calc init affine params */
  if (1) {
    /* Pos - Middle Point */
    intptr_t mid_x = (calc_left + calc_right) / 2;
    intptr_t mid_y = (calc_top + calc_bottom) / 2;
    intptr_t affoff_x = -(virtual_dim_x/2);
    intptr_t affoff_y = (intptr_t) line - mid_y;

    affine_bx = affine_dx*affoff_x+affine_dmx*affoff_y+mid_x*256;
    affine_by  = affine_dy*affoff_x+affine_dmy*affoff_y+mid_y*256;
  }

  if (oam[0] & 0x2000) {
    /* 256 PAL */
    for (cnt = virtual_pos_x; cnt != virtual_pos_x+virtual_dim_x; cnt++) {
      /* Check Range */
      intptr_t xpos = affine_bx >> 8;
      intptr_t ypos = affine_by >> 8;

      if (xpos >= check_left 
        && xpos < check_right 
         && ypos >= check_top
        && ypos < check_bottom)
      {
        intptr_t tile_x = xpos - check_left;
        intptr_t tile_y = ypos - check_top;
        intptr_t tile_mod8_x = tile_x & 7;
        intptr_t tile_mod8_y = tile_y & 7;
        intptr_t tid;
        uint8_t *chr;
        uint8_t memb;
        const uintptr_t ii = cnt&511;

        tile_x >>= 3;
        tile_y >>= 3;

        tid = tbid + tile_y * tile_pitch + tile_x *2;
        /* fetch tile mem */
        chr = (uint8_t *)GetSP_Chr256 (rtc, tid, tile_mod8_y);
        memb = chr[tile_mod8_x];

        if (solid (vattr_b[ii])) {
          if (memb) {
            vptr_b[ii] =  filter (pal256[memb], vptr_b[ii], coeff1, coeff2, vattr_b[ii], bld_map);
            vattr_b[ii] = ch_mode_mask;
          }
        }
      }
      affine_bx += affine_dx;
      affine_by += affine_dy;
    }
  } else {
    /* 16 PAL */
    for (cnt = virtual_pos_x; cnt != virtual_pos_x+virtual_dim_x; cnt++) {
      /* Check Range */
      intptr_t xpos = affine_bx >> 8;
      intptr_t ypos = affine_by >> 8;

      if (xpos >= check_left 
        && xpos < check_right 
         && ypos >= check_top
        && ypos < check_bottom)
      {
        intptr_t tile_x = xpos - check_left;
        intptr_t tile_y = ypos - check_top;
        intptr_t tile_mod8_x = tile_x & 7;
        intptr_t tile_mod8_y = tile_y & 7;
        intptr_t tid;
        uint8_t *chr;
        uint8_t memb;
        const uintptr_t ii = cnt&511;

        tile_x >>= 3;
        tile_y >>= 3;

        tid = tbid + tile_y * tile_pitch + tile_x;
        /* fetch tile mem */
        chr = (uint8_t *)GetSP_Chr16 (rtc, tid, tile_mod8_y);
        memb = chr[tile_mod8_x>>1];
        if (tile_mod8_x & 1)
          memb >>= 4;
        else 
          memb &= 15;

        if (solid (vattr_b[ii])) {
          if (memb) {
            vptr_b[ii] =  filter (pal16[memb], vptr_b[ii], coeff1, coeff2, vattr_b[ii], bld_map);
            vattr_b[ii] = ch_mode_mask;
          }
        }
      }
      affine_bx += affine_dx;
      affine_by += affine_dy;
    }
  }
}

finline 
void AffineMartixSP_Draw_Shadow (
           struct rasterizer_caps *rtc,
             uint16_t *oamb, 
             uint16_t slot,
             uint16_t tile_pitch,
     uint16_t *vattr_b,
     uintptr_t line,
     uintptr_t dim_x,
     uintptr_t dim_y,
     uintptr_t pos_x,
     uintptr_t pos_y)
{
  uint16_t *oam;
  uintptr_t virtual_pos_x;
  uintptr_t virtual_pos_y;
  uintptr_t virtual_dim_x;
  uintptr_t virtual_dim_y;
  uintptr_t tbid;
  intptr_t calc_left;
  intptr_t calc_right;
  intptr_t calc_top;
  intptr_t calc_bottom;
  intptr_t check_left;
  intptr_t check_right;
  intptr_t check_top;
  intptr_t check_bottom;
  intptr_t affine_bx;
  intptr_t affine_by;
  intptr_t affine_dx;
  intptr_t affine_dy;
  intptr_t affine_dmx;
  intptr_t affine_dmy;
  intptr_t cnt;
  int16_t *affbank;

  oam = & oamb[slot <<2];

  /* Get affine params */
  affbank = (int16_t *)& oamb[(oam[1] >> 9 & 31) << 4];
  affine_dx =affbank[3] ;
  affine_dmx =affbank[3+4] ;
  affine_dy =affbank[3+8] ;
  affine_dmy =affbank[3+12] ;

  if (line >= 160 && line <= 255)
    return ;

  if (oam[0] & 0x200) {
    /* TODO: Scanline 128-160 BUG. */
    virtual_pos_x = pos_x;
    virtual_pos_y = pos_y;
    virtual_dim_x = dim_x << 1;
    virtual_dim_y = dim_y << 1;
    check_left = pos_x + (dim_x >> 1);
    check_top = dim_y >> 1;
  } else {
    virtual_pos_x = pos_x;
    virtual_pos_y = pos_y;
    virtual_dim_x = dim_x;
    virtual_dim_y = dim_y;
    check_left = pos_x;
    check_top = 0;
  }
  virtual_pos_x &= 511;
  virtual_pos_y &= 255;

  if (virtual_pos_x >= 240 && (virtual_pos_x + virtual_dim_x) <= 512)
    return ;
  if (virtual_pos_y >= 160 && (virtual_pos_y + virtual_dim_y) <= 256)
    return ;

  calc_left = virtual_pos_x;
  calc_right = virtual_pos_x + virtual_dim_x;
  tbid = oam[2] & 1023;

  if ((virtual_pos_y + virtual_dim_y) > 256) {
    /* Cross screen bottom to up.*/
    uintptr_t phase = virtual_pos_y + virtual_dim_y - 1 & 255;
    if (!(line >= 0 && line <= phase))
      return ;
    /* Make affine guard negtive */
    calc_top = virtual_pos_y - 256;
    check_top = check_top + pos_y - 256;
    /* Get Y Offset in tile 
      x init vector always -dim_x/2 *affine param.
    */
  } else {
    if (line < virtual_pos_y) /* TODO : oam bug */
      return ;
    if (line > (virtual_pos_y + virtual_dim_y -1)) /* TODO : oam bug */
      return ;

    check_top += pos_y;
    calc_top = virtual_pos_y;
  }
  check_bottom = check_top+ dim_y;
  check_right = check_left + dim_x;
  calc_bottom = calc_top + virtual_dim_y;

  /* Calc init affine params */
  if (1) {
    /* Pos - Middle Point */
    intptr_t mid_x = (calc_left + calc_right) / 2;
    intptr_t mid_y = (calc_top + calc_bottom) / 2;
    intptr_t affoff_x = -(virtual_dim_x/2);
    intptr_t affoff_y = (intptr_t) line - mid_y;

    affine_bx = affine_dx*affoff_x+affine_dmx*affoff_y+mid_x*256;
    affine_by  = affine_dy*affoff_x+affine_dmy*affoff_y+mid_y*256;
  }

  if (oam[0] & 0x2000) {
    /* 256 PAL */
    for (cnt = virtual_pos_x; cnt != virtual_pos_x+virtual_dim_x; cnt++) {
      /* Check Range */
      intptr_t xpos = affine_bx >> 8;
      intptr_t ypos = affine_by >> 8;

      if (xpos >= check_left 
        && xpos < check_right 
         && ypos >= check_top
        && ypos < check_bottom)
      {
        intptr_t tile_x = xpos - check_left;
        intptr_t tile_y = ypos - check_top;
        intptr_t tile_mod8_x = tile_x & 7;
        intptr_t tile_mod8_y = tile_y & 7;
        intptr_t tid;
        uint8_t *chr;
        uint8_t memb;
        tile_x >>= 3;
        tile_y >>= 3;

        tid = tbid + tile_y * tile_pitch + tile_x *2;
        /* fetch tile mem */
        chr = (uint8_t *)GetSP_Chr256 (rtc, tid, tile_mod8_y);
        memb = chr[tile_mod8_x];
        if (memb) {
            vattr_b[cnt&255] |= SOLID_MODE_OBJWIN_MASK;
        }
      }
      affine_bx += affine_dx;
      affine_by += affine_dy;
    }
  } else {
    /* 16 PAL */
    for (cnt = virtual_pos_x; cnt != virtual_pos_x+virtual_dim_x; cnt++) {
      /* Check Range */
      intptr_t xpos = affine_bx >> 8;
      intptr_t ypos = affine_by >> 8;

      if (xpos >= check_left 
        && xpos < check_right 
         && ypos >= check_top
        && ypos < check_bottom)
      {
        intptr_t tile_x = xpos - check_left;
        intptr_t tile_y = ypos - check_top;
        intptr_t tile_mod8_x = tile_x & 7;
        intptr_t tile_mod8_y = tile_y & 7;
        intptr_t tid;
        uint8_t *chr;
        uint8_t memb;
        tile_x >>= 3;
        tile_y >>= 3;

        tid = tbid + tile_y * tile_pitch + tile_x;
        /* fetch tile mem */
        chr = (uint8_t *)GetSP_Chr16 (rtc, tid, tile_mod8_y);
        memb = chr[tile_mod8_x>>1];
        if (tile_mod8_x & 1)
          memb >>= 4;
        else 
          memb &= 15;

        if (memb) {
          vattr_b[cnt&255] |= SOLID_MODE_OBJWIN_MASK;
        }
      }
      affine_bx += affine_dx;
      affine_by += affine_dy;
    }
  }
}

finline 
void 
StdSP_Render (struct rasterizer_caps *rtc, 
                            const uint16_t mode_mask,
                            const uint16_t layerLevel, /* bit << 10 */
                            const kable en_effect,
                            const uint16_t bld_map,
                                 kable (*solid) (uint16_t back_attr),
                                 uint16_t coeff_b,
                            uint16_t coeff1, 
                            uint16_t coeff2)
{
  intptr_t cnt; 
  intptr_t line = rtc->gpu->line.blk;
  uint16_t *oam;
  intptr_t dim_x;
  intptr_t dim_y;
  int32_t x_coo;
  int32_t y_coo;
  intptr_t tile_id;
  int32_t tile_pitch;
  uint16_t *vattr_b = & rtc->gpu->vattr[ 8 ];
  uint16_t *vptr_b = & rtc->gpu->vbuf[rtc->gpu->vptr_pitch/2*rtc->gpu->line.blk + 8 ];
  uint16_t *vattr;
  uint16_t *vptr;
  uint16_t *vaptr; 
  uint16_t *vaptr_b = & rtc->gpu->vptr_cahce[0];
  uintptr_t tbid_t;
  uint16_t *pal16;
  uint16_t *pal256 = (uint16_t *) & rtc->gpu->palette16_b[512];
  uint16_t ch_mode_mask = SOLID_CHAN_SPRITE_MASK |  mode_mask;
  void *oamb = & rtc->gpu->oam[0];

  /* Maybe useless ??*/
  // memcpy (vaptr_b, vptr_b, 512);
  /* check sprite mapper,  when tile switch to next line */
  if (rtc->gpu->ctl.blk & 0x40) {
    /* 1D Mappering. continuous mapping */
    tile_pitch = 0;
  } else {
    /* 2D Mappering. pitch mapping */
    tile_pitch = 32;
  }

  for (cnt = 127; cnt != (intptr_t) -1; cnt--) {  

    oam = (uint16_t *)& rtc->gpu->oam[cnt<<3];
    if (       ((oam[0] & 0x300 ) == 0x200) 
      || (oam[0] & 0x800) 
      || ((oam[2] & 0xC00) != layerLevel) )
      continue;

    switch ((oam[1] & 0xC000) | (oam[0] >> 2 & 0x3000)) {
    case 0x0000: dim_x = 8; dim_y = 8; break;
    case 0x1000: dim_x =16; dim_y = 8; break;
    case 0x2000: dim_x = 8; dim_y =16; break;
    case 0x4000: dim_x =16; dim_y =16; break;
    case 0x5000: dim_x =32; dim_y = 8; break;
    case 0x6000: dim_x = 8; dim_y =32; break;
    case 0x8000: dim_x =32; dim_y =32; break;
    case 0x9000: dim_x =32; dim_y =16; break;
    case 0xA000: dim_x =16; dim_y =32; break;
    case 0xC000: dim_x =64; dim_y =64; break;
    case 0xD000: dim_x =64; dim_y =32; break;
    case 0xE000: dim_x =32; dim_y =64; break;
    default:
      DEBUG_BREAK ();
    }
    
    x_coo = oam[1] & 511; /* X Wrapround ??*/
    y_coo = oam[0] & 255; 
    tile_id = oam[2] & 1023;
    vattr = & vattr_b[x_coo];
    vptr = & vptr_b[x_coo];
    vaptr = & vaptr_b[x_coo];
    pal16 = & pal256[ (oam[2] >> 12) << 4];

    if (oam[0] & 0x2000) { /* 8bit pal~*/
       if ((tile_pitch & 32) == 0)
         tile_pitch = dim_x>> 2; /* XXX: mod 2^n~*/
       /* tile_id &= 1022; */
    } else  if ((tile_pitch & 32) == 0)
         tile_pitch = dim_x>> 3; 

    if (oam[1] & 0x2000) /* FIXME: maybe bug in complex attribute deal */
      tbid_t =tile_id + tile_pitch * (((line-(intptr_t) y_coo) & 0x80000000) ? ((dim_y - 1 - (  line-y_coo)) + 256) >> 3: (dim_y - 1 - (  line-y_coo)) >> 3);
    else 
       tbid_t =tile_id + tile_pitch * (((line-(intptr_t) y_coo) & 0x80000000) ? (line+256-y_coo) >> 3: (line-y_coo) >> 3);

    if (oam[0] & 0x100) {
      if (oam[0] & 0x400) {
        /*  If alpha is specified, it always executes regardless of the rendering type */
        AffineMartixSP_Draw (rtc, ch_mode_mask, bld_map, (uint16_t *)oamb, cnt, 
        tile_pitch,  
           vptr_b, 
           vattr_b,
           pal16,
           pal256,
           coeff1,
           coeff2, line, dim_x, dim_y, x_coo, y_coo, solid, alpha16);
        continue;
      } else {
        switch (rtc->opca) {
        case 0:
        AffineMartixSP_Draw (rtc, ch_mode_mask, bld_map, (uint16_t *)oamb, cnt, 
        tile_pitch,  
           vptr_b, 
           vattr_b,
           pal16,
           pal256,
           coeff1,
           coeff2, line, dim_x, dim_y, x_coo, y_coo, solid, pixcpy);
          break;
        case 1:
        AffineMartixSP_Draw (rtc, ch_mode_mask, bld_map, (uint16_t *)oamb, cnt, 
        tile_pitch,  
           vptr_b, 
           vattr_b,
           pal16,
           pal256,
           coeff1,
           coeff2, line, dim_x, dim_y, x_coo, y_coo, solid, alpha16);
          break;
        case 2:
        AffineMartixSP_Draw (rtc, ch_mode_mask, bld_map, (uint16_t *)oamb, cnt, 
        tile_pitch,  
           vptr_b, 
           vattr_b,
           pal16,
           pal256,
           coeff_b,
           coeff_b, line, dim_x, dim_y, x_coo, y_coo, solid, brightness_inc16);
          break;
        case 3:
        AffineMartixSP_Draw (rtc, ch_mode_mask, bld_map, (uint16_t *)oamb, cnt, 
        tile_pitch, 
           vptr_b, 
           vattr_b,
           pal16,
           pal256,
           coeff_b,
           coeff_b, line, dim_x, dim_y, x_coo, y_coo, solid, brightness_dec16);
          break;
        default:
          assert (0);
          break;
        }
      }
    } else {
      if (oam[0] & 0x400) {
        CommonSP_Draw (rtc, ch_mode_mask, bld_map, 
           vptr_b, 
           vattr_b,
           & oam[0],
           pal16,
           pal256,
           coeff1,
           coeff2, tbid_t, line, dim_x, dim_y, x_coo, y_coo, solid, alpha16);
        continue;
      } else {
        switch (rtc->opca) {
        case 0:
        CommonSP_Draw (rtc, ch_mode_mask, bld_map, 
           vptr_b, 
           vattr_b,
           & oam[0],
           pal16,
           pal256,
           coeff1,
           coeff2, tbid_t, line, dim_x, dim_y, x_coo, y_coo, solid, pixcpy);
          break;
        case 1:
        CommonSP_Draw (rtc, ch_mode_mask, bld_map, 
           vptr_b, 
           vattr_b,
           & oam[0],
           pal16,
           pal256,
           coeff1,
           coeff2, tbid_t, line, dim_x, dim_y, x_coo, y_coo, solid, alpha16);
          break;
        case 2:
        CommonSP_Draw (rtc, ch_mode_mask, bld_map, 
           vptr_b, 
           vattr_b,
           & oam[0],
           pal16,
           pal256,
           coeff_b,
           coeff_b, tbid_t, line, dim_x, dim_y, x_coo, y_coo, solid, brightness_inc16);
          break;
        case 3:
        CommonSP_Draw (rtc, ch_mode_mask, bld_map, 
           vptr_b, 
           vattr_b,
           & oam[0],
           pal16,
           pal256,
           coeff_b,
           coeff_b, tbid_t, line, dim_x, dim_y, x_coo, y_coo, solid, brightness_dec16);
          break;
        default:
          assert (0);
          break;
        }
      }
    }
  }
}

finline 
void sp_render_shadow (struct rasterizer_caps *const rtc)
{
  intptr_t cnt; 
  intptr_t line = rtc->gpu->line.blk;
  uint16_t *oam;
  intptr_t dim_x;
  intptr_t dim_y;
  int32_t x_coo;
  int32_t y_coo;
  intptr_t tile_id;
  int32_t tile_pitch;
  uint16_t *vattr_b = & rtc->gpu->vattr[ 8 ];
  uintptr_t tbid_t;
  void *oamb = & rtc->gpu->oam[0];

  /* check sprite mapper,  when tile switch to next line */
  if (rtc->gpu->ctl.blk & 0x40) {
    /* 1D Mappering. continuous mapping */
    tile_pitch = 0;
  } else {
    /* 2D Mappering. pitch mapping */
    tile_pitch = 32;
  }

  for (cnt = 127; cnt != (intptr_t) -1; cnt--) {
    
    oam = (uint16_t *)& rtc->gpu->oam[cnt<<3];
    if ((oam[0] & 0x300 ) == 0x200) {
      continue;
    } else if ((oam[0] & 0xC00) != 0x800)
      continue;

    switch ((oam[1] & 0xC000) | (oam[0] >> 2 & 0x3000)) {
    case 0x0000: dim_x = 8; dim_y = 8; break;
    case 0x1000: dim_x =16; dim_y = 8; break;
    case 0x2000: dim_x = 8; dim_y =16; break;
    case 0x4000: dim_x =16; dim_y =16; break;
    case 0x5000: dim_x =32; dim_y = 8; break;
    case 0x6000: dim_x = 8; dim_y =32; break;
    case 0x8000: dim_x =32; dim_y =32; break;
    case 0x9000: dim_x =32; dim_y =16; break;
    case 0xA000: dim_x =16; dim_y =32; break;
    case 0xC000: dim_x =64; dim_y =64; break;
    case 0xD000: dim_x =64; dim_y =32; break;
    case 0xE000: dim_x =32; dim_y =64; break;
    default:
      DEBUG_BREAK ();
    }
    
    x_coo = oam[1] & 511; /* X Wrapround ??*/
    y_coo = oam[0] & 255; 
    tile_id = oam[2] & 1023;

    if (oam[0] & 0x2000) { /* 8bit pal~*/
       if ((tile_pitch & 32) == 0)
         tile_pitch = dim_x>> 2; /* XXX: mod 2^n~*/
    } else  if ((tile_pitch & 32) == 0)
         tile_pitch = dim_x>> 3; 

    tbid_t = tile_id + tile_pitch * (((line-(intptr_t) y_coo) & 0x80000000) ? (line+256-y_coo) >> 3: (line-y_coo) >> 3); /* XXX:*/

    if (oam[0] & 0x100) 
        AffineMartixSP_Draw_Shadow (rtc, (uint16_t *)oamb, cnt, tile_pitch, 
           vattr_b, line, dim_x, dim_y, x_coo, y_coo);
    else 
        CommonSP_Draw_Shadow (rtc, vattr_b, & oam[0], 
              tbid_t, line, dim_x, dim_y, x_coo, y_coo);
  }
}
finline
kable solid_nowin (uint16_t attr)
{
  return true;
} 
finline
kable solid_out (uint16_t attr)
{
  return (attr & SOLID_MODE_FULL_MASK) ? false : true;
} 
finline
kable solid_win1 (uint16_t attr)
{
  return (attr & SOLID_MODE_WIN1_MASK) ? true : false;
} 
finline
kable solid_win0 (uint16_t attr)
{
  return (attr & SOLID_MODE_WIN0_MASK) ? true : false;
} 
finline
kable solid_shadow (uint16_t attr)
{
  return (attr & SOLID_MODE_OBJWIN_MASK) ? true : false;
} 
static 
void StdBG_Render_shadow
            (struct rasterizer_caps *rtc, 
                   uint16_t chan_mask,
                   uint16_t bld_map,
                   uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, pixcpy); break;
  case 1: CommonBG_Render (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, alpha16); break;
  case 2: CommonBG_Render (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_inc16); break;
  case 3: CommonBG_Render (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_nowindow
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, pixcpy); break;
  case 1: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, alpha16); break;
  case 2: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_inc16); break;
  case 3: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_win0
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, pixcpy); break;
  case 1: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, alpha16); break;
  case 2: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_inc16); break;
  case 3: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_win1
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, pixcpy); break;
  case 1: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, alpha16); break;
  case 2: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_inc16); break;
  case 3: CommonBG_Render (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_outwin
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, pixcpy); break;
  case 1: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, alpha16); break;
  case 2: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_inc16); break;
  case 3: CommonBG_Render (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_Rot_shadow
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, pixcpy); break;
  case 1: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, alpha16); break;
  case 2: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_inc16); break;
  case 3: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_Rot_win0
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, pixcpy); break;
  case 1: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, alpha16); break;
  case 2: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_inc16); break;
  case 3: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_Rot_win1
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, pixcpy); break;
  case 1: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, alpha16); break;
  case 2: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_inc16); break;
  case 3: CommonBG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_Rot_outwin
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, pixcpy); break;
  case 1: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, alpha16); break;
  case 2: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_inc16); break;
  case 3: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void StdBG_Render_Rot_nowindow
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, pixcpy); break;
  case 1: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, alpha16); break;
  case 2: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_inc16); break;
  case 3: CommonBG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode3_Render_shadow
            (struct rasterizer_caps *rtc, 
                   uint16_t chan_mask,
                   uint16_t bld_map,
                   uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, pixcpy); break;
  case 1: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, alpha16); break;
  case 2: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_inc16); break;
  case 3: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode3_Render_nowindow
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, pixcpy); break;
  case 1: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, alpha16); break;
  case 2: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_inc16); break;
  case 3: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode3_Render_win0
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, pixcpy); break;
  case 1: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, alpha16); break;
  case 2: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_inc16); break;
  case 3: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode3_Render_win1
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, pixcpy); break;
  case 1: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, alpha16); break;
  case 2: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_inc16); break;
  case 3: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode3_Render_outwin
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, pixcpy); break;
  case 1: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, alpha16); break;
  case 2: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_inc16); break;
  case 3: Mode3Bitmap8Full_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode4_Render_shadow
            (struct rasterizer_caps *rtc, 
                   uint16_t chan_mask,
                   uint16_t bld_map,
                   uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, pixcpy); break;
  case 1: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, alpha16); break;
  case 2: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_inc16); break;
  case 3: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode4_Render_nowindow
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, pixcpy); break;
  case 1: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, alpha16); break;
  case 2: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_inc16); break;
  case 3: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode4_Render_win0
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, pixcpy); break;
  case 1: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, alpha16); break;
  case 2: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_inc16); break;
  case 3: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode4_Render_win1
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, pixcpy); break;
  case 1: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, alpha16); break;
  case 2: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_inc16); break;
  case 3: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode4_Render_outwin
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, pixcpy); break;
  case 1: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, alpha16); break;
  case 2: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_inc16); break;
  case 3: Mode4Bitmap4SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode5_Render_shadow
            (struct rasterizer_caps *rtc, 
                   uint16_t chan_mask,
                   uint16_t bld_map,
                   uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, pixcpy); break;
  case 1: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, alpha16); break;
  case 2: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_inc16); break;
  case 3: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_OBJWIN_MASK, bld_map, coeff1, coeff2, solid_shadow, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode5_Render_nowindow
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, pixcpy); break;
  case 1: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, alpha16); break;
  case 2: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_inc16); break;
  case 3: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_nowin, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode5_Render_win0
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, pixcpy); break;
  case 1: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, alpha16); break;
  case 2: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_inc16); break;
  case 3: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN0_MASK, bld_map, coeff1, coeff2, solid_win0, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode5_Render_win1
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, pixcpy); break;
  case 1: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, alpha16); break;
  case 2: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_inc16); break;
  case 3: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask | SOLID_MODE_WIN1_MASK, bld_map, coeff1, coeff2, solid_win1, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}
static 
void Mode5_Render_outwin
            (struct rasterizer_caps *rtc, 
                               uint16_t chan_mask,
                   uint16_t bld_map,
                            uint16_t coeff1, uint16_t coeff2)
{
  switch (rtc->opca) {
  case 0: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, pixcpy); break;
  case 1: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, alpha16); break;
  case 2: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_inc16); break;
  case 3: Mode5Bitmap8SwapBuffer_BG_Render_Rot (rtc, chan_mask, bld_map, coeff1, coeff2, solid_out, brightness_dec16); break;
  default: DEBUG_BREAK ();
  }
}

finline 
void vec_memset16a512b (void *const mem, const uint16_t mask) {
  __m128i maskvec = _mm_shufflelo_epi16 (*(__m128i *)& mask, 0); /* TODO: pshufb done */
  __m128i *smemptr = (__m128i *) mem;
  maskvec = _mm_unpacklo_epi16 (maskvec, maskvec);

#undef SIMD_GUP_
#define SIMD_GUP_(x)\
  _mm_store_si128 (& smemptr[(x)+0], maskvec);\
  _mm_store_si128 (& smemptr[(x)+1], maskvec);\
  _mm_store_si128 (& smemptr[(x)+2], maskvec);\
  _mm_store_si128 (& smemptr[(x)+3], maskvec);\
  _mm_store_si128 (& smemptr[(x)+4], maskvec);\
  _mm_store_si128 (& smemptr[(x)+5], maskvec);\
  _mm_store_si128 (& smemptr[(x)+6], maskvec);\
  _mm_store_si128 (& smemptr[(x)+7], maskvec);

  SIMD_GUP_(0)
  SIMD_GUP_(8)
  SIMD_GUP_(16)
  SIMD_GUP_(24)
}

void win_render (struct gpu *gpu, uintptr_t *sort4_t, 
                    struct rasterizer_caps *rtinfos_t,
                      const uint16_t mode_mask,
                      const uint16_t winIo_Shift, /* require: must shift to high for win0/outwin io register */
                      const uint16_t coeff_b,
                      const uint16_t coeff_a1,
                      const uint16_t coeff_a2,
                      const uint16_t mode,
                               uint16_t en_bgmask,
                               kable (*sp_solid) (uint16_t back_attr),
                               void (*bg_render_std) (struct rasterizer_caps *rtc,  uint16_t chan_mask, uint16_t bld_map, uint16_t coeff1, uint16_t coeff2),
                               void (*bg_render_rot) (struct rasterizer_caps *rtc,  uint16_t chan_mask, uint16_t bld_map, uint16_t coeff1, uint16_t coeff2),
                               void (*mode3_render_rot) (struct rasterizer_caps *rtc,  uint16_t chan_mask, uint16_t bld_map, uint16_t coeff1, uint16_t coeff2),
                               void (*mode4_render_rot) (struct rasterizer_caps *rtc,  uint16_t chan_mask, uint16_t bld_map, uint16_t coeff1, uint16_t coeff2),                            
                               void (*mode5_render_rot) (struct rasterizer_caps *rtc,  uint16_t chan_mask, uint16_t bld_map, uint16_t coeff1, uint16_t coeff2) )
{
  struct rasterizer_caps rtinfos[5];
  uintptr_t sort[4]; 
  uintptr_t layer_cur;
  uintptr_t layer_van;
  uintptr_t cnt;
  uint16_t bld_map;
  uint16_t spbld_mask;
  const uintptr_t enb_mask = 0x100;
  const uintptr_t sp_mask =  enb_mask << 4;
  const uintptr_t eff_mask = 0x2000;
  const kable en_sprite = (en_bgmask & sp_mask) && (winIo_Shift & sp_mask);
  const kable en_effect = !! (winIo_Shift & eff_mask);

  memcpy (rtinfos, rtinfos_t, sizeof (rtinfos));
  memcpy (sort, sort4_t, sizeof (sort));

  layer_cur = 3;
  layer_van = 3;
  spbld_mask = 0;

  if (gpu->bld_ctl.blk & sp_mask)
    spbld_mask |= SOLID_CHAN_SPRITE_MASK;

  if (en_effect == false) {
    rtinfos[0].opca = 
    rtinfos[1].opca = 
    rtinfos[2].opca = 
    rtinfos[3].opca = 
    rtinfos[4].opca = 0;
  }

#define CALC_PREV_PRI_BLD()                   \
  do {                                        \
    uintptr_t id;                             \
    bld_map = 0;                              \
    for (  id = 0; id != cnt; id++) {         \
         const uintptr_t ch_ac = sort[id] & 3; \
         if (gpu->bld_ctl.blk & enb_mask << ch_ac)   \
           bld_map |= SOLID_CHAN_BG_BASE_MASK << ch_ac; \
    }                                                   \
  } while (0)
  for (cnt = 0; cnt != 4; cnt++) { 
    const uintptr_t bga_isa = sort[cnt];
    const uintptr_t ch = bga_isa & 3;
    const uintptr_t en_mask = enb_mask << ch;

    if (en_bgmask & winIo_Shift & en_mask) {

      if (  en_sprite != false ) {

        layer_van = layer_cur;
        layer_cur = bga_isa >> 8 & 3;
        
        /* Shadow window enable sprite ??*/
        if (layer_van != layer_cur) {
          uintptr_t layer_start;
          
          for ( layer_start = layer_van; 
                layer_start != layer_cur;
                layer_start--){
          /* Collect target 2 blend/alpha mask  
                It's important to assume that sprite doesn't mix with itself.
          */
                CALC_PREV_PRI_BLD();
                
                StdSP_Render (& rtinfos[4], mode_mask,
                  layer_start << 10,
                        en_effect,   bld_map, 
                 sp_solid,
                  coeff_b,  coeff_a1,  coeff_a2);
          }
        }
      }
      CALC_PREV_PRI_BLD();
      bld_map |= spbld_mask;
     // if (ch == 0)
     //   continue;
      switch (mode) {
      case 1: 
        if (ch != 2)
      case 0: 
          bg_render_std (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2); 
        else 
      case 2:
          bg_render_rot (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2); 
        break;
      case 3:
        if (ch == 2)
          mode3_render_rot (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2); 
        break;
      case 4:
        if (ch == 2)
          mode4_render_rot (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2); 
        break;
      case 5:
        if (ch == 2)
          mode5_render_rot (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2); 
      default:
        break;
      }
    }
  }
  if (en_sprite != false) {
    CALC_PREV_PRI_BLD();
    
    for ( ; layer_cur != (uintptr_t) -1;
             layer_cur--){

          StdSP_Render (& rtinfos[4], mode_mask,
            layer_cur << 10,
                  en_effect,   bld_map, 
            sp_solid,
            coeff_b,  coeff_a1,  coeff_a2);
    }
  }
}

finline
void nowin_render (struct gpu * gpu, uintptr_t * sort4_t, 
                    struct rasterizer_caps * rtinfos_t,
                      const uint16_t coeff_b,
                      const uint16_t coeff_a1,
                      const uint16_t coeff_a2,
                      const uint16_t mode,
                               uint16_t en_bgmask )
{
  struct rasterizer_caps rtinfos[5];
  uintptr_t sort[4]; 
  uintptr_t layer_cur;
  uintptr_t layer_van;
  uintptr_t cnt;
  uint16_t bld_map;
  uint16_t spbld_mask;
  const uintptr_t enb_mask = 0x100;
  const uintptr_t sp_mask =  enb_mask << 4;
  const kable en_sprite = !! (en_bgmask & sp_mask);

  memcpy (rtinfos, rtinfos_t, sizeof (rtinfos));
  memcpy (sort, sort4_t, sizeof (sort));

  layer_cur = 3;
  layer_van = 3;
  spbld_mask = 0;

  if (gpu->bld_ctl.blk & sp_mask)
    spbld_mask |= SOLID_CHAN_SPRITE_MASK;

  for (cnt = 0; cnt != 4; cnt++) { 
    const uintptr_t bga_isa = sort[cnt];
    const uintptr_t ch = bga_isa & 3;
    const uintptr_t en_mask = enb_mask << ch;

    if (en_bgmask & en_mask) {

      if (  en_sprite != false ) {

        layer_van = layer_cur;
        layer_cur = bga_isa >> 8 & 3;
        
        /* Shadow window enable sprite ??*/
        if (layer_van != layer_cur) {
          uintptr_t layer_start;
          
          for ( layer_start = layer_van; 
                layer_start != layer_cur;
                layer_start--){
          /* Collect target 2 blend/alpha mask  
                It's important to assume that sprite doesn't mix with itself.
          */
                CALC_PREV_PRI_BLD();
                
                StdSP_Render (& rtinfos[4], 0,
                  layer_start << 10,
                        true,   bld_map, 
                 solid_nowin,
                  coeff_b,  coeff_a1,  coeff_a2);
          }
        }
      }
      CALC_PREV_PRI_BLD();
      bld_map |= spbld_mask;
      // if (ch != 0)
     //   continue;
      switch (mode) {
      case 1: 
        if (ch != 2)
      case 0: 
          StdBG_Render_nowindow (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2); 
        else 
      case 2:
          StdBG_Render_Rot_nowindow (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2); 
        break;
      case 3:
        if (ch == 2)
          Mode3_Render_nowindow (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2);
        break;
      case 4:
        if (ch == 2)
          Mode4_Render_nowindow (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2);
        break;
      case 5:
        if (ch == 2)
          Mode5_Render_nowindow (& rtinfos[ch], SOLID_CHAN_BG_BASE_MASK << ch, bld_map,  rtinfos[ch].opca == 1 ? coeff_a1 : coeff_b, coeff_a2);
      default:
        break;
      }
    }
  }
  if (en_sprite != false) {
    CALC_PREV_PRI_BLD();
    
    for ( ; layer_cur != (uintptr_t) -1;
             layer_cur--){

          StdSP_Render (& rtinfos[4], 0,
            layer_cur << 10,
                  true,   bld_map, 
            solid_nowin,
            coeff_b,  coeff_a1,  coeff_a2);
    }
  }
}

void gpu_scanline (struct gpu *const gpu) {

  struct rasterizer_caps rtinfos[5];

  kable win_done = false;
  kable win0_done = false;
  kable win1_done = false;
  kable winobj_done = false;
  uintptr_t bg_enable_mask = gpu->ctl.blk;
  uintptr_t cnt;          
  uintptr_t sort[4] = { 0x0000 | gpu->chan[0].ctl.blk << 8 & 0x300,                    
                                 0x0001 | gpu->chan[1].ctl.blk << 8 & 0x300,
                                 0x0002 | gpu->chan[2].ctl.blk << 8 & 0x300,
                                 0x0003 | gpu->chan[3].ctl.blk << 8 & 0x300 };
  uintptr_t n;

  const uintptr_t gpu_mode = gpu->ctl.blk & 7;
  const uint16_t coeff_b_t =  gpu->bri_args.blk & 31;
  const uint16_t coeff_a1_t = gpu->bld_args.blk & 31;
  const uint16_t coeff_a2_t = gpu->bld_args.blk >> 8 & 31;
  const uint16_t coeff_b =  (coeff_b_t > 16) ? 16 : coeff_b_t;
  const uint16_t coeff_a1 = (coeff_a1_t > 16) ? 16 : coeff_a1_t;
  const uint16_t coeff_a2 = (coeff_a2_t > 16) ? 16 : coeff_a2_t;
           uint16_t *const vbptr = & gpu->vbuf[gpu->vptr_pitch/2*gpu->line.blk + 8];

  if (gpu->ctl.blk & 0x2000) 
    win0_done = true;
  if (gpu->ctl.blk & 0x4000) 
    win1_done = true;
  if (gpu->ctl.blk & 0x8000) 
    winobj_done = true;
  if (winobj_done || win1_done ||  win0_done)
    win_done = true;

  /* TODO: More rigorous  testing, repeater code emmm... */
  switch (gpu_mode) {
  case 0:
    /* MODE-0 ALIVE BG : BG0 BG1 BG2 BG3 - 2D MAPPER BASE NES/CGB Tile extend (4Bit/8Bit Switchable) */ 
    break;
  case 1:
    /* MODE-1 ALIVE BG : BG0 BG1 BG2(2 is AffineBG) */ 
    bg_enable_mask &= ~(1 << 11);
    break;
  case 2:
    /* MODE-2 ALIVE BG : BG2 BG3(2, 3 is AffineBG) */ 
    bg_enable_mask &= ~(3 << 8);
    break;
  case 3:
    /* MODE-3 240x160 pixels, 32768 colors 75KB */
  case 4:
    /* MODE-4 240x160 pixels, 255 colors 2 frame */
  case 5:
    /* MODE-5 160*128 pixels, 32768 colors 2 frame */
    bg_enable_mask &= 0x400;
    break;
  case 6:
  case 7:
    DEBUG_BREAK ();
    break;
  }

  /* fill backdrop color 
      see. GBA Programming Manual v1.1 :: 7 Color Palettes::3) Color 0 Transparency (page 73)
  */
  vec_memset16a512b (vbptr, * (uint16_t *)& gpu->palette16_b[0]);
  vec_memset16a512b (& gpu->vattr[8], 0);

  for (cnt = 0; cnt != sizeof (rtinfos)/ sizeof (rtinfos[0]); cnt++) {

    const uint8_t mosaic = gpu->mosaic.blk >> ((cnt == 4) ? 8 : 0) & 0xFF;

    rtinfos[cnt].chan = & gpu->chan[cnt];
    rtinfos[cnt].chanId = cnt;
    rtinfos[cnt].gpu = gpu;
    rtinfos[cnt].order = 0;
    rtinfos[cnt].sub_order = 0;
    rtinfos[cnt].chr_base = 0;
    rtinfos[cnt].opca = 0;
    rtinfos[cnt].winId = cnt & 1;
    rtinfos[cnt].solid = 0;
    rtinfos[cnt].interp_x = false;
    rtinfos[cnt].interp_y = false;
    rtinfos[cnt].interp_xvec = mosaic & 0x0F;
    rtinfos[cnt].interp_yvec = mosaic >> 4 & 0x0F;

    if ( (rtinfos[cnt].chan->ctl.blk & 64)) {
      if (rtinfos[cnt].interp_xvec != 0)
        rtinfos[cnt].interp_x = true;
      if (rtinfos[cnt].interp_yvec != 0)
        rtinfos[cnt].interp_y = true;
    }
    if (gpu->bld_ctl.blk & 1 << cnt) 
      rtinfos[cnt].opca = (gpu->bld_ctl.blk >> 6) & 3; 
    else ;
  }

  for (cnt = 0; cnt != 3; cnt++) { 
    int c;
    for (c = cnt + 1; c != 4; c++) 
      if (sort[cnt] < sort[c]) {
        uintptr_t temp = sort[c];
        sort[c] = sort[cnt];
        sort[cnt] = temp;
      }
  }
#define VISUAL_GBA_LOGIC_FORGOTTEN
#ifndef VISUAL_GBA_LOGIC_FORGOTTEN
  if (win_done != false) {
    if (winobj_done != false) {
      sp_render_shadow (& rtinfos[4]);
      win_render (gpu, & sort[0], & rtinfos[0], SOLID_MODE_OBJWIN_MASK, gpu->win_out.blk, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_shadow, StdBG_Render_shadow, StdBG_Render_Rot_shadow);
    }
    if (win1_done != false) {
        uintptr_t left = gpu->clip_x[1].blk >> 8 & 255;
        uintptr_t right = (gpu->clip_x[1].blk & 255);
        uintptr_t top = gpu->clip_y[1].blk >> 8 & 255;
        uintptr_t bottom = (gpu->clip_y[1].blk & 255);
        uint16_t *vattr_b = & gpu->vattr[8];

        if (!(left | right))
          goto _out;
        if (!(top | bottom))
          goto _out;
        if (left == right)
          goto _out;
        if (bottom == top)
          goto _out;

        if (right >= 240 || left >= right)
          right = 240;
        if (bottom >= 160 || top >= bottom)
          bottom = 160;
        if (!(gpu->line.blk >top 
          && gpu->line.blk < bottom))
          goto _out;

        for (n = left; n != right; n++) {
            vattr_b[n] = SOLID_MODE_WIN1_MASK;
        }
       win_render (gpu, & sort[0], & rtinfos[0], SOLID_MODE_WIN1_MASK, gpu->win_in.blk, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_win1, StdBG_Render_win1, StdBG_Render_Rot_win1);
    }
    _out:
    if (win0_done != false) {
        uintptr_t left = gpu->clip_x[0].blk >> 8 & 255;
        uintptr_t right = (gpu->clip_x[0].blk & 255);
        uintptr_t top = gpu->clip_y[0].blk >> 8 & 255;
        uintptr_t bottom = (gpu->clip_y[0].blk & 255);
        uint16_t *vattr_b = & gpu->vattr[8];

        if (!(left | right))
          goto _out2;
        if (!(top | bottom))
          goto _out2;
        if (left == right)
          goto _out2;
        if (bottom == top)
          goto _out2;
        if (right >= 240 || left >= right)
          right = 240;
        if (bottom >= 160 || top >= bottom)
          bottom = 160;
        if (left >= right)
         goto _out2;
        if (top >= bottom)
         goto _out2;
        if (!(gpu->line.blk >top 
          && gpu->line.blk < bottom))
          goto _out2;

        for (n = left; n != right; n++) {
            vattr_b[n] = SOLID_MODE_WIN0_MASK;
        }
       win_render (gpu, & sort[0], & rtinfos[0], SOLID_MODE_WIN0_MASK, gpu->win_in.blk << 8, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_win0, StdBG_Render_win0, StdBG_Render_Rot_win0);
    }
    _out2:
       win_render (gpu, & sort[0], & rtinfos[0], 0, gpu->win_out.blk << 8, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_out, StdBG_Render_outwin, StdBG_Render_Rot_outwin);
  } else  nowin_render (gpu, sort, rtinfos, coeff_b, coeff_a1, coeff_a2, gpu_mode, bg_enable_mask);
#else 
  if (win_done != false) {
    if (winobj_done != false) {
      sp_render_shadow (& rtinfos[4]);
      win_render (gpu, & sort[0], & rtinfos[0], SOLID_MODE_OBJWIN_MASK, gpu->win_out.blk, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_shadow, StdBG_Render_shadow, StdBG_Render_Rot_shadow, Mode3_Render_shadow, Mode4_Render_shadow, Mode5_Render_shadow);
    }
    if (win1_done != false) {
        uintptr_t left = gpu->clip_x[1].blk >> 8 & 255;
        uintptr_t right = (gpu->clip_x[1].blk & 255);
        uintptr_t top = gpu->clip_y[1].blk >> 8 & 255;
        uintptr_t bottom = (gpu->clip_y[1].blk & 255);
        uint16_t *vattr_b = & gpu->vattr[8];

        if (top > bottom)
          if (!(gpu->line.blk >= top || gpu->line.blk < bottom))
            goto _out;
          else {}
        else if (top < bottom)
          if (!(gpu->line.blk >= top && gpu->line.blk < bottom))
            goto _out;
          else {}

        if (left > right) {
          for (n = left; n < 256; n++) {
            vattr_b[n] = SOLID_MODE_WIN1_MASK;
            vbptr[n] = *(uint16_t *)& gpu->palette16_b[0];
          }
          for (n = right; n > 0; n--) {
            vattr_b[n] = SOLID_MODE_WIN1_MASK;
            vbptr[n] = *(uint16_t *)& gpu->palette16_b[0];
          }
        } else {
          for (n = left; n != right; n++) {
            vattr_b[n] = SOLID_MODE_WIN1_MASK;
            vbptr[n] = *(uint16_t *)& gpu->palette16_b[0];
          }
        }
       win_render (gpu, & sort[0], & rtinfos[0], SOLID_MODE_WIN1_MASK, gpu->win_in.blk, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_win1, StdBG_Render_win1, StdBG_Render_Rot_win1, Mode3_Render_win1, Mode4_Render_win1, Mode5_Render_win1);
    }
    _out:
    if (win0_done != false) {
        uintptr_t left = gpu->clip_x[0].blk >> 8 & 255;
        uintptr_t right = (gpu->clip_x[0].blk & 255);
        uintptr_t top = gpu->clip_y[0].blk >> 8 & 255;
        uintptr_t bottom = (gpu->clip_y[0].blk & 255);
        uint16_t *vattr_b = & gpu->vattr[8];

        if (top > bottom)
          if (!(gpu->line.blk >= top || gpu->line.blk < bottom))
            goto _out2;
          else {}
        else if (top < bottom)
          if (!(gpu->line.blk >= top && gpu->line.blk < bottom))
            goto _out2;
          else {}

        if (left > right) {
          for (n = left; n < 256; n++) {
            vattr_b[n] = SOLID_MODE_WIN0_MASK;
            vbptr[n] = *(uint16_t *)& gpu->palette16_b[0];
          }
          for (n = right; n > 0; n--) {
            vattr_b[n] = SOLID_MODE_WIN0_MASK;
            vbptr[n] = *(uint16_t *)& gpu->palette16_b[0];
          }
        } else {
          for (n = left; n != right; n++) {
            vattr_b[n] = SOLID_MODE_WIN0_MASK;
            vbptr[n] = *(uint16_t *)& gpu->palette16_b[0];
          }
        }
       win_render (gpu, & sort[0], & rtinfos[0], SOLID_MODE_WIN0_MASK, gpu->win_in.blk << 8, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_win0, StdBG_Render_win0, StdBG_Render_Rot_win0, Mode3_Render_win0, Mode4_Render_win0, Mode5_Render_win0);
    }
    _out2:
       win_render (gpu, & sort[0], & rtinfos[0], 0, gpu->win_out.blk << 8, coeff_b, coeff_a1, coeff_a2, gpu_mode, 
        bg_enable_mask, solid_out, StdBG_Render_outwin, StdBG_Render_Rot_outwin, Mode3_Render_outwin, Mode4_Render_outwin, Mode5_Render_outwin);
  } else  nowin_render (gpu, sort, rtinfos, coeff_b, coeff_a1, coeff_a2, gpu_mode, bg_enable_mask);
#endif 
}

finline 
uint16_t gpu_read (struct gpu *gpu, uint32_t addr) {
  switch (addr & 0x3FF) {
  case 0x000: return gpu->ctl.blk;
  case 0x002: return gpu->col_swap.blk;
  case 0x004: return gpu->status.blk;
  case 0x006: return gpu->line.blk;
  case 0x008: return gpu->chan[0].ctl.blk;
  case 0x00A: return gpu->chan[1].ctl.blk;
  case 0x00C: return gpu->chan[2].ctl.blk;
  case 0x00E: return gpu->chan[3].ctl.blk;
  case 0x048: return gpu->win_in.blk & 0x3F3F;
  case 0x04A: return gpu->win_out.blk & 0x3F3F;
  case 0x050: return gpu->bld_ctl.blk & 0x3FFF;
  case 0x052: return gpu->bld_args.blk & 0x1F1F;
  case 0x04E: return gpu->ugpio_4E.blk;
  case 0x056: return gpu->ugpio_56.blk;
  case 0x010:
  case 0x012:
  case 0x014:
  case 0x016:
  case 0x018:
  case 0x01A:
  case 0x01C:
  case 0x01E:
  case 0x020:
  case 0x022:
  case 0x024:
  case 0x026:
  case 0x028:
  case 0x02A:
  case 0x02C: return gpu->agb->arm7.opcode[0];
  default:
    DEBUG_BREAK ();
    return 0;
  }
}

finline 
void gpu_write (struct gpu *gpu, uint32_t addr, uint16_t value, const IO_WRITE_BLOCK block) {

#define blk_done16_8(io_blk) \
  (block == IO_WRITE_16) ? ((io_blk).blk = value) : ((io_blk).blk8[addr&1] = (uint8_t) (value))
#define blk_done16_8_ext(io_blk, value16, value_lo8, value_hi8) \
  (block == IO_WRITE_16) ? ((io_blk).blk = (value16)) \
                      : (   (addr&1)    ?((io_blk).blk8[addr&1] = (uint8_t) (value_hi8)) \
                                              : ((io_blk).blk8[addr&1] = (uint8_t) (value_lo8)))
#define blk_done16_8_x32(io_blk, blk16_pos) \
  (block == IO_WRITE_16) ? ((io_blk).blk16[blk16_pos] = value) : (  fto_uint8p ((io_blk).blk16[blk16_pos])[addr & 1] = (uint8_t) (value))

  switch (addr & 0x7FE) {
  case 0x000: blk_done16_8 (gpu->ctl); break;
  case 0x002: blk_done16_8 (gpu->col_swap); break;
  case 0x004: blk_done16_8_ext (gpu->status,gpu->status.blk & 0x47 | value & ~0x47, gpu->status.blk & 0x47 | value & ~0x47, value); break;
  case 0x008: blk_done16_8_ext (gpu->chan[0].ctl ,value & ~0x2000, value & ~0x2000, value); break;
  case 0x00A: blk_done16_8_ext (gpu->chan[1].ctl ,value & ~0x2000, value & ~0x2000, value); break;
  case 0x00C: blk_done16_8_ext (gpu->chan[2].ctl ,value, value, value); break;
  case 0x00E: blk_done16_8_ext (gpu->chan[3].ctl ,value, value, value); break;
  case 0x010: blk_done16_8 (gpu->chan[0].loopy_x); break;
  case 0x012: blk_done16_8 (gpu->chan[0].loopy_y); break;
  case 0x014: blk_done16_8 (gpu->chan[1].loopy_x); break;
  case 0x016: blk_done16_8 (gpu->chan[1].loopy_y); break;
  case 0x018: blk_done16_8 (gpu->chan[2].loopy_x); break;
  case 0x01A: blk_done16_8 (gpu->chan[2].loopy_y); break;
  case 0x01C: blk_done16_8 (gpu->chan[3].loopy_x); break;
  case 0x01E: blk_done16_8 (gpu->chan[3].loopy_y); break;
  case 0x020: blk_done16_8 (gpu->chan[2].dx); break;
  case 0x022: blk_done16_8 (gpu->chan[2].dmx);  break;
  case 0x024: blk_done16_8 (gpu->chan[2].dy); break;
  case 0x026: blk_done16_8 (gpu->chan[2].dmy); break;
  case 0x030: blk_done16_8 (gpu->chan[3].dx); break;
  case 0x032: blk_done16_8 (gpu->chan[3].dmx); break;
  case 0x034: blk_done16_8 (gpu->chan[3].dy); break;
  case 0x036: blk_done16_8 (gpu->chan[3].dmy); break;
  case 0x040: blk_done16_8 (gpu->clip_x[0]); break;
  case 0x042: blk_done16_8 (gpu->clip_x[1]); break;
  case 0x044: blk_done16_8 (gpu->clip_y[0]); break;
  case 0x046: blk_done16_8 (gpu->clip_y[1]); break;
  case 0x048: blk_done16_8 (gpu->win_in); break;
  case 0x04A: blk_done16_8 (gpu->win_out); break;
  case 0x04C: blk_done16_8 (gpu->mosaic); break;
  case 0x050: blk_done16_8 (gpu->bld_ctl); break;
  case 0x052: blk_done16_8 (gpu->bld_args); break;
  case 0x054: blk_done16_8 (gpu->bri_args); break;
  case 0x04E: blk_done16_8 (gpu->ugpio_4E); break;
  case 0x056: blk_done16_8 (gpu->ugpio_56); break;
  case 0x028: 
    blk_done16_8_x32 (gpu->chan[2].ref_x, 0);
    gpu->chan[2].loopy_dmx.blk = gpu->chan[2].ref_x.blk;
    if (gpu->chan[2].loopy_dmx.blk & 0x8000000) 
      gpu->chan[2].loopy_dmx.blk |= 0xF0000000;
    break;
  case 0x02A:
    blk_done16_8_x32 (gpu->chan[2].ref_x, 1);
    gpu->chan[2].loopy_dmx.blk = gpu->chan[2].ref_x.blk;
    if (gpu->chan[2].loopy_dmx.blk & 0x8000000) 
      gpu->chan[2].loopy_dmx.blk |= 0xF0000000;
    break;
  case 0x02C: 
    blk_done16_8_x32 (gpu->chan[2].ref_y, 0);
    gpu->chan[2].loopy_dmy.blk = gpu->chan[2].ref_y.blk;
    if (gpu->chan[2].loopy_dmy.blk & 0x8000000) 
      gpu->chan[2].loopy_dmy.blk |= 0xF0000000;
    break;
  case 0x02E:
    blk_done16_8_x32 (gpu->chan[2].ref_y, 1);
    gpu->chan[2].loopy_dmy.blk = gpu->chan[2].ref_y.blk;
    if (gpu->chan[2].loopy_dmy.blk & 0x8000000) 
      gpu->chan[2].loopy_dmy.blk |= 0xF0000000;
    break;
  case 0x038: 
    blk_done16_8_x32 (gpu->chan[3].ref_x, 0);
    gpu->chan[3].loopy_dmx.blk = gpu->chan[3].ref_x.blk;
    if (gpu->chan[3].loopy_dmx.blk & 0x8000000) 
      gpu->chan[3].loopy_dmx.blk |= 0xF0000000;
    break;
  case 0x03A:
    blk_done16_8_x32 (gpu->chan[3].ref_x, 1);
    gpu->chan[3].loopy_dmx.blk = gpu->chan[3].ref_x.blk;
    if (gpu->chan[3].loopy_dmx.blk & 0x8000000) 
      gpu->chan[3].loopy_dmx.blk |= 0xF0000000;
    break;
  case 0x03C: 
    blk_done16_8_x32 (gpu->chan[3].ref_y, 0);
    gpu->chan[3].loopy_dmy.blk = gpu->chan[3].ref_y.blk;
    if (gpu->chan[3].loopy_dmy.blk & 0x8000000) 
      gpu->chan[3].loopy_dmy.blk |= 0xF0000000;
    break;
  case 0x03E:
    blk_done16_8_x32 (gpu->chan[3].ref_y, 1);
    gpu->chan[3].loopy_dmy.blk = gpu->chan[3].ref_y.blk;
    if (gpu->chan[3].loopy_dmy.blk & 0x8000000) 
      gpu->chan[3].loopy_dmy.blk |= 0xF0000000;
    break;
  default: 
    DEBUG_BREAK ();
    break;
  }
}

finline 
void gpu_dispose_invblank (struct gpu *gpu) {
  /* deal srcoll, affine function */
  int id;
  for (id = 0; id != 4; id++) {
    struct gpu_channel *t = & gpu->chan[id];

    t->loopy_y_shadow.blk = t->loopy_y.blk;
    t->loopy_x_shadow.blk = t->loopy_x.blk;
    t->dx_shadow.sblk = t->dx.sblk;
    t->dy_shadow.sblk = t->dy.sblk;
    t->dmx_shadow.sblk = t->dmx.sblk;
    t->dmy_shadow.sblk = t->dmy.sblk;
    t->loopy_dmx.sblk = t->ref_x.sblk;
    t->loopy_dmy.sblk = t->ref_y.sblk;
  }
}

finline 
uint16_t swap_rb (uint16_t value) {
  return  value >> 10 & 31 
        |          value & 31 << 5 
        |          (value & 31) << 10;
}

finline 
uint32_t swap_rw (uint32_t value) {
  return  value >> 16 & 0xFF 
        |          value & 0xFF << 8
        |          (value & 0xFF) << 16;
}

finline 
uint32_t ext_rb (uint16_t value) {
  uint32_t a = value >> 0 & 0x1F;
  uint32_t b = value >> 5 & 0x1F;
  uint32_t c = value >>10 & 0x1F;
  a <<= 3;
  b <<= 3;
  c <<= 3;
  return a | b << 8 | c << 16;
}

finline 
void gpu_dispose_inhblank (struct gpu *gpu, kable update_affine) {
  /* -------------------------------------------------- */
  int id;
  for (id = 0; id != 4; id++) {
    struct gpu_channel *t = & gpu->chan[id];
    t->loopy_y_shadow.blk = t->loopy_y.blk;
    t->loopy_x_shadow.blk = t->loopy_x.blk;
    t->dx_shadow.sblk = t->dx.sblk;
    t->dy_shadow.sblk = t->dy.sblk;
    t->dmx_shadow.sblk = t->dmx.sblk;
    t->dmy_shadow.sblk = t->dmy.sblk;

    if (update_affine) {
      t->loopy_dmx.blk += t->dmx_shadow.blk;
      t->loopy_dmy.blk += t->dmy_shadow.blk;
    }
  }
}

finline 
void gpu_adjust_palette (struct gpu *gpu, uint16_t io_address) {
  int32_t id = io_address & 0x3FF;
  uint16_t *ptr16 = (uint16_t *) & gpu->palette[id & -2];
  * (uint16_t *)& gpu->palette2[id & -2] = swap_rb (* (uint16_t *)& gpu->palette[id & -2]);
  * (uint32_t *)& gpu->palette3[id & -2] = ext_rb (* (uint16_t *)& gpu->palette[id & -2]);
  * (uint32_t *)& gpu->palette4[id & -2] = swap_rw (* (uint32_t *)& gpu->palette3[id & -2]);
}
finline 
void gpu_adjust_palette16 (struct gpu *gpu, uint16_t io_address) {
  const uint32_t ioaddr = io_address & -2;
  gpu_adjust_palette (gpu, ioaddr);
  gpu_adjust_palette (gpu, ioaddr + 1);
}
finline 
void gpu_adjust_palette32 (struct gpu *gpu, uint16_t io_address) {
  const uint32_t ioaddr = io_address & -4;
  gpu_adjust_palette16 (gpu, ioaddr);
  gpu_adjust_palette16 (gpu, ioaddr + 2);
}
finline 
void gpu_adjust_palette64 (struct gpu *gpu, uint16_t io_address) {
  const uint32_t ioaddr = io_address & -8;
  gpu_adjust_palette32 (gpu, ioaddr);
  gpu_adjust_palette32 (gpu, ioaddr + 4);
}

finline 
void gpu_init (struct gpu *gpu) {
  /* -------------------------------------------------- */
  memset (gpu, 0, sizeof (struct gpu));

  gpu->vattr = (uint16_t *) (((uintptr_t)& gpu->vattrb[512]) & (uintptr_t) -128);
  gpu->vbuf = (uint16_t *) (((uintptr_t)& gpu->vbufb[512+1024*180]) & (uintptr_t) -128);
  gpu->vattr -= 8;
  gpu->vbuf -= 8;
  gpu->vptr_pitch = 2048;
  gpu->vptrcc = (uint16_t *) (((uintptr_t)& gpu->vptr_cahce[512]) & (uintptr_t) -128);
  gpu->vid_buf.pixel = (int16_t *) & gpu->vbuf[8];
  gpu->vid_buf.h = 160;
  gpu->vid_buf.w = 240;
  gpu->vid_buf.pitch = 2048;
  gpu->palette16_b = & gpu->palette2[0];
}

finline 
void gpu_reset (struct gpu *gpu) {
  gpu_init (gpu);
  /* -------------------------------------------------- */
  gpu->ctl.blk = 0;
  gpu->col_swap.blk = 0;
  gpu->status.blk = 0;
  gpu->line.blk = 0;
  gpu->clip_x[0].blk = 0;
  gpu->clip_y[0].blk = 0;
  gpu->clip_x[1].blk = 0;
  gpu->clip_y[1].blk = 0;
  gpu->win_in.blk = 0;
  gpu->win_out.blk = 0;
  gpu->mosaic.blk = 0;
  gpu->bld_ctl.blk = 0;
  gpu->bld_args.blk = 0;
  gpu->bri_args.blk = 0;
  gpu->ugpio_4E.blk = 0;
  gpu->ugpio_56.blk = 0;

  memset (gpu->oam, 0, sizeof (gpu->oam));
  memset (& gpu->chan[0], 0, sizeof (struct gpu_channel));
  memset (& gpu->chan[1], 0, sizeof (struct gpu_channel));
  memset (& gpu->chan[2], 0, sizeof (struct gpu_channel));
  memset (& gpu->chan[3], 0, sizeof (struct gpu_channel));
  memset (& gpu->chan[4], 0, sizeof (struct gpu_channel));

  gpu->vattr = (uint16_t *) (((uintptr_t)& gpu->vattrb[512]) & (uintptr_t) -128);
  gpu->vbuf = (uint16_t *) (((uintptr_t)& gpu->vbufb[512+1024*180]) & (uintptr_t) -128);
  gpu->vattr -= 8;
  gpu->vbuf -= 8;
  gpu->vptr_pitch = 2048;
  gpu->vptrcc = (uint16_t *) (((uintptr_t)& gpu->vptr_cahce[512]) & (uintptr_t) -128);
  gpu->vid_buf.pixel = (int16_t *) & gpu->vbuf[8];
  gpu->vid_buf.h = 160;
  gpu->vid_buf.w = 240;
  gpu->vid_buf.pitch = 2048;
}

#endif 