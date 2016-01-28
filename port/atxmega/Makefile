
#KBUILD_CFLAGS += $(call cc-option,-mpreferred-stack-boundary=2)
port_flags :=
KBUILD_CFLAGS += -mmcu=$(MCU)

#port_flags := $(call cc-option, -DCORE_M3)
#port_flags += $(call cc-option, -D__NEWLIB__)



port_flags += $(call cc-option, -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0)



port_flags += $(call cc-option, -I$(srctree)/FreeRTOS/portable/GCC/ATMega323)

#AFS
#port_flags += $(call cc-option, -DXMEGA)
port_flags += $(call cc-option, -std=gnu99)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/Config)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/common/utils)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/common/services/ioport)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/common/services/sleepmgr)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/common/services/clock)

port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/drivers/twi)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/utils/preprocessor)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/utils)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/drivers/cpu)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/drivers/nvm)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/drivers/pmic)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/drivers/sleep)
port_flags += $(call cc-option, -I$(srctree)/port/atxmega/include/ASF/xmega/drivers/tc)

port-y += port/atxmega/
port-y += FreeRTOS/portable/GCC/ATMega323/


LINUXINCLUDE +=  -I$(srctree)/port/atxmega/board/inc \
		-I$(srctree)/port/atxmega/chip/inc


ifdef CONFIG_CPU_AVRXMEGA7
	LDFLAGS := -m avrxmega7
	
endif
#ifdef CONFIG_MCU_LPC176X
#
#include port/cortex-m3/lpc175x_6x/Makefile

#endif

KBUILD_CFLAGS += $(port_flags)

LDFLAGS_jammci += -Wl,-gc-sections --specs=nosys.specs -nostdlib -static -nostartfiles
