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

#ifndef APU_INL
#define APU_INL 1

finline
void channel_fifo_reset (struct channel_fifo *fifo) {
  fifo->read =0;
  fifo->write = 0;
  fifo->count = 0;
  fifo->out =0;
  memset (& fifo->buf[0], 0, sizeof (fifo->buf));
}

finline
void channel_fifo_pop (struct channel_fifo *fifo) {
  if (fifo->count <= 0) {
    fifo->count =
    fifo->write =
    fifo->read = 
    fifo->out = 0;
  } else {
    fifo->out = fifo->buf[fifo->read];
    fifo->read  = fifo->read  + 1 & 31;
    fifo->count--;
  }
}

finline
void channel_fifo_push (struct channel_fifo *fifo, int8_t sample, uint32_t addr) {
  if (fifo->count >= 32) {
#if 1
    fifo->buf[fifo->write ^ 31] = sample; 
    fifo->write  = fifo->write  + 1 & 31;
    fifo->count = 32;
#endif 
  } else {
    fifo->buf[fifo->write] = sample;
    fifo->write  = fifo->write  + 1 & 31;
    fifo->count++;
  }
}

static 
void squ_reset_freq (struct channel_squ *squ, const uint32_t chan, const uint32_t freq) {
  
  const double squ_hzmod = (double) (2048 - (freq & 2047));
  const double squ_freq = 131072.0 / squ_hzmod;
  const double squ_fhz = AGB_CPU_FREQ_lf / squ_freq;

  //if (chan == 0)
  //  printf ("channel:%d RESET %04X\n", chan, freq);

  squ->signal_trigger = (int32_t) (squ_fhz / 8.0); 
}

finline 
void squ_render (struct apu *apu, int32_t delta) {

  struct channel_squ *const squ = & apu->squ_ch[0];

  if (squ->enable != false) {
    squ->signal_phase += delta;
    if (squ->signal_phase >= squ->signal_trigger) {
      squ->duty_poll ++;
      squ->signal_phase -= squ->signal_trigger;
    }
    if (squ->envlope != false) {
      squ->envlope_phase += delta;
      if (squ->envlope_phase >= squ->envlope_trigger) {
        if (squ->nrx1_x2.blk & 0x800) {
          squ->signal_volume ++;
          squ->envlope_phase -= squ->envlope_trigger;
          if (squ->signal_volume > 15) {
            squ->signal_volume = 15;
          }
        } else {
          squ->signal_volume --;
          squ->envlope_phase -= squ->envlope_trigger;
          if (squ->signal_volume < 0) {
            squ->signal_volume = 0;
          }
        }
      }
    }
    if (squ->loop == false) {
      if (squ->length_count > 0) {
        squ->length_phase += delta;
        if (squ->length_phase >= AGB_SQU_PLENGTH_TRIGGER) {
          squ->length_count --;
          squ->length_phase -= AGB_SQU_PLENGTH_TRIGGER;
          if (squ->length_count <= 0) {
            squ->length_count = 0;
            squ->signal_phase = 0;
            squ->envlope_phase = 0;
            squ->sweep_phase = 0;
            squ->enable = false;
            apu->settings.nr52.blk &= ~1;
          }
        }
      } 
    }
    if (squ->sweep != false) {
      squ->sweep_phase += delta;
      if (squ->sweep_phase >= squ->sweep_trigger) {
       int _freq_n = squ->nrx3_x4.blk & 0x7FF;
       int _freq_s = _freq_n;
       int _block_n = squ->sweep_phase/ squ->sweep_trigger;
       squ->sweep_phase -= squ->sweep_trigger;
       if (squ->nrx0.blk & 8)  
           _freq_n = (int) ((double) _freq_n - (double) _freq_n  / (double)(1 << (squ->nrx0.blk & 7)));
       else 
           _freq_n = (int) ((double) _freq_n + (double) _freq_n  / (double)(1 << (squ->nrx0.blk & 7)));
       if (_freq_n <= 0) 
         _freq_n = _freq_s;
       else if (_freq_n > 2047) {
         _freq_n = 2047; /* FIXME: right ??, i don't know */
         squ->sweep_phase = 0;
         squ->enable = false;
         apu->settings.nr52.blk &= ~1;
       }
       squ->nrx3_x4.blk =  squ->nrx3_x4.blk & ~0x7FF | _freq_n;
       squ_reset_freq (squ, 0, _freq_n);
      }
    }
  }
}

finline 
void squ2_render (struct apu *apu, int32_t delta) {

  struct channel_squ *const squ = & apu->squ_ch[1];

  if (squ->enable != false) {
    squ->signal_phase += delta;
    if (squ->signal_phase >= squ->signal_trigger) {
      squ->duty_poll ++;
      squ->signal_phase -= squ->signal_trigger;
    }
    if (squ->envlope != false) {
      squ->envlope_phase += delta;
      if (squ->envlope_phase >= squ->envlope_trigger) {
        if (squ->nrx1_x2.blk & 0x800) {
          squ->signal_volume ++;
          squ->envlope_phase -= squ->envlope_trigger;
          if (squ->signal_volume > 15) {
            squ->signal_volume = 15;
          }
        } else {
          squ->signal_volume --;
          squ->envlope_phase -= squ->envlope_trigger;
          if (squ->signal_volume < 0) {
            squ->signal_volume = 0;
          }
        }
      }
    }
    if (squ->loop == false) {
      if (squ->length_count > 0) {
        squ->length_phase += delta;
        if (squ->length_phase >= AGB_SQU_PLENGTH_TRIGGER) {
          squ->length_count --;
          squ->length_phase -= AGB_SQU_PLENGTH_TRIGGER;
          if (squ->length_count <= 0) {
            squ->length_count = 0;
            squ->signal_phase = 0;
            squ->envlope_phase = 0;
            squ->sweep_phase = 0;
            squ->enable = false;
            apu->settings.nr52.blk &= ~2;
          }
        }
      } 
    }
  }
}

finline 
void wave_render (struct apu *apu, int32_t delta) {
  if ((apu->settings.nr52.blk & 0x84) == 0x84
    && (apu->wave_ch.nr30.blk & 0x80) == 0x80) {
      struct channel_wave *wav = & apu->wave_ch;
      /* wave out */
      wav->output_phase += delta;
      if (wav->output_phase >= wav->output_trigger) {
        wav->counter += wav->output_phase / wav->output_trigger;
        wav->output_phase %= wav->output_trigger;
      }
      /* length count */
      wav->length_phase += delta;
      if (wav->length_phase > wav->length_trigger) {
        wav->length_count -= wav->length_phase/ wav->length_trigger;
        wav->length_phase %= wav->length_trigger;
        if (wav->length_count <= 0) {
          wav->length_count = 0;
          if (wav->nr33_34.blk & 0x4000)
            apu->settings.nr52.blk &= ~4;
        }
      }
  }
}

finline 
intptr_t lfsr15 (intptr_t seed) {
  /* step15 lfsr: 15, 14 (14,13)*/
  intptr_t d2 = (seed & 0x4000) >> 14;
  intptr_t d3 = (seed & 0x2000) >> 13;
  intptr_t out = 1 & (d2 ^ d3);
  seed <<= 1;
  seed |= out;
  seed &= 0x7FFF;
  return seed;
}
finline 
intptr_t lfsr7 (intptr_t seed) {
  /* step7 lfsr: 7, 6 (6,5)*/
  intptr_t d2 = (seed & 0x40) >> 6;
  intptr_t d3 = (seed & 0x20) >> 5;
  intptr_t out = 1 & (d2 ^ d3);
  seed <<= 1;
  seed |= out;
  seed &= 0x7F;
  return seed;
}
finline 
void noi_render (struct apu *apu, int32_t delta) {
  if ((apu->settings.nr52.blk & 0x88) == 0x88) {
    struct channel_noise *noi = & apu->noise_ch;
    /* noi signal */
    noi->signal_phase += delta;
    if (noi->signal_phase >= noi->signal_trigger) {
      int n_block = noi->signal_phase / noi->signal_trigger;
      noi->signal_phase %= noi->signal_trigger;
      while (n_block-- > 0) 
        noi->signal_output = (noi->nr43_44.blk & 8) 
            ? lfsr7 (noi->signal_output & 0x7F)
                 : lfsr15 (noi->signal_output & 0x7FFF);
    }
    /* noise envlope */
    if ((noi->nr41_42.blk >> 8) & 7) {
      noi->envlope_phase += delta;
      if (noi->envlope_phase >= noi->envlope_trigger) {
        noi->envlope_phase %= noi->envlope_trigger;
        if (noi->nr41_42.blk & 0x800) {
          noi->signal_volume += noi->envlope_phase / noi->envlope_trigger;
          if (noi->signal_volume > 15)
            noi->signal_volume = 15;
        } else {
          noi->signal_volume -= noi->envlope_phase / noi->envlope_trigger;
          if (noi->signal_volume < 0) {
#ifdef NR_NOI_TEST1
            apu->settings.nr52 &= ~(1 << (ch & 1));
#endif 
            noi->signal_volume = 0;
          }
        }
      }
    }
    /* length count */
    noi->length_phase += delta;
    if (noi->length_phase > noi->length_trigger) {
      noi->length_count -= noi->length_phase/ noi->length_trigger;
      noi->length_phase %= noi->length_trigger;
      if (noi->length_count <= 0) {
        noi->length_count = 0;
        if (noi->nr43_44.blk & 0x4000)
          apu->settings.nr52.blk &= ~8;
      }
    }
  }
}


finline 
uint16_t apu_read (struct apu *apu, uint32_t addr) {

  /* TODO: NR MASK */
  switch (addr & 0x3FF) {
  case 0x060: return apu->squ_ch[0].nrx0.blk;
  case 0x062: return apu->squ_ch[0].nrx1_x2.blk;
  case 0x064: return apu->squ_ch[0].nrx3_x4.blk;
  case 0x068: return apu->squ_ch[1].nrx1_x2.blk;
  case 0x06C: return apu->squ_ch[1].nrx3_x4.blk;
  case 0x070: return apu->wave_ch.nr30.blk;
  case 0x072: return apu->wave_ch.nr31_32.blk;
  case 0x074: return apu->wave_ch.nr33_34.blk;
  case 0x078: return apu->noise_ch.nr41_42.blk;
  case 0x07C: return apu->noise_ch.nr43_44.blk;
  case 0x080: return apu->settings.nr50_51.blk;
  case 0x082: return apu->settings.snd_ctl.blk;
  case 0x084: return apu->settings.nr52.blk;
  case 0x088: return apu->settings.pwm_ctl.blk;
  case 0x090: case 0x0092: case 0x094: case 0x096: 
  case 0x098: case 0x009A: case 0x09C: 
  case 0x09E: return apu->wave_ch.xram[((addr - 0x090 & 0x3FF) >> 1) + (apu->wave_ch.nr30.blk & 0x40) ? 0 : 8];
  default:
    DEBUG_BREAK ();
  }
  return 0;
}

static 
void squ_reset_duty (struct channel_squ *squ, size_t duty) {
  uint8_t duty125[8] = { 0, 0, 0, 1, 0, 0, 0, 0};
  uint8_t duty250[8] = { 0, 0, 0, 1, 1, 0, 0, 0};
  uint8_t duty500[8] = { 0, 0, 1, 1, 1, 1, 0, 0};
  uint8_t duty750[8] = { 0, 1, 1, 1, 1, 1, 1, 0};

  switch (duty & 3) {
  case 0: memcpy (& squ->duty[0], & duty125[0], sizeof (duty125)); break;
  case 1: memcpy (& squ->duty[0], & duty250[0], sizeof (duty125)); break;
  case 2: memcpy (& squ->duty[0], & duty500[0], sizeof (duty125)); break;
  case 3: memcpy (& squ->duty[0], & duty750[0], sizeof (duty125)); break;
  default:
    assert (0);
    break;
  }
}



finline 
void apu_write (struct apu *apu, uint32_t addr, uint16_t value) {

  const uint32_t entries = (addr & 0x3FF) >= NR_CH1_CH2_BOUNDARY ? 1 : 0;
  struct channel_squ *const squ = & apu->squ_ch[entries];

  switch (addr & 0x3FF) {
  case SG1_SWP: 
    squ->nrx0.blk = value;
    squ->sweep_trigger = AGB_SQU_PSWEEP_TRIGGER * (value >> 4 & 7);
    if (squ->sweep_trigger == 0)
      squ->sweep = false;
    else 
      squ->sweep = true;
    break;
  case SG1_ELDV: 
  case SG2_ELDV:
    squ->nrx1_x2.blk = value;
    squ->length_count = 64 - (value & 63);
    squ->envlope_trigger = AGB_SQU_PENVLOPE_TRIGGER * (value >> 8 & 7);
    squ->signal_volume = value >> 12 & 15;

    if (squ->envlope_trigger != 0)
      squ->envlope = true;
    else 
      squ->envlope = false;
    squ_reset_duty (squ, value >> 6 & 3);
    break;
  case SG1_FC:
  case SG2_FC:
    if (value & 0x8000) {
      apu->settings.nr52.blk |= 1 << entries;
        /*  Trigger Event
         *  Writing a value to NRx4 with bit 7 set causes the following things to occur:
         * 
         *  Channel is enabled (see length counter).
         *  If length counter is zero, it is set to 64
         *  Frequency timer is reloaded with period.
         *  Volume envelope timer is reloaded with period.
         *  Channel volume is reloaded from NRx2.
         *  Square 1's sweep does several things 
         *  Note that if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again.

        /*  During a trigger event, several things occur:

            Square 1's frequency is copied to the shadow register.
            The sweep timer is reloaded.
            The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
            If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately.
            
          TODO: Some features
        */
      squ->sweep = false;
      squ->loop = true;
      squ->enable = true;
      squ->envlope = false;
      
      squ->nrx3_x4.blk = value;

      squ->length_count = 64 - (squ->nrx1_x2.blk & 63);
      squ->signal_volume = squ->nrx1_x2.blk >> 12;
      squ->envlope_trigger = AGB_SQU_PENVLOPE_TRIGGER * (squ->nrx1_x2.blk >> 8 & 7);
      squ->sweep_phase = 0;
      squ->sweep_trigger = AGB_SQU_PSWEEP_TRIGGER * (squ->nrx0.blk >> 4 & 7);

      squ->length_phase = 0;
      squ->envlope_phase = 0;
      squ->signal_phase = 0;

      if (squ->envlope_trigger != 0)
        squ->envlope = true;
      if (squ->sweep_trigger != 0)
        squ->sweep = true;
      if (value & 0x4000)
        squ->loop = false;

      squ_reset_duty (squ, squ->nrx1_x2.blk >> 6 & 3);
    }
    // apu->squ_ch[entries].signal_trigger =  (int) (  16777216.0/ (4194304.0 / (32.0 * (double)(2048 - (value & 2047)) )))/8;
    squ->nrx3_x4.blk = value & ~0x8000;
    squ_reset_freq (squ, entries,  value);
    break;
  case 0x070:
    apu->wave_ch.nr30.blk = value;
    break;
  case 0x072:
    apu->wave_ch.length_count = 256 - (value & 255);
    apu->wave_ch.length_trigger = (int32_t) (16777216.0/256.0);
    apu->wave_ch.nr31_32.blk = value;
    break;
  case 0x074:
    if (value & 0x80) {
      apu->settings.nr52.blk |= 4;
      /*  Trigger Event
       *  Writing a value to NR34 with bit 7 set causes the following things to occur:
       * 
       *  Channel is enabled (see length counter).
       *  If length counter is zero, it is set to 256.
       *  Frequency timer is reloaded with period.
       *  Wave channel's position is set to 0 but sample buffer is NOT refilled.
       *  Note that if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again.
       *
       *  TODO: Some features
      */
      if (!apu->wave_ch.length_count) {
        apu->wave_ch.length_count = 256;
        if (apu->wave_ch.nr33_34.blk & 0x4000) {
          apu->wave_ch.output_phase = 0; 
        }
      }
      apu->wave_ch.length_trigger = 16780000/256;
    }
    apu->wave_ch.output_trigger = (int)((16780000.0/ (131072.0 / (double)(2048 - (value & 2047)))) / 
                                                        ((apu->wave_ch.nr30.blk & 0x20) ? 64.0 : 32.0));
    apu->wave_ch.nr33_34.blk = value;
    break;
  case 0x078:
    apu->settings.nr52.blk |= 8;
    apu->noise_ch.nr41_42.blk = value;
    apu->noise_ch.signal_volume = value >> 12;
    apu->noise_ch.length_count = 64 - (value & 63);
    apu->noise_ch.length_trigger = 16780000/256;
    apu->noise_ch.envlope_trigger = 16780000/64 * (value >> 8 & 7);
    break;
  case 0x07C: 
    if (value & 0x80) {
      apu->settings.nr52.blk |= 8;
      /*  Trigger Event
       *  Writing a value to NR44 with bit 7 set causes the following things to occur:
       * 
       *  Channel is enabled (see length counter).
       *  If length counter is zero, it is set to 64 
       *  Frequency timer is reloaded with period.
       *  Volume envelope timer is reloaded with period.
       *  Channel volume is reloaded from NR42.
       *  Noise channel's LFSR bits are all set to 1.
       *  Note that if the channel's DAC is off, after the above actions occur the channel will be immediately disabled again.
      */    
      if (!apu->noise_ch.length_count) {
        apu->noise_ch.length_count = 64;
        if (apu->noise_ch.nr43_44.blk & 0x4000) {
          apu->noise_ch.signal_phase = 0; 
          apu->noise_ch.envlope_phase = 0; 
        }
      }
      apu->noise_ch.signal_volume = value >> 12;
      apu->noise_ch.length_trigger = 16780000/256;
      apu->noise_ch.envlope_trigger = 16780000/64 * (value >> 8 & 7);
      apu->noise_ch.signal_output = -1;
    } else if ((value ^ apu->noise_ch.nr43_44.blk) & 8) {
      apu->noise_ch.signal_output = -1;
    }
    apu->noise_ch.signal_trigger = (int)(16780000.0/ (524288.0 / (double)(1 << (1 + (value >> 4 & 15))) 
                                                               / ((value & 7) ? (double) (value & 7) : 0.5  )));
    apu->noise_ch.nr43_44.blk = value;
    break;
  case 0x080: 
    apu->settings.nr50_51.blk = value;
    break;
  case 0x082: 
    if (value & 0x800) 
      channel_fifo_reset (& apu->fifo_ch[0]);
    if (value & 0x8000) 
      channel_fifo_reset (& apu->fifo_ch[1]);
    apu->settings.snd_ctl.blk = value;
    break;
  case 0x084: 
    apu->settings.nr52.blk = apu->settings.nr52.blk & 15 | ~(value & 15);
    break;
  case 0x088: 
    apu->settings.pwm_ctl.blk = value; /* TODO: pwm control */
    break;
  case 0x0A0: 
  case 0x0A2: /* FIFO- A Write */
    channel_fifo_push (& apu->fifo_ch[0], (int8_t) (value & 0xFF), addr);
    channel_fifo_push (& apu->fifo_ch[0], value >> 8, addr + 2);
    break;
  case 0x0A4: 
  case 0x0A6: /* FIFO- B Write */
    channel_fifo_push (& apu->fifo_ch[1], (int8_t) (value & 0xFF), addr);
    channel_fifo_push (& apu->fifo_ch[1], value >> 8, addr + 2);
    break;
  case 0x090: case 0x092: case 0x094: case 0x096:
  case 0x098: case 0x09A: case 0x09C: 
  case 0x09E: 
    if (apu->wave_ch.nr30.blk & 0x40) {
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+0] = value >> 4 & 15;
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+1] = value & 15;
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+2] = value >> 12 & 15; 
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+3] = value >> 8 & 15;
    } else {
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+ 32+0] = value >> 4 & 15;
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+ 32+1] = value & 15;
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+ 32+2] = value >> 12 & 15; 
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+ 32+3] = value >> 8 & 15;
    }
    apu->wave_ch.xram[((addr - 0x90) >> 1) + (apu->wave_ch.nr30.blk & 0x40) ? 0 : 8] = value;
    break;
  default:
    break;//DEBUG_BREAK ();
  }
}

finline 
void apu_write_b (struct apu *apu, uint32_t addr, uint8_t value) {

  uint16_t value_t;
  switch (addr & 0x3FE) {
  case 0x060: value_t = apu->squ_ch[0].nrx0.blk; break;
  case 0x062: value_t = apu->squ_ch[0].nrx1_x2.blk; break; 
  case 0x064: value_t = apu->squ_ch[0].nrx3_x4.blk; break;
  case 0x068: value_t = apu->squ_ch[1].nrx1_x2.blk; break; 
  case 0x06C: value_t = apu->squ_ch[1].nrx3_x4.blk; break;
  case 0x070: value_t = apu->wave_ch.nr30.blk; break;
  case 0x072: value_t = apu->wave_ch.nr31_32.blk; break;
  case 0x074: value_t = apu->wave_ch.nr33_34.blk; break;
  case 0x078: value_t = apu->noise_ch.nr41_42.blk; break;
  case 0x07C: value_t = apu->noise_ch.nr43_44.blk; break;
  case 0x080: value_t = apu->settings.nr50_51.blk; break;
  case 0x084: value_t = apu->settings.nr52.blk; break;
  case 0x088: value_t = apu->settings.pwm_ctl.blk; break;
  case 0x082: value_t = apu->settings.snd_ctl.blk; break;
  case 0x090: case 0x091: case 0x092: case 0x093:
    channel_fifo_push (& apu->fifo_ch[0], value, addr);
    return;
  case 0x094: case 0x095: case 0x096: case 0x097:
    channel_fifo_push (& apu->fifo_ch[1], value, addr);
    return;
  case 0x098: case 0x009A: case 0x09C: case 0x0A0: case 0x0A2: case 0xA4: case 0xA6:
  case 0x09E: 
  case 0x098+1: case 0x009A+1: case 0x09C+1: case 0x0A0+1: case 0x0A2+1: case 0xA4+1: case 0xA6+1:
  case 0x09E+1:
    if (apu->wave_ch.nr30.blk & 0x40) {
      /* Bank 0 write --------------------------- */
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+0] = value >> 4 & 15;
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+1] = value & 15;
    } else {
      /* Bank 1 write --------------------------- */
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+ 32+0] = value >> 4 & 15;
      apu->wave_ch.qram[((addr & 0xFF) - 0x98) * 2+ 32+1] = value & 15;
    }
    apu->wave_ch.xram[((addr - 0x90) >> 1) + (apu->wave_ch.nr30.blk & 0x40) ? 0 : 8] = value;
    return ;
  default:
    break;//DEBUG_BREAK ();
  }
  if (addr & 1)
    value_t = value_t & 0xFF | 256 * value;
  else 
    value_t = value_t & 0xFF00 |  value;
  apu_write (apu, addr & 0x3FE, value_t);
}

finline 
uint8_t apu_mixer_u8 (struct apu *apu) {
}
finline 
int8_t apu_mixer_s8 (struct apu *apu) {
}
finline 
uint16_t apu_mixer_u16 (struct apu *apu) {
}
finline int16_t
adds16 (int16_t u, int16_t v) {
  uint32_t out = (unsigned)u+(unsigned)v;
  if (out > INT16_MAX)
    out = INT16_MAX;
  return (int16_t)out;
}


finline 
void distribution_mixer_component16 (void);



static const 
int16_t step4Int16[16] = {
  0,
  INT16_MIN + 4681 * 0,
  INT16_MIN + 4681 * 1,
  INT16_MIN + 4681 * 2,
  INT16_MIN + 4681 * 3,
  INT16_MIN + 4681 * 4,
  INT16_MIN + 4681 * 5,
  INT16_MIN + 4681 * 6,
  INT16_MIN + 4681 * 7,
  INT16_MIN + 4681 * 8,
  INT16_MIN + 4681 * 9,
  INT16_MIN + 4681 *10,
  INT16_MIN + 4681 *11,
  INT16_MIN + 4681 *12,
  INT16_MIN + 4681 *13,
  INT16_MIN + 4681 *14
};

finline 
void apu_mixer_s16 (struct apu *apu, struct pcm_sample_16 *volume) {
  /* Synthesizer- Signed 16bit PCM 
    *             Square1 Square2 Wave Noise FIFO-2 
    * Amplifier       
    */
  if (!(apu->settings.nr52.blk & 0x80)) {
    /* sound close,  make mute */
    volume->bit16[0] =
    volume->bit16[1] = 0;
  } else {
    const int32_t cgb_master_volume__[4] = { 1, 2, 4, 0 /* [3] is UB */ }; /* 4000082's d1~d0, total := 4 */
    int16_t chan_out[6] = { 0, 0, 0, 0, 0, 0 };
    int32_t mixer_out[2] = { 0, 0 };
    kable chan_left[6] = { false, false, false, false,  false, false} ;
    kable chan_right[6] = { false, false, false, false,  false, false} ;
    const int32_t cgb_master_volume = cgb_master_volume__[apu->settings.snd_ctl.blk & 3];
    double chan_left_total = 0.0;
    double chan_right_total = 0.0;
#ifdef APU_NR50_51_OUTPUT_LEVEL0_IS_MUTE
    double chan_cgb_left_volume_rt = (apu->settings.nr50_51.blk >> 4 & 7
#else 
    const int32_t chan_cgb_left_b = (apu->settings.nr50_51.blk >> 4 & 7) + 1;
    const int32_t chan_cgb_right_b = (apu->settings.nr50_51.blk >> 0 & 7) + 1;
    const double chan_cgb_left_volume_rt =  ((double) (chan_cgb_left_b * cgb_master_volume)) / 32.0;
    const double chan_cgb_right_volume_rt =  ((double) (chan_cgb_right_b * cgb_master_volume)) / 32.0;
#endif 
    const double chan_fifo_a_volume_rt = (apu->settings.snd_ctl.blk & 4) ? 1.0 : 0.5;
    const double chan_fifo_b_volume_rt = (apu->settings.snd_ctl.blk & 8) ? 1.0 : 0.5;

    struct channel_squ *squ = & apu->squ_ch[0];
    struct channel_squ *squ2 = & apu->squ_ch[1];
    struct channel_wave *wav = & apu->wave_ch;
    struct channel_noise *noi = & apu->noise_ch;
    struct channel_fifo *fifo_a = & apu->fifo_ch[0];
    struct channel_fifo *fifo_b = & apu->fifo_ch[1];

    /* GBC Sound 4bit -> 16bit 
        FIFO Sound 8Bit -> 16Bit */
    /* total := 32 */
    /* squ 1 := 4
    /* squ 2 := 4
    /* wav4 := 4 
    /* noi := 4
    /* fifo-a := 8
    /* fifo-b := 8 */

    chan_out[0] = squ->duty[squ->duty_poll & 7] ? squ->signal_volume : 0;
    chan_out[1] = squ2->duty[squ2->duty_poll & 7] ? squ2->signal_volume : 0;
    /* TODO: 4Bit PCM */
    chan_out[2] = 0;
    chan_out[3] = noi->signal_output ? noi->signal_volume : 0;

    chan_out[4] = apu->fifo_ch[0].out * 256;
    chan_out[5] = apu->fifo_ch[1].out * 256;
    chan_out[0] = step4Int16[chan_out[0]];
    chan_out[1] = step4Int16[chan_out[1]];
    chan_out[2] = step4Int16[chan_out[2]];
    chan_out[3] = step4Int16[chan_out[3]];

    //chan_out[4]  = 0;
    //chan_out[5] =  0;


#undef APPEND_ENABLE_CHAN_INFOS
#define APPEND_ENABLE_CHAN_INFOS(reg_body, bit_start, ch_en, ch_total, ratio)\
    if ((reg_body) >> (bit_start) & 1) { \
      (ch_en) = true; \
      (ch_total) += (ratio);\
    } 

#define SQU1_COMPONENT 4.0
#define SQU2_COMPONENT 4.0
#define PCM4_COMPONENT 4.0
#define NOISE_COMPONENT 4.0
#define FIFOA_COMPONENT 64.0
#define FIFOB_COMPONENT 64.0

    /* check channel left output status */
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 12, chan_left[0], chan_left_total, SQU1_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 13, chan_left[1], chan_left_total, SQU2_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 14, chan_left[2], chan_left_total, PCM4_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 15, chan_left[3], chan_left_total, NOISE_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.snd_ctl.blk,  9, chan_left[4], chan_left_total, FIFOA_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.snd_ctl.blk, 13, chan_left[5], chan_left_total, FIFOB_COMPONENT);
     /* check channel right output status */
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 8, chan_right[0], chan_right_total, SQU1_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 9, chan_right[1], chan_right_total, SQU2_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 10, chan_right[2], chan_right_total, PCM4_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.nr50_51.blk, 11, chan_right[3], chan_right_total, NOISE_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.snd_ctl.blk,  8, chan_right[4], chan_right_total, FIFOA_COMPONENT);
    APPEND_ENABLE_CHAN_INFOS (apu->settings.snd_ctl.blk, 12, chan_right[5], chan_right_total, FIFOB_COMPONENT);

#undef MIXER_MAKE
#define MIXER_MAKE(enId, mixerId, chanId, mixer_rt, mixer_total, vol_rt)\
   if (enId[chanId] != false)\
    mixer_out[mixerId] += (int16_t) ((((double) chan_out[chanId]) * mixer_rt) / mixer_total * vol_rt)
#undef MIXER_SATURAT_OUTPUT
#define MIXER_SATURAT_OUTPUT(mixerId)\
    if (mixer_out[mixerId] > INT16_MAX)\
      volume->sbit16[mixerId] = INT16_MAX;\
    else if (mixer_out[mixerId] < INT16_MIN)\
      volume->sbit16[mixerId] = INT16_MIN;\
    else \
      volume->sbit16[mixerId] = mixer_out[mixerId]

    /* mixer left channel */
    MIXER_MAKE (chan_left, 0, 0, SQU1_COMPONENT, chan_left_total, chan_cgb_left_volume_rt);
    MIXER_MAKE (chan_left, 0, 1, SQU2_COMPONENT, chan_left_total, chan_cgb_left_volume_rt);
    MIXER_MAKE (chan_left, 0, 2, PCM4_COMPONENT, chan_left_total, chan_cgb_left_volume_rt);
    MIXER_MAKE (chan_left, 0, 3, NOISE_COMPONENT, chan_left_total, chan_cgb_left_volume_rt);
    MIXER_MAKE (chan_left, 0, 4, FIFOA_COMPONENT, chan_left_total, chan_fifo_a_volume_rt);
    MIXER_MAKE (chan_left, 0, 5, FIFOB_COMPONENT, chan_left_total, chan_fifo_b_volume_rt);
    MIXER_SATURAT_OUTPUT (0);

    /* mixer right channel */
    MIXER_MAKE (chan_right, 1, 0, SQU1_COMPONENT, chan_right_total, chan_cgb_right_volume_rt);
    MIXER_MAKE (chan_right, 1, 1, SQU2_COMPONENT, chan_right_total, chan_cgb_right_volume_rt);
    MIXER_MAKE (chan_right, 1, 2, PCM4_COMPONENT, chan_right_total, chan_cgb_right_volume_rt);
    MIXER_MAKE (chan_right, 1, 3, NOISE_COMPONENT, chan_right_total, chan_cgb_right_volume_rt);
    MIXER_MAKE (chan_right, 1, 4, FIFOA_COMPONENT, chan_right_total, chan_fifo_a_volume_rt);
    MIXER_MAKE (chan_right, 1, 5, FIFOB_COMPONENT, chan_right_total, chan_fifo_b_volume_rt);
    MIXER_SATURAT_OUTPUT (1);
  }
}

finline 
float apu_mixer_float (struct apu *apu) {
}

finline 
void apu_reset (struct apu *apu) {

  apu->squ_ch[0].signal_trigger =  (int) (  2097500.0  / (131072.0 / (double)(2048 - (0 & 2047))) );
  apu->squ_ch[1].signal_trigger =  (int) (  2097500.0  / (131072.0 / (double)(2048 - (0 & 2047))) );

  apu->wave_ch.output_trigger = (int)((16780000.0/ (131072.0 / (double)(2048 - (0 & 2047)))) / 
                                                  ((apu->wave_ch.nr30.blk & 0x20) ? 64.0 : 32.0));
  apu->wave_ch.length_trigger = 16780000/256;
  apu->noise_ch.signal_trigger = (int)(16780000.0/ (524288.0 / (double)(1 << (1 + (0 >> 4 & 15))) 
                                                          / ((0 & 7) ? (double) (0 & 7) : 0.5  )));
  apu->noise_ch.length_trigger = 16780000/256;
}

finline
void apu_tick (struct apu *apu, int32_t delta) {
 
  if (apu->settings.nr52.blk & 0x80) {
    squ_render (apu, delta);
    squ2_render (apu, delta);
    //wave_render (apu, delta);
    //noi_render (apu, delta);
  }
  /* Syth timing. XXX: Output may be lost, waveform distortion problem*/
  if (apu->sndbuf.sample_request == APU_SAMPLE_TYPE_SINT16) {
    apu->sam_phase += delta;
    if (apu->sam_phase >= apu->sam_trigger) {
      struct pcm_sample_16 sample;
      apu_mixer_s16 (apu, & sample);
      apu->sam_phase -= apu->sam_trigger;
      if (apu->sam_start <apu->sndbuf.blk_nums) {
        int16_t *pmix_snd = (int16_t *) & apu->sndbuf.buf[4 * apu->sam_start];
        pmix_snd [0] = sample.sbit16[0];
        pmix_snd [1] = sample.sbit16[1];
        apu->sam_start++;
      }
    }
  } else    {
    assert (0);
    /* TODO:*/
  }
}

finline
void apu_buffer_reset (struct apu *apu) {

  switch (apu->sndbuf.sample_request) {
  case APU_SAMPLE_TYPE_UINT8:
  case APU_SAMPLE_TYPE_SINT8:
    {
      int16_t *pmix_snd = (int16_t *) & apu->sndbuf.buf[0];
      int16_t tofill;

      if (apu->sam_start != 0)
        tofill = pmix_snd[apu->sam_start - 1];
      else 
        tofill = 0;
      
      while (apu->sam_start <apu->sndbuf.blk_nums) {
        pmix_snd[apu->sam_start++] = tofill;
      }
    }
    break;
  case APU_SAMPLE_TYPE_FLOAT:
    {
      int64_t *pmix_snd = (int64_t *) & apu->sndbuf.buf[0];
      int64_t tofill;

      if (apu->sam_start != 0)
        tofill = pmix_snd[apu->sam_start - 1];
      else 
        tofill = 0;
      
      while (apu->sam_start <apu->sndbuf.blk_nums) {
        pmix_snd[apu->sam_start++] = tofill;
      }
    }
    break;
  case APU_SAMPLE_TYPE_UINT16:
  case APU_SAMPLE_TYPE_SINT16:
    {
      int32_t *pmix_snd = (int32_t *) & apu->sndbuf.buf[0];
      int32_t tofill;

      if (apu->sam_start != 0)
        tofill = pmix_snd[apu->sam_start - 1];
      else 
        tofill = 0;
      
      while (apu->sam_start <apu->sndbuf.blk_nums) {
        pmix_snd[apu->sam_start++] = tofill;
      }
    }
    break;
  }
  apu->sam_phase = 0;
  apu->sam_start = 0;
}

#endif 