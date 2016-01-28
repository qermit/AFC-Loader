
#KBUILD_CFLAGS += $(call cc-option,-mpreferred-stack-boundary=2)
#port_flags += $(call cc-option, -D__USE_LPCOPEN,)
port_flags += $(call cc-option, -D__LPC17XX__ )
port_flags += $(call cc-option, -D__CODE_RED )


LINUXINCLUDE +=  -I$(srctree)/port/cortex-m3/lpc175x_6x/board/inc \
        -I$(srctree)/port/cortex-m3/lpc175x_6x/chip/inc 

port-y += port/cortex-m3/lpc175x_6x/



#KBUILD_CFLAGS += $(tmp_flags)

