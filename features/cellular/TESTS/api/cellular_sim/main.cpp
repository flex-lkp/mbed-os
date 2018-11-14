/*
 * Copyright (c) 2018, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#if !defined(MBED_CONF_NSAPI_PRESENT)
#error [NOT_SUPPORTED] A json configuration file is needed. Skipping this build.
#endif

#include "CellularUtil.h" // for CELLULAR_ helper macros
#include "CellularTargets.h"

#ifndef CELLULAR_DEVICE
#error [NOT_SUPPORTED] CELLULAR_DEVICE must be defined
#endif

#ifndef MBED_CONF_APP_CELLULAR_SIM_PIN
#error [NOT_SUPPORTED] SIM pin code is needed. Skipping this build.
#endif

#include "greentea-client/test_env.h"
#include "unity.h"
#include "utest.h"

#include "mbed.h"

#include "CellularContext.h"
#include "CellularDevice.h"
#include "CellularSIM.h"
#include "../../cellular_tests_common.h"
#include CELLULAR_STRINGIFY(CELLULAR_DEVICE.h)

#define NETWORK_TIMEOUT (180*1000)

static CellularContext *ctx;
static CellularDevice *device;

static void init_to_device_ready_state()
{
    ctx = CellularContext::get_default_instance();
    TEST_ASSERT(ctx != NULL);
    TEST_ASSERT(ctx->set_device_ready() == NSAPI_ERROR_OK);

    device = CellularDevice::get_default_instance();
    TEST_ASSERT(device != NULL);
}

static void test_sim_interface()
{
    CellularSIM *sim = device->open_sim();
    TEST_ASSERT(sim != NULL);

    // set SIM at time out to 9000
    device->set_timeout(9000);
    wait(4); // we need to wait for some time so that SIM interface is working in all modules.
    // 1. test set_pin
    nsapi_error_t err = sim->set_pin(MBED_CONF_APP_CELLULAR_SIM_PIN);
    MBED_ASSERT(err == NSAPI_ERROR_OK);

   // 2. test set_pin_query
    wait(1);
    err = sim->set_pin_query(MBED_CONF_APP_CELLULAR_SIM_PIN, false);
    TEST_ASSERT(err == NSAPI_ERROR_OK || err == NSAPI_ERROR_UNSUPPORTED);

    wait(1);
    err = sim->set_pin_query(MBED_CONF_APP_CELLULAR_SIM_PIN, true);
    TEST_ASSERT(err == NSAPI_ERROR_OK || err == NSAPI_ERROR_UNSUPPORTED);

    wait(1);
    // 3. test get_sim_state
    CellularSIM::SimState state;
    err = sim->get_sim_state(state);
    TEST_ASSERT(err == NSAPI_ERROR_OK);
    TEST_ASSERT(state == CellularSIM::SimStateReady);

    wait(1);
    // 4. test get_imsi
    char imsi[16] = {0};
    err = sim->get_imsi(imsi);
    TEST_ASSERT(err == NSAPI_ERROR_OK);
    TEST_ASSERT(strlen(imsi) > 0);
}

using namespace utest::v1;

static utest::v1::status_t greentea_failure_handler(const Case *const source, const failure_t reason)
{
    greentea_case_failure_abort_handler(source, reason);
    return STATUS_ABORT;
}

static Case cases[] = {
    Case("CellularSIM init", init_to_device_ready_state, greentea_failure_handler),
    Case("CellularSIM test interface", test_sim_interface, greentea_failure_handler)
};

static utest::v1::status_t test_setup(const size_t number_of_cases)
{
    GREENTEA_SETUP(10 * 60, "default_auto");
    return verbose_test_setup_handler(number_of_cases);
}

static Specification specification(test_setup, cases);

int main()
{
#if MBED_CONF_MBED_TRACE_ENABLE
    trace_open();
#endif
    int ret = Harness::run(specification);
#if MBED_CONF_MBED_TRACE_ENABLE
    trace_close();
#endif
    return ret;
}
