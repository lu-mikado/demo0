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
 
/* XXX: The implementation of DMA has problems in critical timing and  halt/sleep (...??) */
/* TODO: GamePak DMA */
/* TODO: Fifo DMA Init noseq load status */
#ifndef DMA_INL
#define DMA_INL 1

#define DMA0SRC_LO 0x0B0
#define DMA0SRC_HI 0x0B2 
#define DMA1SRC_LO 0x0BC
#define DMA1SRC_HI 0x0BE
#define DMA2SRC_LO 0x0C8
#define DMA2SRC_HI 0x0CA
#define DMA3SRC_LO 0x0D4
#define DMA3SRC_HI 0x0D6
#define DMA0DST_LO (DMA0SRC_LO+4)
#define DMA0DST_HI (DMA0SRC_HI+4)
#define DMA1DST_LO (DMA1SRC_LO+4)
#define DMA1DST_HI (DMA1SRC_HI+4)
#define DMA2DST_LO (DMA2SRC_LO+4)
#define DMA2DST_HI (DMA2SRC_HI+4)
#define DMA3DST_LO (DMA3SRC_LO+4)
#define DMA3DST_HI (DMA3SRC_HI+4)
#define DMA0_COUNTER 0xB8
#define DMA1_COUNTER 0xC4
#define DMA2_COUNTER 0xD0
#define DMA3_COUNTER 0xDC
#define DMA0_CONTROL (DMA0_COUNTER+2)
#define DMA1_CONTROL (DMA1_COUNTER+2)
#define DMA2_CONTROL (DMA2_COUNTER+2)
#define DMA3_CONTROL (DMA3_COUNTER+2)

finline 
void dma_startup_0 (struct dma0 *dma, struct apu *apu) {
  const uint32_t dsignal = dma->control.blk >> 5 & 3;
  const uint32_t ssignal = dma->control.blk >> 7 & 3;
  const uint32_t trans32 = dma->control.blk >> 10 & 1;
  const uint32_t counter = dma->counter.blk & dma->counter_max - 1;
  const uint32_t vector[4] = { 1, -1, 0, 1 };

  dma->fifo = null;
  dma->source_vec = vector[ssignal];
  dma->destin_vec = vector[dsignal];
  dma->source_internal.blk = dma->source.blk & dma->source_mask;
  dma->destin_internal.blk = dma->destin.blk & dma->destin_mask;

  if (trans32 != 0) {
    dma->source_vec <<= 2;
    dma->destin_vec <<= 2;
    dma->source_internal.blk &= -4;
    dma->destin_internal.blk &= -4;
  } else {
    dma->source_vec <<= 1;
    dma->destin_vec <<= 1;
    dma->source_internal.blk &= -2;
    dma->destin_internal.blk &= -2;
  }

  if (counter == 0)
    dma->counter_internal.blk = dma->counter_max;
  else 
    dma->counter_internal.blk = counter;
  
  if ((dma->chan == 1 || dma->chan == 2)
    && (dma->control.blk & 0xB000) == 0xB000)
  {
    const uint32_t addr = dma->destin.blk & 15;

    dma->source_vec = vector[ssignal] << 2;
    dma->source_internal.blk &= -4;

    if (addr < 4) {
      dma->fifo = & apu->fifo_ch[0];
      dma->destin_vec = 0;
    } else {
      dma->fifo = & apu->fifo_ch[1];
      dma->destin_vec = 0;
    }
  }
}

finline 
void dma_startup_1 (struct dma0 *dma, struct apu *apu) {
  const uint32_t dsignal = dma->control.blk >> 5 & 3;
  const uint32_t ssignal = dma->control.blk >> 7 & 3;
  const uint32_t trans32 = dma->control.blk >> 10 & 1;
  const uint32_t counter = dma->counter.blk & dma->counter_max - 1;
  const uint32_t vector[4] = { 1, -1, 0, 1 };

  dma->fifo = null;
  dma->source_vec = vector[ssignal];
  dma->destin_vec = vector[dsignal];

  if (dsignal == 3)
    dma->destin_internal.blk = dma->destin.blk & dma->destin_mask;

  if (trans32 != 0) {
    dma->source_vec <<= 2;
    dma->destin_vec <<= 2;
    dma->source_internal.blk &= -4;
    dma->destin_internal.blk &= -4;
  } else {
    dma->source_vec <<= 1;
    dma->destin_vec <<= 1;
    dma->source_internal.blk &= -2;
    dma->destin_internal.blk &= -2;
  }

  if (counter == 0)
    dma->counter_internal.blk = dma->counter_max;
  else 
    dma->counter_internal.blk = counter;
  
  if ((dma->chan == 1 || dma->chan == 2)
    && (dma->control.blk & 0xB000) == 0xB000)
  {
    const uint32_t addr = dma->destin.blk & 15;
    
    dma->source_vec = vector[ssignal] << 2;
    dma->source_internal.blk &= -4;

    if (addr < 4) {
      dma->fifo = & apu->fifo_ch[0];
      dma->destin_vec = 0;
    } else {
      dma->fifo = & apu->fifo_ch[1];
      dma->destin_vec = 0;
    }
  }
}

finline  
void dma_re_dispatch  (struct gba *const agb) {
  if (agb->dma.active == null) {
    int32_t id;
    for (id = 3; id >= 0; id--) {
      struct dma0 *const dma = & agb->dma.component[id];
      if (dma->ready != false && (dma->control.blk & 0x8000) != 0)
        agb->dma.active = dma;
    }
    if (agb->dma.active != null) {
      agb->dma.active->ready = false;
      dma_startup_1 (agb->dma.active, & agb->apu);
    }
  }
}

static 
void dma_writeIO  (struct gba *const agb, const int ch, uint16_t value) {

  struct dma0 *const dma_cur = & agb->dma.component[ch];

  if (value & 0x8000  && !(dma_cur->control.blk & 0x8000)) {
    /* DMA Open pulse */ 
    dma_cur->control.blk = value;
    dma_startup_0 (dma_cur, & agb->apu);
    
    /* TODO: more repeat stop DMA's details */
    if ((value & 0x3000) == 0) {
      /* Promptly DMA */
      dma_cur->ready = true;
      dma_re_dispatch (agb);
    }
  }
  if ((value & 0x8000) == 0) {
    dma_cur->ready = false;
  }
  dma_cur->control.blk = value;
}

finline 
void dma_reset (struct dma *dma) {
  memset (& dma->component[0], 0, sizeof (struct dma0));
  memset (& dma->component[1], 0, sizeof (struct dma0));
  memset (& dma->component[2], 0, sizeof (struct dma0));
  memset (& dma->component[3], 0, sizeof (struct dma0));

  dma->component[0].chan = 0;
  dma->component[0].source_mask = 0x7FFFFFF;
  dma->component[0].destin_mask = 0x7FFFFFF;
  dma->component[0].counter_max = 0x4000;

  dma->component[1].chan = 1;
  dma->component[1].source_mask = 0xFFFFFFF;
  dma->component[1].destin_mask = 0x7FFFFFF;
  dma->component[1].counter_max = 0x4000;

  dma->component[2].chan = 2;
  dma->component[2].source_mask = 0xFFFFFFF;
  dma->component[2].destin_mask = 0x7FFFFFF;
  dma->component[2].counter_max = 0x4000;

  dma->component[3].chan = 3;
  dma->component[3].source_mask = 0xFFFFFFF;
  dma->component[3].destin_mask = 0xFFFFFFF;
  dma->component[3].counter_max = 0x10000;

  dma->ugpio_E0.blk = 0;
}

finline 
void dma_hbl_stuff (struct  gba *agb) {
  if ((agb->dma.component[0].control.blk & 0xB000) == 0xA000) agb->dma.component[0].ready = true;
  if ((agb->dma.component[1].control.blk & 0xB000) == 0xA000) agb->dma.component[1].ready = true;
  if ((agb->dma.component[2].control.blk & 0xB000) == 0xA000) agb->dma.component[2].ready = true;
  if ((agb->dma.component[3].control.blk & 0xB000) == 0xA000) agb->dma.component[3].ready = true;

  dma_re_dispatch (agb);
}

finline 
void dma_vbl_stuff (struct  gba *agb) {
  if ((agb->dma.component[0].control.blk & 0xB000) == 0x9000) agb->dma.component[0].ready = true;
  if ((agb->dma.component[1].control.blk & 0xB000) == 0x9000) agb->dma.component[1].ready = true;
  if ((agb->dma.component[2].control.blk & 0xB000) == 0x9000) agb->dma.component[2].ready = true;
  if ((agb->dma.component[3].control.blk & 0xB000) == 0x9000) agb->dma.component[3].ready = true;

  dma_re_dispatch (agb);
}

#endif