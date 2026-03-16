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

#ifndef GBA_H
#define GBA_H 1

#ifdef __cplusplus
extern "C" {
#endif

  /*
#define GBA_PROTOTYPE_INLINE
#define GBA_PROTOTYPE_EXPORT
#define GBA_PROTOTYPE_IMPORT
  */
#if !defined (GBA_PROTOTYPE_EXPORT) && !defined (GBA_PROTOTYPE_IMPORT)
# ifndef GBA_PROTOTYPE_INLINE
#  define GBA_PROTOTYPE_INLINE
# endif
#endif

#ifdef _WIN32
# define _CRT_SECURE_NO_DEPRECATE
#endif

#ifdef _WIN32
# include <Windows.h>
# include <Shlwapi.h>
#endif

#include "stdint.h"
#include "list.inl"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define AGB_CPU_FREQ  16777216
#define AGB_CPU_FREQ_f  16777216.0
#define AGB_CPU_FREQ_lf  16777216.0
#define AGB_SQU_PLENGTH_TRIGGER  65536
#define AGB_SQU_PLENGTH_TRIGGER_f  65536.0
#define AGB_SQU_PLENGTH_TRIGGER_lf  65536.0
#define AGB_SQU_PENVLOPE_TRIGGER  262144
#define AGB_SQU_PENVLOPE_TRIGGER_f  262144.0
#define AGB_SQU_PENVLOPE_TRIGGER_lf  262144.0
#define AGB_SQU_PSWEEP_TRIGGER  131072
#define AGB_SQU_PSWEEP_TRIGGER_f  131072.0
#define AGB_SQU_PSWEEP_TRIGGER_lf  131072.0
#define AGB_LCD_REFRESH_FREQ  59.727

#define BDIFFUSE(value, bitStart, bitMask)\
  ((value) >> (bitStart) & (bitMask))

/* XXX: This union requires memory continuity */
union iopad_blk8 {
  uint8_t blk;
  int8_t sblk;
};
/* XXX: This union requires memory continuity */
union iopad_blk16 {
  uint8_t blk8[2];
  int8_t sblk8[2];
  uint16_t blk;
  int16_t sblk;
};
/* XXX: This union requires memory continuity */
union iopad_blk32 {
  uint8_t blk8[4];
  int8_t sblk8[4];
  uint16_t blk16[2];
  int16_t sblk16[2];
  uint32_t blk;
  int32_t sblk;
};
/* XXX: This union requires memory continuity */
union iopad_blk64 {
  uint8_t blk8[8];
  int8_t sblk8[8];
  uint16_t blk16[4];
  int16_t sblk16[4];
  uint32_t blk32[2];
  int32_t sblk32[2];
  uint64_t blk;
  int64_t sblk;
};
struct header {
  uint32_t entry; /* ROM entry */
  uint8_t nintendo_logo[156];
  uint8_t title[12];
  uint8_t code[4];
  uint8_t marker[2];
  uint8_t fixed; /* must be 0x96 in GBA ROM Header */
  uint8_t main_unit;
  uint8_t device;
  uint8_t reserved_area[7];
  uint8_t soft_ver;
  uint8_t crc_code;
  uint8_t reserved_area2[2];
  /* ------------- Multi boot mode ---------------------------*/
  uint32_t entry2; /* RAM entry */
  uint8_t boot;
  uint8_t slave;
  uint8_t reserved_area3[26];
  uint32_t joybus;
};

#define fto_uint8p(x)((uint8_t *)(&(x)))
#define fto_uint16p(x)((uint16_t *)(&(x)))
#define fto_uint32p(x)((uint32_t *)(&(x)))
#define fto_uint8pv(x)(*(uint8_t *)(&(x)))
#define fto_uint16pv(x)(*(uint16_t *)(&(x)))
#define fto_uint32pv(x)(*(uint32_t *)(&(x)))
#define fto_uint64p(x)((uint64_t *)(&(x)))
#define ref_to_uint8p(x)((uint8_t *)((x)))
#define ref_to_uint16p(x)((uint16_t *)((x)))
#define ref_to_uint32p(x)((uint32_t *)((x)))
#define ref_to_uint64p(x)((uint64_t *)((x)))

#ifndef NULL_DEFINED
# ifdef __cplusplus
#  ifdef __BORLANDC__
#   define null 0
#  else
#   define null nullptr
#  endif
# else
#  define null ((void *)0)
# endif
#endif

typedef uintptr_t kable;
#if !defined (BOOL_DEFINED) \
 && !defined (__cplusplus)
  # define true 1
  # define false 0
#endif

#if defined (_MSC_VER) || defined (__ICC) || defined (__INTEL_COMPILER) || defined (__BORLANDC__)
# define finline static  __forceinline
# define dinline         static __declspec(noinline)
# define callstd         __stdcall
# define callc           __cdecl
# define calign(x)      __declspec(align(x))
# if defined(GBA_PROTOTYPE_EXPORT)
#  define gba_impl          __declspec(dllexport)
# elif  defined(GBA_PROTOTYPE_INLINE)
#  define gba_impl
# elif  defined(GBA_PROTOTYPE_IMPORT)
#  define gba_impl          __declspec(dllimport)
# else
#  error "Not specified GBA Prototype"
#endif
#elif defined (__GNUC__) || defined (__GNUG__)
# define finline static  __attribute__((always_inline))
# define dinline         __attribute__((noinline))
# define callstd         __attribute__((stdcall))
# define callc           __attribute__((cdecl))
# define calign(x)      __attribute__((aligned(x)))
# if  defined(GBA_PROTOTYPE_EXPORT)
#  define gba_impl          __attribute__((dllexport))
# elif  defined(GBA_PROTOTYPE_INLINE)
#  define gba_impl
# elif  defined(GBA_PROTOTYPE_IMPORT)
#  define gba_impl          __attribute__((dllimport))
# else
#  error "Not specified GBA Prototype"
#endif
#else
# error unsupported compiler!
#endif

#ifdef _WIN32
# ifdef _MSC_VER
#  ifdef _DEBUG
#    define DEBUG_BREAK() __debugbreak()
#  else
#    define DEBUG_BREAK() ((void)0)
#  endif
# else
#  define DEBUG_BREAK() ((void)0)
# endif
#endif

#ifdef _WIN32
# ifdef _MSC_VER
#  define FORCE_BREAK() do { __asm { int 3 } } while (0)
# else
#  define FORCE_BREAK() ((void)0)
# endif
#endif

#ifdef _DEBUG0
# define DEBUG_OUT(...) printf (__VA_ARGS__)
#else
# define DEBUG_OUT(...) ((void)0)
#endif

#define APU_SAMPLE_TYPE_SINT8 0
#define APU_SAMPLE_TYPE_UINT8 1
#define APU_SAMPLE_TYPE_SINT16 2
#define APU_SAMPLE_TYPE_UINT16 3
#define APU_SAMPLE_TYPE_FLOAT 4
#define APU_SAMPLE_TYPE_DEFAULT APU_SAMPLE_TYPE_SINT16
typedef intptr_t APU_SAMPLE_TYPE;

#define GPU_COL32_X8B8G8R8 0
#define GPU_COL32_X8R8G8B8 1
#define GPU_COL16_X1B5G5R5 2
#define GPU_COL16_X1R5G5B5 3
#define GPU_COL_DEFAULT GPU_COL16_X1R5G5B5
typedef intptr_t GPU_COL_MOLD;

/* for gba::wait */
#define WAIT_204_INSTRUCTION_CACHE_OPEN_MASK 0x4000

/* for gpu::status */
#define GPU_STAT_VBL 0x01
#define GPU_STAT_HBL 0x02
#define GPU_STAT_LYCS 0x04
#define GPU_STAT_VBL_IRQ 0x08
#define GPU_STAT_HBL_IRQ 0x10
#define GPU_STAT_LYC_IRQ 0x20

typedef int DMA_REQUEST;
#define DMA_REQUEST_NORMAL 0
#define DMA_REQUEST_VBL 1
#define DMA_REQUEST_HBL 2
#define DMA_REQUEST_SEPC_FIFO 3
#define DMA_REQUSET_SPEC_GPU 4
#define DMA_REQUSET_SPEC_GAMEPAK 5

typedef int IO_WRITE_BLOCK;
#define IO_WRITE_8 0
#define IO_WRITE_16 1
#define IO_WRITE_32 2

typedef uint32_t IRQ_REQUEST_MASK;
#define IRQ_REQUEST_VBL (1 << 0)
#define IRQ_REQUEST_HBL (1 << 1)
#define IRQ_REQUEST_LY (1 << 2)
#define IRQ_REQUEST_TIMER0 (1 << 3)
#define IRQ_REQUEST_TIMER1 (1 << 4)
#define IRQ_REQUEST_TIMER2 (1 << 5)
#define IRQ_REQUEST_TIMER3 (1 << 6)
#define IRQ_REQUEST_TIMER_START IRQ_REQUEST_TIMER0
#define IRQ_REQUEST_COMM (1 << 7)
#define IRQ_REQUEST_DMA0 (1 << 8)
#define IRQ_REQUEST_DMA1 (1 << 9)
#define IRQ_REQUEST_DMA2 (1 << 10)
#define IRQ_REQUEST_DMA3 (1 << 11)
#define IRQ_REQUEST_DMA_START IRQ_REQUEST_DMA0
#define IRQ_REQUEST_KEYPAD (1 << 12)
#define IRQ_REQUEST_GAMEPAK (1 << 13)

struct context;

int callc context_init (struct context **ctx);
void callc context_get (struct context *ctx);
void callc context_set (struct context *ctx);
void callc context_swap (struct context *ctx);
void callc context_swap2 (struct context *to, struct context *recv);
void callc context_copy (struct context *to, struct context *from);
void callc context_setpc (struct context *ctx, void *program_pointer);
void callc context_setstack (struct context *ctx, uintptr_t stack);
void *callc context_getpc (struct context *ctx);
uintptr_t callc context_getstack (struct context *ctx);
void callc context_uninit (struct context **ctx);

struct coroutine;

#define COROUTINE_EXEC_INIT 0
#define COROUTINE_EXEC_CURRENT 1
#define COROUTINE_EXEC_EXITUS 2
#define COROUTINE_EXEC_LINK 3
#define COROUTINE_EXEC_CACHE 4
#define COROUTINE_EXEC_USER 5
typedef int COROUTINE_EXEC_OBJ;

int coroutine_ctor (struct coroutine **co, int stack_size);
void coroutine_dtor (struct coroutine *co);
struct context *coroutine_getctx (struct coroutine *co, COROUTINE_EXEC_OBJ e_obj);
void coroutine_setctx (struct coroutine *co, COROUTINE_EXEC_OBJ e_obj, struct context * ctx);
void coroutine_swito (struct coroutine *co, kable interrupt, COROUTINE_EXEC_OBJ e_obj, struct context *ctx);
void coroutine_setcall (struct coroutine *co, int (*startup)(void *param), void *param);
void coroutine_run (struct coroutine *co);

struct gba_pad {
  uint16_t a:1;
  uint16_t b:1;
  uint16_t select:1;
  uint16_t start:1;
  uint16_t right:1;
  uint16_t left:1;
  uint16_t up:1;
  uint16_t down:1;
  uint16_t r:1;
  uint16_t l:1;
};
struct controller {
  union iopad_blk16 input;
  union iopad_blk16 ctl;
  uint32_t phase;
  uint32_t trigger;
  struct gba *agb;
};

#define ARM7_MODE_USER 0
#define ARM7_MODE_FIQ 1
#define ARM7_MODE_IRQ 2
#define ARM7_MODE_MGR 3
#define ARM7_MODE_ABT 7
#define ARM7_MODE_UNDEF 11
#define ARM7_MODE_SYS 15

#define R812b_BLOCK 5
#define R812b_EXCEPT_FIQ (R812b_BLOCK*0)
#define R812b_FIQ (R812b_BLOCK*1)

#define R1314b_BLOCK 2
#define R1314b_SYSUSER (R1314b_BLOCK*0)
#define R1314b_MGR (R1314b_BLOCK*1)
#define R1314b_IRQ (R1314b_BLOCK*2)
#define R1314b_FIQ (R1314b_BLOCK*3)
#define R1314b_ABT (R1314b_BLOCK*4)
#define R1314b_UDEF (R1314b_BLOCK*5)

#define SPSRb_BLOCK 1
#define SPSRb_SYSUSER (SPSRb_BLOCK*0)
#define SPSRb_MGR (SPSRb_BLOCK*1)
#define SPSRb_IRQ (SPSRb_BLOCK*2)
#define SPSRb_FIQ (SPSRb_BLOCK*3)
#define SPSRb_ABT (SPSRb_BLOCK*4)
#define SPSRb_UDEF (SPSRb_BLOCK*5)

#define AGB_IO_BASE 0x4000000
#define AGB_MAKEIO_OFF(x) ((x))

union field_f {
  struct flags_ {
    uint8_t v;
    uint8_t c;
    uint8_t z;
    uint8_t n;
  } sig;

  uint8_t blk8[4];
  uint16_t blk16[2];
#define nz blk16[1]
  uint32_t blk;
};

struct psr {
  union {
    struct {
      uint32_t mode:5;
      uint32_t thumb:1;
      uint32_t fiq:1;
      uint32_t irq:1;
      uint32_t dummy:20;
      uint32_t v:1;
      uint32_t c:1;
      uint32_t z:1;
      uint32_t n:1;
    };
    uint32_t blk;
    uint16_t blk16[2];
    uint8_t blk8[4];
  };
};
struct cpsr_f {
  union field_f  flags;
  kable thumb;
  kable irq_disable;
  kable fiq_disable;
  uint32_t mode;
};

#define LSHIFT(rn_mask, shift)\
 ((rn_mask) <<(shift))
#define RSHIFT(rn_mask, shift)\
 ((rn_mask) >>(shift))
#define THUMB_CODE_MASK 0xffffffe
#define ARM7_CODE_MASK 0xffffffc
#define ARM7_MEM_MASK 0xffffffc
#define ARM7_MAX_BIT_PEVALUE 0x80000000
#define ARM7_NEG_BIT 0x80000000
#define ARM7_UWORD_MAX 0xffffffff
#define ARM7_FLAG_N 0x80000000
#define ARM7_FLAG_Z 0x40000000
#define ARM7_FLAG_C 0x20000000
#define ARM7_FLAG_V 0x10000000
#define ARM7_MODE_HSET_MASK 0x10
#define ARM7_OPCODE_THUMB_MASK 0x20
#define ARM7_FIQ_DIS_MASK 0x40
#define ARM7_IRQ_DIS_MASK 0x80

struct arm7 {
  union iopad_blk32 regs[16];   /* Register r0-r12, SP, LR LINK, PC */
  union iopad_blk32 r812_t[10];/* Register Bank Cache r8-12 for FIQ mode*/
  union iopad_blk32 r1314_t[12]; /* Register Bank Cache r13-r14*/
  union iopad_blk32 int_magic; /* Block 0 := ! IE
                                  Block 1 := ! IF
                                  Block 2 := ! IME
                                  Block 3 := !! arm7's irq disable bit */
  struct cpsr_f cpsr_fast;
  struct psr cpsr;
  struct psr spsr_t[6]; /* Register SPSR Bank*/
  uint32_t opcode[2]; /*  Pipeline Opcode */
  union iopad_blk32 ime;
  union iopad_blk16 ie;
  union iopad_blk16 ifs;
  uint32_t nextNoSeqFetch;
  struct gba *agb;
  uint32_t unused;
};
struct timer0 {
  union iopad_blk16  counter;
  union iopad_blk16  reload;
  union iopad_blk16  control;
  uint32_t reload_adjust;
  uint32_t trigger;
  uint32_t phase;
  uint32_t overflow;
};
struct timer {
  struct timer0 component[4];
  union {
    int8_t item[4]; /* 0:<- no signal 1:<-std 2: cascade*/
    int32_t blk;
  } igniter;
};

struct dma0 {
  union iopad_blk32 counter_internal;
  union iopad_blk32 source_internal;
  union iopad_blk32 destin_internal;
  union iopad_blk16 counter;
  union iopad_blk32 source;
  union iopad_blk32 destin;
  union iopad_blk16 control;

  uint32_t source_mask;
  uint32_t destin_mask;
  uint32_t counter_max;
  uint32_t source_vec;
  uint32_t destin_vec;
  kable ready;
  int32_t chan;

  struct channel_fifo *fifo;
};
struct dma {
  struct dma0 component[4];
  struct dma0 *active;
  union iopad_blk16 ugpio_E0;
};

#define SG1_SWP AGB_MAKEIO_OFF(0x60)
#define SG1_ELDV AGB_MAKEIO_OFF(0x62) /* envlope, length, duty, volume */
#define SG1_FC AGB_MAKEIO_OFF(0x64) /* freq, ctl */

#define SG2_ELDV AGB_MAKEIO_OFF(0x68)
#define SG2_FC AGB_MAKEIO_OFF(0x6C)

#define SG3_DBS AGB_MAKEIO_OFF(0x70) /* dimension, bank, switch */
#define SG3_LVF AGB_MAKEIO_OFF(0x72)/* length, volume, force */
#define SG3_FC AGB_MAKEIO_OFF(0x74) /* freq, ctl */

#define SG4_ELV AGB_MAKEIO_OFF(0x78) /* envlope, length, volume */
#define SG4_FSC AGB_MAKEIO_OFF(0x7C) /* freq, lfsr-step, ctl */

#define SG_CGB_MIXCTL  AGB_MAKEIO_OFF(0x80)
#define SG_FIFO_MIXCTL  AGB_MAKEIO_OFF(0x82)
#define SG_STATUS_MASTER_ENABLE  AGB_MAKEIO_OFF(0x84)

#define SG_BIAS  AGB_MAKEIO_OFF(0x88)

#define NR10   AGB_MAKEIO_OFF(0x60)
#define NR11  AGB_MAKEIO_OFF(0x62)
#define NR12  AGB_MAKEIO_OFF(0x63)
#define NR13  AGB_MAKEIO_OFF(0x64)
#define NR14   AGB_MAKEIO_OFF(0x65)

#define NR21  AGB_MAKEIO_OFF(0x68)
#define NR22  AGB_MAKEIO_OFF(0x69)
#define NR23  AGB_MAKEIO_OFF(0x6C)
#define NR24   AGB_MAKEIO_OFF(0x6B)

#define NR30   AGB_MAKEIO_OFF(0x70)
#define NR31  AGB_MAKEIO_OFF(0x72)
#define NR32  AGB_MAKEIO_OFF(0x73)
#define NR33  AGB_MAKEIO_OFF(0x74)
#define NR34   AGB_MAKEIO_OFF(0x75)

#define NR41  AGB_MAKEIO_OFF(0x78)
#define NR42  AGB_MAKEIO_OFF(0x79)
#define NR43  AGB_MAKEIO_OFF(0x7C)
#define NR44   AGB_MAKEIO_OFF(0x7B)

#define NR50  AGB_MAKEIO_OFF(0x80)
#define NR51   AGB_MAKEIO_OFF(0x81)
#define NRMIX_EXT_L AGB_MAKEIO_OFF(0x82)
#define NRMIX_EXT_H AGB_MAKEIO_OFF(0x83)
#define NR52   AGB_MAKEIO_OFF(0x84)

#define NR_CH1_CH2_BOUNDARY  0x068
/* Channel 1, 2 dual-wave channels (Channel 1 supports "real-time" frequency conversion scanning) */
/* In GBC/GB games, it is usually used as the main melody channel. */
struct channel_squ {
  int32_t sweep_phase; /* Frequency scan basic timestamp (based on CPU clock cycle) */
  int32_t sweep_trigger; /* Frequency scan update clock point (based on CPU clock cycle) */
  int32_t signal_phase; /* Square wave signal updates the basic timestamp (based on CPU clock cycle).
                                         One f (x) of the waveform, that is, one hz,
                                         scans eight points of the current square wave duty cycle (based on CPU clock cycle). */
  int32_t signal_trigger; /* Square wave signal update clock point (based on CPU clock cycle) */
  int32_t length_phase; /* Basic time stamp of square wave signal sound length (based on CPU clock cycle) */
  int32_t envlope_phase; /* Square wave signal envelope basic timestamp (based on CPU clock cycle) */
  int32_t envlope_trigger; /* Square wave envelope update clock point (based on CPU clock cycle) */
  int32_t signal_volume; /* Signal amplification, also known as Volume. */
  int32_t duty_poll; /* Duty ratio look-up count, which essentially changes tone color, but also has a role in internal frequency division */
  int32_t length_count;/* Length counter */
  uint8_t duty[8]; /* Square wave duty ratio vector table */
  kable sweep;
  kable envlope;
  kable loop;
  kable enable;
  union iopad_blk16  nrx0; /* Square-wave correlation registers used by GBA (sweep frequency) */
  union iopad_blk16  nrx1_x2; /* Square-wave correlation registers used by GBA (duty cycle/length/envelope [GBC NR register merge]) */
  union iopad_blk16  nrx3_x4; /* Square wave correlation registers used by GBA (restart/channel frequency write [high bit] [GBC NR register merge]) */
};
/* Channel 3: 4-bit waveform
/* In GBC/GB games, it is usually used as configurable waveforms,
   such as triangular/sinusoCal synthesis, and can also play some bad voices? */
struct channel_wave {
  int32_t output_phase; /* Waveform update basic timestamp (based on CPU clock cycle) */
  int32_t output_trigger; /* Waveform update clock point, once f (x), that is, 1 Hz will scan 32/64 waveform scan points (based on CPU clock cycle) */
  int32_t length_phase; /* Waveform tone length basic timestamp (based on CPU clock cycle) */
  int32_t length_trigger; /* Waveform length update clock point (based on CPU clock cycle) */
  int32_t length_count;/* Length counter */
  int32_t counter; /* Waveform playback count */
  uint8_t qram[64];  /* 4-bit waveform correlation register used by GBA (PCM4-bit waveform writing) */
  uint16_t xram[16];  /* 4-bit waveform correlation register used by GBA (PCM4-bit waveform writing) */
  union iopad_blk16  nr30; /* 4-bit waveform-related registers used by GBA (channel switch/bank mode selection) */
  union iopad_blk16  nr31_32; /* 4-bit waveform-related registers used by GBA (length/volume) */
  union iopad_blk16  nr33_34; /* 4-bit waveform-related registers used by GBA (frequency/control) */
};
/* Channel 4: LFSR Random noise */
/* Used as special effects and some simple drums in GBC/GB games. */
struct channel_noise {
  int32_t signal_phase; /* Noise signal update basic timestamp (based on CPU clock cycle) */
  int32_t signal_trigger; /* Noise signal update clock point (based on CPU clock cycle) */
  int32_t length_phase; /* Noise length update basic timestamp (based on CPU clock cycle) */
  int32_t length_trigger; /* Noise length update clock point (based on CPU clock cycle) */
  int32_t envlope_phase; /* Noise envelope update basic timestamp (based on CPU clock cycle) */
  int32_t length_count;/* Length counter */
  int32_t envlope_trigger; /* Noise envelope update clock point (based on CPU clock cycle) */
  int32_t signal_output; /* Noise signal output, note. As a level signal, only 0 or 1 will be output. */
  int32_t signal_volume; /* Signal amplification, also known as Volume. */
  union iopad_blk16  nr41_42; /* Noise-related registers used by GBA (length/envelope) */
  union iopad_blk16  nr43_44; /* Noise-related registers used by GBA (frequency/control) */
};
/* Channel 5, 6 FIFO, 8-bit signed PCM, with the same name as DirectSound in Microsoft DirectX. */
/* The strongest voidal channel, supporting 8Bit PCM,
   you can even use to play some clear voices, (Namco's Tales of Phantasia OP in GBA)  */
struct channel_fifo {
  int8_t buf [32]; /* PCM ring buffer */
  int8_t write; /* PCM write location */
  int8_t count; /* PCM effective count */
  int8_t read; /* PCM read location */
  int8_t out;
  uint32_t addr[32]; /* for debug */
  uint32_t addr_cur;
};
struct channel_pcmtest {
  int16_t value;
};
/* Channel master control/settings */
struct channel_control {
  union iopad_blk16  nr50_51;
  union iopad_blk16  nr52;
  union iopad_blk16  snd_ctl;
  union iopad_blk16  pwm_ctl;
};
struct pcm_sample_8 {
  uint8_t bit8[2];
};
struct pcm_sample_16 {
  union {
    uint16_t bit16[2];
    int16_t sbit16[2];
  };
};
struct pcm_sample_flt {
  float bitflt[2];
};
union pcm_sample {
  struct pcm_sample_8 sample_8;
  struct pcm_sample_16 sample_16;
  struct pcm_sample_flt sample_flt;
  uint8_t sample_sg8;
};
union pcm_sample_pt {
  struct pcm_sample_8 *pt_8;
  struct pcm_sample_16 *pt_16;
  struct pcm_sample_flt *pt_flt;
  uint8_t *pt_sg8;
};
/* host device callback for PSG Sound Synthesizer */
struct apu_framebuffer {
  uint8_t buf[735*2*4+64]; /* max size, frame sample max 44100, channel max 2, and per sample max (float) */
  double phase;
  double trigger;
  int byte;
  int blk_align;
  int blk_nums;
  int blk_depth;
  APU_SAMPLE_TYPE sample_request;
};
/* Synthesizer for GBA (based on GBC, adding two 8-bit PCM channels) */
struct apu {
  struct channel_squ squ_ch[2];
  struct channel_wave wave_ch;
  struct channel_noise noise_ch;
  struct channel_fifo fifo_ch[2];
  struct channel_control settings;
  struct apu_framebuffer sndbuf;

  uint64_t mod_buf[4096];
  uint32_t mod_length;
  uint32_t clks;
  uint32_t sam_phase;
  uint32_t sam_trigger;
  uint32_t sam_start; /* start's pos */
  uint32_t sam_play; /* end's pos */
  uint32_t sam_req;
  uint32_t sam_loop_lim;
  uint32_t sam_safe_range;
  int16_t pcm_left;
  int16_t pcm_right;
  intptr_t pcm_st;
  /* Synchronization of audio synthesis
   * -----------------------------------------------------------------------------------------------------------------
   * First we need a delay, then we need to determine the number of PCMs that
   * need to be rendered according to the rendering position and starting position of the PCM,
   * so that the delay can be kept within an acceptable range.
   */
  struct gba *agb;
};
struct gpu_channel {
  union iopad_blk16 ctl;
  union iopad_blk32 loopy_dmy;
  union iopad_blk32 loopy_dmx;
  union iopad_blk16 loopy_x_shadow;
  union iopad_blk16 loopy_y_shadow;
  union iopad_blk16 loopy_x;
  union iopad_blk16 loopy_y;
  union iopad_blk32 dx_shadow;
  union iopad_blk32 dmx_shadow;
  union iopad_blk32 dy_shadow;
  union iopad_blk32 dmy_shadow;
  union iopad_blk16 dx;
  union iopad_blk16 dmx;
  union iopad_blk16 dy;
  union iopad_blk16 dmy;
  union iopad_blk32 ref_x;
  union iopad_blk32 ref_y;
};
/* host device callback for lcd video */
struct ppu_framebuffer {
  int32_t w;
  int32_t h;
  union {
    int16_t *pixel_16; /* gb, gbc, gba use X1B5G5R5 pixel format */
    int32_t *pixel_32;
    void *pixel;
  };
  int32_t pitch;
  GPU_COL_MOLD clr_request;
};
/* Graphic Processors */
struct gpu {
  union iopad_blk16 ctl;
  union iopad_blk16 col_swap;
  union iopad_blk16 status;
  union iopad_blk16 line;
  union iopad_blk16 clip_x[2];
  union iopad_blk16 clip_y[2];
  union iopad_blk16 win_in;
  union iopad_blk16 win_out;
  union iopad_blk16 mosaic;
  union iopad_blk16 bld_ctl;
  union iopad_blk16 bld_args;
  union iopad_blk16 bri_args;
  union iopad_blk16 ugpio_4E;
  union iopad_blk16 ugpio_56;

  uint8_t oam[0x400+32];/* 1K Object attr mem (OAM) */
  uint8_t vram[0x20000+32]; /* 96K video memory (nametable and chr or bitmap data)*/
  uint8_t palette[0x400+32]; /* 1K bg/sp palette */
  uint8_t palette2[0x400+32]; /* dummy palette */
  uint8_t *palette16_b; /* bank for col16*/
  uint16_t palette3[0x400+32]; /* dummy palette */
  uint16_t palette4[0x400+32]; /* dummy palette */

  union {
    uint16_t vbufb[1024*640];
    uint32_t vbufb32[1024*640];
  };
  union {
    uint16_t vptr_cahce[1024];
    uint32_t vptr_cahce32[1024];
  };
  union {
    uint16_t *vbuf;
    uint32_t vbuf32;
  };
  union {
    uint16_t vptr_pitch;
    uint32_t vptr_pitch32;
  };
  uint16_t *vattr;
  uint16_t *vptrcc;
  uint16_t vattrb[4096+32];
  uint32_t clks;
  /*  d0:objwin
       d1:win0
       d2:win1
       d3:sprite mask
       d4:chan 0 mask
       d5:chan 1 mask
       d6:chan 2 mask
       d7:chan 3 mask
    */
#define SOLID_MODE_OBJWIN_MASK 0x01
#define SOLID_MODE_WIN0_MASK 0x02
#define SOLID_MODE_WIN1_MASK 0x04
#define SOLID_MODE_FULL_MASK (SOLID_MODE_OBJWIN_MASK |  SOLID_MODE_WIN0_MASK | SOLID_MODE_WIN1_MASK)
#define SOLID_CHAN_SPRITE_MASK 0x08
#define SOLID_CHAN_BG_BASE_MASK 0x10
#define SOLID_CHAN_0_MASK 0x10
#define SOLID_CHAN_1_MASK 0x20
#define SOLID_CHAN_2_MASK 0x40
#define SOLID_CHAN_3_MASK 0x80
#define SOLID_CHAN_FULL_MASK (SOLID_CHAN_0_MASK | SOLID_CHAN_1_MASK | SOLID_CHAN_2_MASK | SOLID_CHAN_3_MASK | SOLID_CHAN_SPRITE_MASK)

  struct gpu_channel chan[5];
  struct ppu_framebuffer vid_buf;
  struct gba *agb;

  GPU_COL_MOLD mold;
};
struct serial {
  uint16_t temp; /* TODO:*/
  struct gba *agb;
};

#define BREAKPOINT_REASON_R15 0
#define BREAKPOINT_REASON_MEM_READ 1
#define BREAKPOINT_REASON_MEM_WRITE 2
#define BREAKPOINT_REASON_HARDWARE_INTERRUPT 3
#define BREAKPOINT_REASON_SOFTWARE_INTERRUPT 4
#define BREAKPOINT_REASON_GLOBAL 5
#define BREAKPOINT_REASON_STEP_IN 6
#define BREAKPOINT_REASON_STEP_OVER 7
#define BREAKPOINT_REASON_STEP_OUT 8
#define BREAKPOINT_REASON_CURSOR 9
#define BREAKPOINT_REASON_INTERRUPT 10
#define BREAKPOINT_REASON_UNKNOWN -1
typedef int BREAKPOINT_REASON;

struct breakpoint_settings {
  kable enable;
  int32_t match_loop;
  int32_t match_expect;
  intptr_t usr_data;

  union {
    uint32_t address; /* for addr, read and write breakpoint */
    uint32_t int_id; /* for software breakpoint interrupt id */
  };
  int size; /* for read, write memory size */
};
struct breakpoint_expr;
struct breakpoint {
  kable enable;
  uint32_t address;
  BREAKPOINT_REASON reason;
  int32_t match_loop;
  int32_t match_expect;
  int32_t int_id;
  struct breakpoint_expr *expr;
  int size;
  char *describe;
  int desclen;
  char *comment;
  int comlen;
  intptr_t usr_data;
};
/* Only support syntax :
    elem := GBA(ARM7TDMI) -> r0~r15, r16 (r16 is spsr), r99 (r99 is cpsr)
                             | irq_, svc_, sys_, usr_ r0~r15, r16
                             | mbus+w|h|b e.g.  1245678:w
                             | number e.g. 11245 (both hex number)
    op   :=  -> >, <, >=, <=, ==,  &, |, ~, (, ), ^, >>, <<, ||, &&, !, +, -, *, / */
 /* keyword := @nw:uint32_t, new write val for mem write, ub in other mode
                   := @pc:uint32_t, correct pc address
                   := @cntset:uint32_t set match count
                   := @cntcur:uint32_t cur match count
                   := @reason:uin32_t same as BREAKPOINT_REASON
                   := @int_mask interrupt mask
  /* self link symbols. -           ! ~ - +*/
  /* link pri, same as clang. */
/* elem check */
/* self link check */
/* each link check */

struct dma_log {
  char *infos_buffer;

};



struct driver {
  void  (callc *controller_drv)  (struct gba *agb, void *input_base);
  void * controller_obj;
  void  (callc *video_post)  (struct gba *agb,  struct ppu_framebuffer *);
  void * lcd_obj;
  void  (callc *sound_post)   (struct gba *agb,  struct apu_framebuffer *);
  void * sound_obj;
  void  (callc *vsync)  (struct gba *agb);
  void * vsync_obj;
  void  (callc *bp_hit)  (struct gba *agb,  struct breakpoint *dev_bp);
  void * bp_obj;
};

#define EEPROM_FREE 0
#define EEPROM_SET_READ_ADDRESS 2
#define EEPROM_SET_WRITE_ADDRESS 3
#define EEPROM_READ_ACK 4
#define EEPROM_WRITE_ACK 5
typedef int EEPROM_STATUS;

#define EEPROM_CAPACITY_512B 0
#define EEPROM_CAPACITY_8K 1
#define EEPROM_CAPACITY_AUTO 2
typedef int EEPROM_CAPACITY;

struct sram {
  uint8_t memory[0x10000+32];
  struct gba *agb;
};
struct eeprom {
  uint8_t memory[0x2000];
  uint8_t bit;
  uint32_t address;
  uint32_t r_stop;
  uint32_t byte;
  uint8_t *bank;
  uint16_t bank_id;
  uint8_t cmd[2];
  uint8_t packet[256];
  uint8_t *write64;
  uint32_t _trunc;
  EEPROM_CAPACITY capacity;
  EEPROM_STATUS status;
  struct gba *agb;
};

#define FLASH_FREE 0
#define FLASH_GET_DEVICE_HASH 1
#define FLASH_END_DEVICE_HASH 2
#define FLASH_ERASE_ENTIRE 3
#define FLASH_ERASE_COMMAND_ENTRY 4
#define FLASH_ERASE_SECTOR_WITHOUT_ATMEL 5
#define FLASH_ERASE_WRITE_SECTOR_ONLY_ATMEL 6
#define FLASH_WRITE_ONE_BYTE 7
#define FLASH_BANK_SWITCH 8
typedef int FLASH_STATUS;

struct rt_clock {

  kable support;
  struct gba *agb;
};

#define SAVE_BACKUP_NONE 0
#define SAVE_BACKUP_GM76V256CLLFW10 1
#define SAVE_BACKUP_MB85R256 2
#define SAVE_BACKUP_EEPROM_512B 3
#define SAVE_BACKUP_EEPROM_8K 4
#define SAVE_BACKUP_FLASH_SST39LF_VF512 5
#define SAVE_BACKUP_FLASH_AT29LV512 6
#define SAVE_BACKUP_FLASH_SANYO 7
#define SAVE_BACKUP_FLASH_MACRONIX_128K 8
#define SAVE_BACKUP_FLASH_MACRONIX_64K 9
#define SAVE_BACKUP_FLASH_PANASONIC 10
#define SAVE_BACKUP_FLASH_128K SAVE_BACKUP_FLASH_SANYO
#define SAVE_BACKUP_FLASH_64K SAVE_BACKUP_FLASH_SST39LF_VF512

typedef int SAVE_BACKUP;

struct flash {
  uint8_t memory[0x20000];
  uint8_t _64k_bankId;
  uint8_t *bank;
  uint8_t man;
  uint8_t dev;
  uint8_t rd_waitstate;
  uint8_t wr_waitstate;
  uint8_t cmd_loc;
  uint8_t block_cnt;
  FLASH_STATUS status;
  SAVE_BACKUP backup_id;
  struct gba *agb;
};

struct backup_infos_ {
  kable rt_clock_support;
  kable crc32_support;
  SAVE_BACKUP backup_id;
  char title[64 + 1];
  uint32_t crc32;
  const char *comment;
};

struct backup_infos {
  kable rt_clock_support;
  SAVE_BACKUP backup_id;
  char title[64 + 1];
  const char *comment;
};

struct memory {
  uint32_t novel_access;
  uint32_t cache_ptr; /* I'm not really sure about the detailed caching information of the GBA machine. */

  uint8_t SRam[0x10000+8];
  uint8_t PRom[0x2000000+8];
  uint8_t WRam[0x40000+8];
  /* 0000011E: 4958    ldr     r1, =#0xFFFFFE00
00000120: 5060    str     r0, [r4, r1] */
  uint8_t IRam[0x8000+8];
  uint8_t bios[0x4000+8];
  uint8_t IOMap[0x1000000+8];

  union iopad_blk32 imc;
  union iopad_blk32 wait;

  union {
    struct eeprom eeprom;
    struct flash flash;
  } save_backup;

  struct eeprom *eeprom_ptr;
  struct flash *flash_ptr;

  uint32_t pwait_seq[3];
  uint32_t pwait_noseq[3];
  uint32_t sram_wait;
  uint32_t wram_wait;

  kable sram_en;

  SAVE_BACKUP backup_id;
};
struct misc {
  union iopad_blk8 post;
  union iopad_blk32 halt;
  union iopad_blk32 undoc_410;
  int32_t clks_line;
  int32_t clks_next;
  int32_t clks_reset;
  int32_t clks_left;
};

struct stack_trace_chunk {
  kable status; /* 0: init 1: push 2: push  */
  kable thumb;
  uint8_t r_id;
  uint32_t do_addr;
};

struct debug_ {
  kable trap;
  kable debug;
  kable bp_hit;
  uint16_t int_enum;
  struct stack_trace_chunk stack_trace[32*1024/ 4]; /* FIXME - XXX: for efficiency, the AGB stack always exists in the IRAM region, and always align a word, isn't it?*/
  struct breakpoint *breakpoint_slot[(304*1024+32*1024*1024)/ 2];
  struct breakpoint weak_bp;
  struct list_ *addr_bp_list;
  struct list_ *read_bp_list;
  struct list_ *write_bp_list;
  struct list_ *int_bp_list;
  struct list_ *soft_int_bp_list;
};

struct gba {
  struct arm7 arm7;
  struct controller joypad;
  struct timer timer;
  struct apu apu;
  struct dma dma;

  struct memory mem;
  struct gpu gpu;
  struct serial sio;
  struct driver platform_io;
  struct rt_clock rc_clock;
  struct header rom_header;
  struct breakpoint weak_bp;
  struct coroutine *carrier;
  struct debug_ dbg;
  struct misc sundry;
};
struct gba_prototype {
  int (callc *create)(struct gba **agb);
  int (callc *fetchrom)(struct gba *agb, FILE *fp);
  int (callc *fetchrom_ansi)(struct gba *agb, const char *filename);
  int (callc *fetchrom_unicode)(struct gba *agb, const wchar_t *filename);
  void (callc *destroy)(struct gba **agb);
  int (callc *frame)(struct gba *agb);
  int (callc *set_bios)(struct gba *agb, void *bios_bin, int size);
  int (callc *setdriver)(struct gba *agb, struct driver *drv);
  int (callc *setgpu_coltype)(struct gba *agb, GPU_COL_MOLD col_mold);
  int (callc *set_apubuffer)(struct gba *agb, APU_SAMPLE_TYPE sample, int sample_sec);
  uint16_t (callc *fast_rhw)(struct gba *agb, uint32_t addr);
  kable (callc *iscode_addr)(struct gba *agb, uint32_t addr);
  int (callc *read_battery)(struct gba *agb, FILE *fd);
  int (callc *save_battery)(struct gba *agb, FILE *fd);
  int (callc *set_debug)(struct gba *agb, kable enable);
  int (callc *stepin)(struct gba *agb);
  int (callc *stepover)(struct gba *agb);
  int (callc *stepout)(struct gba *agb);
  int (callc *cursor_bp)(struct gba *agb, uint32_t address);
  struct breakpoint *(callc *breakpoint_create) (struct gba *agb, kable add_todbglist, BREAKPOINT_REASON reason, kable enable, uint32_t address, int size, char *describe, int desclen, int match_cnt, char *comment, int comlen, char *err_buf, int buf_max);
  int (callc *breakpoint_modify) (struct gba *agb, struct breakpoint *breakpoint_, kable enable, uint32_t address, int size, char *describe, int desclen, int match_cnt, char *comment, int comlen, char *err_buf, int buf_max);
  int (callc *breakpoint_insert) (struct gba *agb, struct breakpoint *breakpoint);
  int (callc *breakpoint_remove) (struct gba *agb, struct breakpoint *breakpoint);
  struct breakpoint *(callc *get_addr_breakpoint_slot) (struct gba *agb, uint32_t addr_);
  void (callc *set_addr_breakpoint_slot) (struct gba *agb, uint32_t addr_,  struct breakpoint *bp);
  void (callc *crt_init) (void);
  void *dl_handle;
};

/* GBA common user method */
gba_impl int callc  gba_create (struct gba **agb);
gba_impl int callc  gba_fetchrom (struct gba *agb, FILE *fp); /* only for inline method */
gba_impl int callc  gba_fetchrom_ansi (struct gba *agb, const char *file_name);
gba_impl int callc  gba_fetchrom_unicode (struct gba *agb, const wchar_t *file_name);
gba_impl void callc  gba_destroy (struct gba **agb);
gba_impl int callc  gba_frame (struct gba *agb);
gba_impl int callc  gba_set_bios (struct gba *agb, void *bios_bin, int size);
gba_impl int callc  gba_setdriver (struct gba *agb, struct driver *drv);
gba_impl int callc  gba_setgpu_coltype (struct gba *agb, GPU_COL_MOLD col_mold);
gba_impl int callc  gba_set_apubuffer (struct gba *agb, APU_SAMPLE_TYPE sample, int sample_sec);
gba_impl uint16_t callc  gba_fast_rhw (struct gba *agb, uint32_t addr);
gba_impl kable callc  gba_iscode_addr (struct gba *agb, uint32_t addr);
gba_impl int callc  gba_read_battery (struct gba *agb, FILE *fd);
gba_impl int callc  gba_save_battery (struct gba *agb, FILE *fd);

/* GBA debugger method */
gba_impl int callc  gba_setdebug (struct gba *agb, kable enable);
gba_impl int callc  gba_stepin (struct gba *agb);
gba_impl int callc  gba_stepover (struct gba *agb);
gba_impl int callc  gba_stepout (struct gba *agb);
gba_impl int callc  gba_cursor_bp (struct gba *agb, uint32_t address);
gba_impl struct breakpoint *callc  gba_breakpoint_create (struct gba *agb, kable add_todbglist, BREAKPOINT_REASON reason, kable enable, uint32_t address, int size, char *describe, int desclen, int match_cnt, char *comment, int comlen, char *err_buf, int buf_max);
gba_impl int callc  gba_breakpoint_modify (struct gba *agb, struct breakpoint *breakpoint_, kable enable, uint32_t address, int size, char *describe, int desclen, int match_cnt, char *comment, int comlen, char *err_buf, int buf_max);
gba_impl int callc  gba_breakpoint_insert (struct gba *agb, struct breakpoint *breakpoint);
gba_impl int callc  gba_breakpoint_remove (struct gba *agb, struct breakpoint *breakpoint);
gba_impl struct breakpoint *callc gba_get_addr_breakpoint_slot (struct gba *agb, uint32_t addr_);
gba_impl void callc gba_set_addr_breakpoint_slot (struct gba *agb, uint32_t addr_,  struct breakpoint *bp);

finline
uint32_t gba_get_cpsr (struct gba *const agb) {

  uint32_t blk = 0;

  if (agb->arm7.cpsr_fast.flags.sig.n != 0)
    blk |= ARM7_FLAG_N;
  if (agb->arm7.cpsr_fast.flags.sig.z != 0)
    blk |= ARM7_FLAG_Z;
  if (agb->arm7.cpsr_fast.flags.sig.c != 0)
    blk |= ARM7_FLAG_C;
  if (agb->arm7.cpsr_fast.flags.sig.v != 0)
    blk |= ARM7_FLAG_V;
  if (agb->arm7.cpsr_fast.fiq_disable != false)
    blk |= ARM7_FIQ_DIS_MASK;
  if (agb->arm7.cpsr_fast.irq_disable != false)
    blk |= ARM7_IRQ_DIS_MASK;
  if (agb->arm7.cpsr_fast.thumb != false)
    blk |= ARM7_OPCODE_THUMB_MASK;
  blk |= agb->arm7.cpsr_fast.mode | ARM7_MODE_HSET_MASK;
  return blk;
}

static
uint32_t gba_get_cur_spsr (struct arm7 *arm7) {

  switch (arm7->cpsr_fast.mode & 15) {
  case ARM7_MODE_USER:
  case ARM7_MODE_SYS:
    return 0;
  case ARM7_MODE_FIQ:
    return arm7->spsr_t[SPSRb_FIQ].blk;
  case ARM7_MODE_IRQ:
    return arm7->spsr_t[SPSRb_IRQ].blk;
  case ARM7_MODE_MGR:
    return arm7->spsr_t[SPSRb_MGR].blk;
  case ARM7_MODE_UNDEF:
    return arm7->spsr_t[SPSRb_UDEF].blk;
  case ARM7_MODE_ABT:
    return arm7->spsr_t[SPSRb_ABT].blk;
  default:
    return 0;
    assert (0);
  }
}

#ifndef GBA_PROTOTYPE_IMPORT
/* GBA internal method */
int  gba_set_savebackup (struct gba *agb, SAVE_BACKUP backup_id);
int  gba_set_sram (struct gba *agb);
int  gba_set_rtc (struct gba *agb);

uint8_t  agb_mbus_rb_seq (struct gba *agb, uint32_t addr);
uint8_t  agb_mbus_rb_noseq (struct gba *agb, uint32_t addr);
uint16_t  agb_mbus_rhw_seq (struct gba *agb, uint32_t addr);
uint16_t  agb_mbus_rhw_noseq (struct gba *agb, uint32_t addr);
uint32_t  agb_mbus_rw_noseq (struct gba *agb, uint32_t addr);
uint32_t  agb_mbus_rw_seq (struct gba *agb, uint32_t addr);
uint16_t  agb_dma_rhw_seq (struct gba *agb, uint32_t addr);
uint16_t  agb_dma_rhw_noseq (struct gba *agb, uint32_t addr);
uint32_t  agb_dma_rw_noseq (struct gba *agb, uint32_t addr);
uint32_t  agb_dma_rw_seq (struct gba *agb, uint32_t addr);
void  agb_mbus_wb_noseq (struct gba *agb, uint32_t addr, uint8_t value);
void  agb_mbus_wb_seq (struct gba *agb, uint32_t addr, uint8_t value);
void  agb_mbus_whw_noseq (struct gba *agb, uint32_t addr, uint16_t value);
void  agb_mbus_whw_seq (struct gba *agb, uint32_t addr, uint16_t value);
void  agb_mbus_ww_noseq (struct gba *agb, uint32_t addr, uint32_t value);
void  agb_mbus_ww_seq (struct gba *agb, uint32_t addr, uint32_t value);
void  agb_dma_whw_noseq (struct gba *agb, uint32_t addr, uint16_t value);
void  agb_dma_whw_seq (struct gba *agb, uint32_t addr, uint16_t value);
void  agb_dma_ww_noseq (struct gba *agb, uint32_t addr, uint32_t value);
void  agb_dma_ww_seq (struct gba *agb, uint32_t addr, uint32_t value);

uint16_t  agb_code_rhw_seq (struct gba *agb, uint32_t addr) ;
uint16_t  agb_code_rhw_noseq (struct gba *agb, uint32_t addr) ;
uint32_t  agb_code_rw_seq (struct gba *agb, uint32_t addr);
uint32_t  agb_code_rw_noseq (struct gba *agb, uint32_t addr);
void   agb_gamepak_prefetch (struct gba *agb, uint32_t addr, uint32_t internal_clks);
void  agb_irq_breakpoint (struct gba *agb, uint16_t mask);

finline
void reset_int_magic (struct gba *const agb) {
  struct arm7 *const arm = & agb->arm7;

  arm->int_magic.blk8[0] =!  (arm->ie.blk &arm->ifs.blk & 0x3FFF) ;
  arm->int_magic.blk8[1] =0;
  arm->int_magic.blk8[2] =!  (arm->ime.blk & 1);
  arm->int_magic.blk8[3] =!!  (arm->cpsr_fast.irq_disable);
}

finline
void check_halt (struct gba *agb, uint16_t int_mask) {
  reset_int_magic (agb);
  if ((agb->sundry. halt.blk & 0x80000080) == 0x80000000)
    if (agb->arm7.ie.blk & int_mask) {
      agb->sundry. halt.blk &= ~0x80000000;
    }
    else ;
  else ;
}

finline
void check_stop (struct gba *agb, uint16_t int_mask) {
  reset_int_magic (agb);
  if ((agb->sundry. halt.blk & 0x80000080) == 0x80000080)
    if (agb->arm7.ie.blk & int_mask)
      agb->sundry. halt.blk &= ~0x80000000;
    else ;
  else ;
}
static
void gba_proto_destroy (struct gba_prototype *prototype) {
#if defined (_WIN32) || defined (WIN32)
  if (prototype->dl_handle != null) {
    CloseHandle ((HANDLE) prototype->dl_handle);
  }
#else
#endif
}

#else
static int gba_prototype_load (struct gba_prototype *prototype) {
#if defined (_WIN32) || defined (WIN32)
#  define GBA_PROTO_LIB "ArchBoyAdvance-CoreLibrary.dll"
  CHAR szPath[MAX_PATH];
  HMODULE win32hd_lib = NULL;

  GetModuleFileNameA (NULL, szPath, sizeof (szPath));
  PathRemoveFileSpecA (szPath);

  strcat (szPath, "\\");
  strcat (szPath, GBA_PROTO_LIB);

  win32hd_lib = LoadLibraryA (szPath);
  assert (win32hd_lib != NULL);

  memset (prototype, 0, sizeof (struct gba_prototype));

  prototype->create = (int (callc *)(struct gba **))GetProcAddress (win32hd_lib, "gba_create");
  prototype->destroy = (void (callc *)(struct gba **))GetProcAddress (win32hd_lib, "gba_destroy");
  prototype->fetchrom = (int (callc *)(struct gba *, FILE *))GetProcAddress (win32hd_lib, "gba_fetchrom");
  prototype->fetchrom_ansi = (int (callc *)(struct gba *, const char *))GetProcAddress (win32hd_lib, "gba_fetchrom_ansi");
  prototype->fetchrom_unicode = (int (callc *)(struct gba *, const wchar_t *))GetProcAddress (win32hd_lib, "gba_fetchrom_unicode");
  prototype->frame = (int (callc *)(struct gba *))GetProcAddress (win32hd_lib, "gba_frame");
  prototype->setdriver = (int (callc *)(struct gba *, struct driver *))GetProcAddress (win32hd_lib, "gba_setdriver");
  prototype->set_bios = (int (callc *)(struct gba *, void *, int))GetProcAddress (win32hd_lib, "gba_set_bios");
  prototype->read_battery =  (int (callc *)(struct gba *, FILE *))GetProcAddress (win32hd_lib, "gba_read_battery");
  prototype->save_battery =  (int (callc *)(struct gba *, FILE *))GetProcAddress (win32hd_lib, "gba_save_battery");
  prototype->setgpu_coltype = (int (callc *)(struct gba *, GPU_COL_MOLD))GetProcAddress (win32hd_lib, "gba_setgpu_coltype");
  prototype->set_apubuffer = (int (callc *)(struct gba *, APU_SAMPLE_TYPE, int))GetProcAddress (win32hd_lib, "gba_set_apubuffer");
  prototype->set_debug = (int (callc *)(struct gba *, kable))GetProcAddress (win32hd_lib, "gba_setdebug");
  prototype->breakpoint_create = (struct breakpoint *(callc *)(struct gba *, kable, BREAKPOINT_REASON, kable, uint32_t, int, char *, int, int, char *, int, char *, int ))GetProcAddress (win32hd_lib, "gba_breakpoint_create");
  prototype->breakpoint_modify = (int (callc *)(struct gba *, struct breakpoint *, kable, uint32_t, int, char *, int, int, char *, int, char *, int))GetProcAddress (win32hd_lib, "gba_breakpoint_modify");
  prototype->breakpoint_insert = (int (callc *)(struct gba *, struct breakpoint *))GetProcAddress (win32hd_lib, "gba_breakpoint_insert");
  prototype->breakpoint_remove = (int (callc *)(struct gba *, struct breakpoint *))GetProcAddress (win32hd_lib, "gba_breakpoint_remove");
  prototype->cursor_bp = (int (callc *)(struct gba *, uint32_t))GetProcAddress (win32hd_lib, "gba_cursor_bp");
  prototype->stepin = (int (callc *)(struct gba *))GetProcAddress (win32hd_lib, "gba_stepin");
  prototype->stepout = (int (callc *)(struct gba *))GetProcAddress (win32hd_lib, "gba_stepout");
  prototype->stepover = (int (callc *)(struct gba *))GetProcAddress (win32hd_lib, "gba_stepover");
  prototype->fast_rhw = (uint16_t (callc *)(struct gba *, uint32_t))GetProcAddress (win32hd_lib, "gba_fast_rhw");
  prototype->iscode_addr = (kable (callc *)(struct gba *, uint32_t))GetProcAddress (win32hd_lib, "gba_iscode_addr");
  prototype->set_addr_breakpoint_slot = (void (callc *)(struct gba *, uint32_t, struct breakpoint *))GetProcAddress (win32hd_lib, "gba_set_addr_breakpoint_slot");
  prototype->get_addr_breakpoint_slot = (struct breakpoint *(callc *)(struct gba *, uint32_t))GetProcAddress (win32hd_lib, "gba_get_addr_breakpoint_slot");

  assert (prototype->create != null);
  assert (prototype->destroy != null);
  assert (prototype->fetchrom != null);
  assert (prototype->fetchrom_ansi != null);
  assert (prototype->fetchrom_unicode != null);
  assert (prototype->frame != null);
  assert (prototype->setdriver != null);
  assert (prototype->set_bios != null);
  assert (prototype->read_battery != null);
  assert (prototype->save_battery != null);
  assert (prototype->setgpu_coltype != null);
  assert (prototype->set_apubuffer != null);
  assert (prototype->set_debug != null);
  assert (prototype->breakpoint_create != null);
  assert (prototype->breakpoint_modify != null);
  assert (prototype->breakpoint_insert != null);
  assert (prototype->breakpoint_remove != null);
  assert (prototype->cursor_bp != null);
  assert (prototype->stepin != null);
  assert (prototype->stepout != null);
  assert (prototype->stepover != null);
  assert (prototype->fast_rhw != null);
  assert (prototype->iscode_addr != null);
  assert (prototype->set_addr_breakpoint_slot != null);
  assert (prototype->get_addr_breakpoint_slot != null);

  prototype->dl_handle = (void *)win32hd_lib;
  return 0;
#else
#endif
}

#endif

#ifdef __cplusplus
}
#endif

#endif
