/*
 * GENERATED FILE - DO NOT EDIT
 * (c) Code Red Technologies Ltd, 2008-2015
 * (c) NXP Semiconductors 2013-2015
 * Linker script memory definitions
 * Created from LinkMemoryTemplate
 * By LPCXpresso v7.7.2 [Build 379] [2015-03-31]  on Thu Oct 15 10:23:38 CEST 2015)
*/

MEMORY
{
  /* Define each memory region */
  BootSector (rx) : ORIGIN = 0x0, LENGTH = 0x20000 /* 128K bytes */
  RamLoc16 (rwx) : ORIGIN = 0x10000000, LENGTH = 0x4000 /* 16K bytes */
  RamAHB16 (rwx) : ORIGIN = 0x2007c000, LENGTH = 0x4000 /* 16K bytes */


}
  /* Define a symbol for the top of each memory region */
  __top_BootSector = 0x0 + 0x20000;
  __top_RamLoc16 = 0x10000000 + 0x4000;
  __top_RamAHB16 = 0x2007c000 + 0x4000;