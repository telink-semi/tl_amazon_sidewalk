echo "************************* start of post build *************************"

os=$(uname)
echo "OS from uname is: ${os}"

print_section_info() {
    echo ""
    echo "==================== Memory Sections Summary ===================="
    echo ""
    ram_data_ble_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.ram_data_ble" | awk '{sum += $2} END {print sum}')
    ram_code_ble_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.ram_code_ble" | awk '{sum += $2} END {print sum}')
    retention_reset_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.retention_reset" | awk '{sum += $2} END {print sum}')
    retention_data_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.retention_data" | awk '{sum += $2} END {print sum}')
    ram_code_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.ram_code\b" | awk '{sum += $2} END {print sum}')
    iram_noinit_data_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.iram_noinit_data" | awk '{sum += $2} END {print sum}')
    iram_bss_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.iram_bss" | awk '{sum += $2} END {print sum}')

    data_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.data" | awk '{sum += $2} END {print sum}')
    sbss_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.sbss" | awk '{sum += $2} END {print sum}')
    bss_size=$(riscv32-elf-size -A $1.elf | grep -E "^\.bss" | awk '{sum += $2} END {print sum}')
    
    ram_data_ble_size=${ram_data_ble_size:-0}
    ram_code_ble_size=${ram_code_ble_size:-0}
    retention_reset_size=${retention_reset_size:-0}
    retention_data_size=${retention_data_size:-0}
    ram_code_size=${ram_code_size:-0}
    iram_noinit_data_size=${iram_noinit_data_size:-0}
    iram_bss_size=${iram_bss_size:-0}
    data_size=${data_size:-0}
    sbss_size=${sbss_size:-0}
    bss_size=${bss_size:-0}

    retention_total=$((ram_data_ble_size+ram_code_ble_size+retention_reset_size + retention_data_size + ram_code_size + iram_noinit_data_size + iram_bss_size))
    data_total=$((data_size + sbss_size + bss_size))

    echo "--- Retention Sections (IRAM) ---"
    printf "  ram_data_ble:       %8d bytes\n" "$ram_data_ble_size"
    printf "  ram_code_ble:       %8d bytes\n" "$ram_code_ble_size"
    printf "  retention_reset:    %8d bytes\n" "$retention_reset_size"
    printf "  retention_data:     %8d bytes\n" "$retention_data_size"
    printf "  ram_code:           %8d bytes\n" "$ram_code_size"
    printf "  iram_noinit_data:   %8d bytes\n" "$iram_noinit_data_size"
    printf "  iram_bss:           %8d bytes\n" "$iram_bss_size"
    echo "  --------------------------------------------------------"
    printf "  IRAM Total:         %8d bytes (%.2f KB)\n" "$retention_total" "$(awk "BEGIN {printf \"%.2f\", $retention_total/1024}")"
    echo ""

    echo "--- Data Sections (DRAM) ---"
    printf "  data:               %8d bytes\n" "$data_size"
    printf "  sbss:               %8d bytes\n" "$sbss_size"
    printf "  bss:                %8d bytes\n" "$bss_size"
    echo "  --------------------------------------------------------"
    printf "  DRAM Total:         %8d bytes (%.2f KB)\n" "$data_total" "$(awk "BEGIN {printf \"%.2f\", $data_total/1024}")"
    echo ""
}

if [ "${os}" = "Linux" ] ; then
    echo "Linux OS"
    echo "check_fw in Linux..."
    chmod 755 ../../../check_fw
    ../../../check_fw $1.bin
    print_section_info $1
else
    echo "Windows OS"
    echo "check_fw in Windows..."
    ../../../tl_check_fw2.exe  $1.bin
    print_section_info $1
fi

echo  "-------------------- SDK version info --------------------"
str=$(grep -E "[\$]{3}[a-zA-Z0-9 _.]+[\$]{3}" --text -o $1.bin | sed 's/\$//g')
if [ -z "$str" ]; 
    then echo "no SDK version found at the end of firmware, please check sdk_version.c and sdk_version.h"
else 
    echo "$str";
fi
echo  "-------------------- SDK version end  --------------------"

if [ "${os}" = "Linux" ] ; then
    filesize=$(stat --format=%s $1.bin)
else
    filesize=$(stat -c %s $1.bin 2>/dev/null || wc -c < $1.bin)
fi
printf "  File size:         %8d bytes (%.2f KB)\n" "$filesize" "$(awk "BEGIN {printf \"%.2f\", $filesize/1024}")"
if [ $filesize -gt 262144 ] ; then
    echo "bin size is greater than 256KB, please refer to handbook!"
fi

echo "this is post build!! current configure is :$1"
echo  "-------------------- SDK version end  --------------------"


echo "************************** end of post build **************************"
echo "this is post build!! current configure is :$1"

