# afcipm bootloader
MMC bootloader for NXP lpc176x

Parts of code were taken from 
* coreIPM project
* original AFC MMC implementation
* DESY/CERN mmc_v2

Followed directories were imported to this project
 * src/board  - lpcxpresso 1769 board files
 * src/chip - lpcopen chip_lpc_175x_6x
 
lpcxpresso 1769 pinout description:
 - UART tx - P4[29] (PAD16)
 - UART rx - P4[28] (PAD15)
 - IPMI I2C SCL - P0[27] (J6-25)
 - IPMI I2C SDA - P9[28] (J6-26)

# Warning!!!
lpcxpresso works with 12MHz oscillator, AFC/AFCK has 8MHz
This code does not checks geographical position yet.
This code will work only in slot 3 


# Licencing

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.