/********************************************************************************************************
 * @file    pmp.h
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
#ifndef __PMP_H__
#define __PMP_H__

/* Check platform support which PMP scheme!
 * CLIC platform only support NAPOT scheme! */

#define SCHEME_NAPOT                            3
#define SCHEME_NA4                              2
#define SCHEME_TOR                              1
#define SCHEME_OFF                              0

#define NAPOT(base, size)                       (unsigned long)(((size) > 0) ? ((((unsigned long)(base) & (~((unsigned long)(size) - 1))) >> 2) | (((unsigned long)(size) - 1) >> 3)) : 0)
#define TOR(top)                                (unsigned long)((unsigned long)(top) >> 2)

/*  PMP Configuration Options
 *  *     7       6       5       4       3       2       1       0
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 * |   L   |       0       |       A       |   X   |   W   |   R   | pmp#cfg
 * +-------+-------+-------+-------+-------+-------+-------+-------+
 */
#define PMP_L_OFF                               0
#define PMP_L_ON                                1
#define PMP_A_OFF                               0
#define PMP_A_NA4                               SCHEME_NA4
#define PMP_A_TOR                               SCHEME_TOR
#define PMP_A_NAPOT                             SCHEME_NAPOT
#define PMP_X_OFF                               0
#define PMP_X_ON                                1
#define PMP_W_OFF                               0
#define PMP_W_ON                                1
#define PMP_R_OFF                               0
#define PMP_R_ON                                1

/**
   PMPCFG_LAXWR(l, a, x, w, r) is a macro used to generate the configuration value
   for a Physical Memory Protection (PMP) entry. It combines lock (l), address
   matching (a), execute (x), write (w), and read (r) permissions into a single
   configuration value to be written into a PMP configuration register.
 */
#define PMPCFG_LAXWR(l,a,x,w,r)                 (((l) << 7) | ((a) << 3) | ((x) << 2) | ((w) << 1) | ((r) << 0))

/* Machine CSR */
#define MSTATUS_MPP_MSK                         ((1ULL << (11)) | (1ULL << (12)))
/* Hart mode for machine/supervisor/user */
#define MSTATUS_MPP_USER                        (0ULL << (11))
#define MSTATUS_MPP_SV                          (1ULL << (11))
#define MSTATUS_MPP_MC                          ((1ULL << (11)) | (1ULL << (12)))

/* Supervisor CSR */
#define SSTATUS_SPP_MSK                         (1ULL << (8))

/* Hart mode for machine/supervisor/user */
#define HART_USER                               0x0
#define HART_SV                                 0x1
#define HART_MC                                 0x3

/* Use EILM-1KB/EDLM-128KB of CF1-AE250 to fit settings of all boards */
#define EILM_SIZE                               0x20000
#define EDLM_SIZE                               0x20000

#define KB                                      (0x1 << 10)
#endif // __PMP_H__
