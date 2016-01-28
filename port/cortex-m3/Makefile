
#KBUILD_CFLAGS += $(call cc-option,-mpreferred-stack-boundary=2)
KBUILD_CFLAGS += -mcpu=$(MCPU) -mthumb

port_flags := $(call cc-option, -DCORE_M3)
port_flags += $(call cc-option, -D__NEWLIB__)

port_flags += $(call cc-option, -fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0)

port_flags += $(call cc-option, -I$(srctree)/FreeRTOS/portable/GCC/ARM_CM3)

port-y += FreeRTOS/portable/GCC/ARM_CM3/

ifdef CONFIG_MCU_LPC176X

include port/cortex-m3/lpc175x_6x/Makefile


endif

KBUILD_CFLAGS += $(port_flags)


LDFLAGS_jammci += -Wl,-gc-sections --specs=nosys.specs -nostdlib -static -nostartfiles
