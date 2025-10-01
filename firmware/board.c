#include "board.h"

void delay_us(uint16_t n)
{
  #ifdef FREQ_SYS
    #if FREQ_SYS <= 6000000
      n >>= 2;
    #endif
    #if FREQ_SYS <= 3000000
      n >>= 2;
    #endif
    #if FREQ_SYS <= 750000
      n >>= 4;
    #endif
  #endif

  while(n) {                        // total = 12~13 Fsys cycles, 1uS @Fsys=12MHz
    SAFE_MOD++;                     // 2 Fsys cycles, for higher Fsys, add operation here
    #ifdef FREQ_SYS
      #if FREQ_SYS >= 14000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 16000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 18000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 20000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 22000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 24000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 26000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 28000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 30000000
        SAFE_MOD++;
      #endif
      #if FREQ_SYS >= 32000000
		    SAFE_MOD++;
      #endif
    #endif
		n--;
  }
}

void delay_ms(uint16_t n)
{
  while(n) {
    delay_us(1000);
    n--;
  }
}

void board_init(void)
{
  // Set internal oscilator:
  SAFE_MOD = 0x55;
  SAFE_MOD = 0xAA;

  // 24MHz
  CLOCK_CFG = CLOCK_CFG & ~ MASK_SYS_CK_SEL | 0x06;

  SAFE_MOD = 0x00;
  // Wait to stablize internal RC.
  delay_ms(10);
}

void board_reset(void)
{
  SAFE_MOD    = 0x55;
  SAFE_MOD    = 0xAA;
  GLOBAL_CFG |= bSW_RESET;
}
