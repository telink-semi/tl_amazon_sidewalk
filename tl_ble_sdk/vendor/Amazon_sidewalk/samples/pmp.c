/********************************************************************************************************
 * @file    pmp.c
 *
 * @brief   This is the source file for B91m
 *
 * @author  Driver Group
 * @date    2023
 *
 * @par     Copyright (c) 2023, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#include "lib/include/core.h"
#include "pmp.h"

//#include <syslog.h>

/**
 * @brief      This function serves to configure a PMP entry using the TOR (Top of Region) scheme.
 * @param[in]  entry    PMP entry number (0-15) to configure.
 * @param[in]  va       Virtual address representing the start of the address range.
 * @param[in]  pmpcfg   Configuration value for the PMP entry.
 * @return     None
 */
void pmp_tor_config(char entry, void* va, char pmpcfg)
{
    switch (entry) {
    case 0:
        write_csr(NDS_PMPADDR0, TOR(va));
        break;
    case 1:
        write_csr(NDS_PMPADDR1, TOR(va));
        break;
    case 2:
        write_csr(NDS_PMPADDR2, TOR(va));
        break;
    case 3:
        write_csr(NDS_PMPADDR3, TOR(va));
        break;
    }

    switch (entry >> 2) {
    case 0:
        write_csr(NDS_PMPCFG0, ((read_csr(NDS_PMPCFG0) & (~((0xFF) << ((entry%4) << 3)))) | (((long)pmpcfg) << ((entry%4) << 3))));
        break;
    }
}

/**
 * @brief   This function serves to initialize PMP (Physical Memory Protection) configuration.
 * @return  None
 */
void ep2t5x_init_pmp_config(void * startAddr)
{
#if 0 /* machine mode and USE_TOR. Note: 721x: 8byte align for pmp TOR. 321x: 4byte or 8byte ? */
    extern unsigned int *_DLM_CODE_VMA_START;
    extern unsigned int *_DLM_CODE_VMA_END;
    //syslog(1, "pmconfig:%p,0x%p", (void*)(&_DLM_CODE_VMA_START), (void*)(&_DLM_CODE_VMA_END));

    /* PMP entry 0 : 0~_DATA_VMA_START(not include)*/
    pmp_tor_config(0, (void*)(&_DLM_CODE_VMA_START), PMPCFG_LAXWR(PMP_L_ON, PMP_A_TOR, PMP_X_ON, PMP_W_ON, PMP_R_ON));

    /* PMP entry 1 : _DATA_VMA_START ~ _DLM_CODE_VMA_END(not include)*/
    pmp_tor_config(1, (void*)(&_DLM_CODE_VMA_END), PMPCFG_LAXWR(PMP_L_ON, PMP_A_TOR, PMP_X_ON, PMP_W_OFF, PMP_R_ON));

    //write_sram32(0x80000, 0x1122334455); // test code
#else /* debug global variable. Note: 721x: 8byte align for pmp TOR. 321x: 4byte or 8byte ? */

    /* PMP entry 0 : g_irqvector */
    pmp_tor_config(0, (void*)(((unsigned int)startAddr)), PMPCFG_LAXWR(PMP_L_ON, PMP_A_TOR, PMP_X_ON, PMP_W_ON, PMP_R_ON));
    pmp_tor_config(1, (void*)(((unsigned int)startAddr) + 4), PMPCFG_LAXWR(PMP_L_ON, PMP_A_TOR, PMP_X_ON, PMP_W_OFF, PMP_R_ON));

    //g_irqvector[0].arg // = (void *)0x9999;
#endif
}

