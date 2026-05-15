#!/bin/bash
system=$(uname);arch=$(arch); echo
system_windows=$(echo $system | grep -i "mingw\|cygwin"); system_linux=$(echo $system | grep -i "linux"); system_mac=$(echo $system | grep -i "Darwin"); echo System: $system
arch_x86=$(echo $arch | grep -i "86"); arch_arm=$(echo $arch | grep -i "arm"); echo Arch: $arch
echo "**************** merge sec ota begin ****************"
output="${2%.bin}_sec_ota.bin"  # 输出: xxx_sec_ota.bin
if [ ! -z $system_linux ]; then echo
	if [ ! -z $arch_x86 ]; then echo
		chmod a+x ${1}/linux/x86/Secure_Boot_Tool_Linux; cat ${3} ${2} > ${output}; echo  
	else echo
		echo [Error] $system:$arch not support
	fi; echo
elif [ ! -z $system_windows ]; then echo
	if [ ! -z $arch_x86 ]; then echo
		cat ${2} ${1} > ${output}; echo 
	else echo
		echo [Error] $system:$arch not support
	fi; echo
elif [ ! -z $system_mac ]; then echo
	if [ ! -z $arch_x86 ]; then echo
		chmod a+x ${1}/mac/x86/Secure_Boot_Tool_Mac_X64; cat ${3} ${2} > ${output}; echo  
	elif [ ! -z $arch_arm ]; then echo
		chmod a+x ${1}/mac/arm/Secure_Boot_Tool_Mac_AARCH64; cat ${3} ${2} > ${output}; echo  
	else echo
		echo [Error] $system:$arch not support
	fi; echo
else echo
	echo [Error] $system not support
fi; echo
echo "**************** merge sec ota end ******************"