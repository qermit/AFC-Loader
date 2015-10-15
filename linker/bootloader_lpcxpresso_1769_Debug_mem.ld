/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2015
 * (c) NXP Semiconductors 2013-2015
 * Linker script memory definitions
 * Created from LinkMemoryTemplate
 * By LPCXpresso v7.7.2 [Build 379] [2015-03-31]  on Thu Jul 23 15:40:17 CEST 2015)
*/

MEMORY
{
  /* Define each memory region */
  BootSector (rx) : ORIGIN = 0x0,    LENGTH = 0x8000 /* 512K bytes */
  FruSector (rx) : ORIGIN = 0x8000, LENGTH = 0x1000
  MMCSector (rx) : ORIGIN = 0x9000, LENGTH = 0x79000
  RamLoc32 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x8000 /* 32K bytes */
  RamAHB32 (rwx) : ORIGIN = 0x2007c000, LENGTH = 0x8000 /* 32K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_BootSector = 0x0 + 0x6000;
  __top_RamLoc32 = 0x10000000 + 0x8000;
  __top_RamAHB32 = 0x2007c000 + 0x8000;



