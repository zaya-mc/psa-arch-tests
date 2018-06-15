/** @file
 * Copyright (c) 2018, Arm Limited or its affiliates. All rights reserved.
 * SPDX-License-Identifier : Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#include "val_test_common.h"

/**
  Publish these functions to the external world as associated to this test ID
**/
TBSA_TEST_PUBLISH(CREATE_TEST_ID(TBSA_VERSION_COUNTERS_BASE, 1),
                  CREATE_TEST_TITLE("Check version counter functionality"),
                  CREATE_REF_TAG("R010/R020/R030/R040/R050/R060_TBSA_COUNT"),
                  entry_hook,
                  test_payload,
                  exit_hook);

tbsa_val_api_t        *g_val;

void
secure_fault_esr (unsigned long *sf_args)
{
    if (IS_TEST_PENDING(g_val->get_status())) {
        g_val->set_status(RESULT_PASS(TBSA_STATUS_SUCCESS));
    } else {
        g_val->set_status(RESULT_FAIL(TBSA_STATUS_INVALID));
    }

    /* Updating the return address in the stack frame in order to avoid periodic fault */
    sf_args[6] = sf_args[6] + 4;
}

__attribute__((naked))
void
SF_Handler(void)
{
    asm volatile("mrs r0, control_ns \n"
                 "mov r1, #0x2       \n"
                 "and r0, r1         \n"
                 "cmp r0, r1         \n"
                 "beq _psp_ns        \n"
                 "mrs r0, msp_ns     \n"
                 "b secure_fault_esr \n"
                 "_psp_ns:           \n"
                 "mrs r0, psp_ns     \n"
                 "b secure_fault_esr \n");
}

void
setup_ns_env(void)
{
    tbsa_status_t status;

    /* Installing Trusted Fault Handler for NS test */
    status = g_val->interrupt_setup_handler(EXCP_NUM_SF, 0, SF_Handler);
    g_val->err_check_set(TEST_CHECKPOINT_1, status);
}

void entry_hook(tbsa_val_api_t *val)
{
    tbsa_test_init_t init = {
                             .bss_start      = &__tbsa_test_bss_start__,
                             .bss_end        = &__tbsa_test_bss_end__
                            };

    val->test_initialize(&init);
    g_val = val;
    val->set_status(RESULT_PASS(TBSA_STATUS_SUCCESS));
}

void test_payload(tbsa_val_api_t *val)
{
    /* setup environment for NS test */
    setup_ns_env();

    val->set_status(RESULT_PASS(TBSA_STATUS_SUCCESS));
}

void exit_hook(tbsa_val_api_t *val)
{
}