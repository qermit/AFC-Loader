#!/bin/sh


set -x 

vmlinux_link()
{
        local lds="${objtree}/${KBUILD_LDS}"

#        if [ "${SRCARCH}" != "um" ]; then
#               ${LD} ${LDFLAGS} ${LDFLAGS_jammci} -o ${2}                  \
#                       -T ${lds}            \
#                       --start-group ${KBUILD_JAMMCI_CORE} --end-group ${1}
#        else
#                ${CC} ${CFLAGS_vmlinux} -o ${2}                              \
#                        -Wl,-T,${lds} ${KBUILD_JAMMCI_CORE}                  \
#			${1}
#                        -Wl,--start-group                                    \
#                                 ${KBUILD_VMLINUX_MAIN}                      \
#                        -Wl,--end-group                                      \
#                        -lutil -lrt ${1}
                rm -f linux
#        fi

${CC} ${CFLAGS_jammci} -o ${2} -Wl,-T,${lds} ${KBUILD_JAMMCI_CORE} ${1} -nostartfiles "-Wl,-Map=${2}.map" \
	 -Wl,-gc-sections --specs=nosys.specs -nostdlib -static -nostartfiles \
	-lm -lc -lgcc \
	-fno-builtin -ffunction-sections -fdata-sections -fno-strict-aliasing -fmessage-length=0


}


vmlinux_link "" jammci


#echo $@
#export
