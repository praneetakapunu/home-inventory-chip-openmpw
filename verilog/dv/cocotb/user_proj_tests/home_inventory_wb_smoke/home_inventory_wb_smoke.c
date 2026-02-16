// SPDX-FileCopyrightText: 2026
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <firmware_apis.h>

// Smoke test for the home-inventory Wishbone register block.
//
// This runs on the Caravel management core and uses the USER_* APIs
// (Wishbone-to-user-project bridge) to poke registers.
//
// Pass/Fail signaling:
// - user GPIO[0] = 1 on pass, 0 on fail
// - management GPIO = 1 when test is done (regardless of pass/fail)

static void fail(unsigned int code) {
    // Put a small signature on user GPIOs for debug (low bits).
    // (Best-effort: not all pads may be observed in all sims.)
    set_gpio_user_l(0xBAD00000u | (code & 0xFFFFu));
    // Explicit fail flag on GPIO0.
    set_gpio_user_l(get_gpio_user_l() & ~0x1u);
    ManagmentGpio_write(1);
    while (1) { /* stop */ }
}

void main() {
    ManagmentGpio_outputEnable();
    ManagmentGpio_write(0);

    enableHkSpi(0); // disable housekeeping SPI to free GPIOs

    // Configure GPIOs for user project observation.
    GPIOs_configureAll(GPIO_MODE_USER_STD_OUT_MONITORED);
    GPIOs_loadConfigs();

    // Enable Wishbone bridge to user project.
    User_enableIF();

    // --- Register offsets (word addressed) ---
    // See chip-inventory/spec/regmap.md
    const int OFF_ID       = 0x0000 / 4;
    const int OFF_VERSION  = 0x0004 / 4;
    const int OFF_CTRL     = 0x0100 / 4;
    const int OFF_ADC_CFG  = 0x0200 / 4;

    // --- Basic ID/version ---
    const unsigned int id = (unsigned int)USER_readWord(OFF_ID);
    const unsigned int ver = (unsigned int)USER_readWord(OFF_VERSION);

    if (id != 0x48494348u) { // "HICH"
        fail(0x001);
    }
    if (ver != 0x00000001u) {
        fail(0x002);
    }

    // --- RW behavior: ADC_CFG lower bits should store, reserved bits zero ---
    USER_writeWord(0x00000005u, OFF_ADC_CFG); // NUM_CH = 5
    const unsigned int adc_cfg = (unsigned int)USER_readWord(OFF_ADC_CFG);
    if ((adc_cfg & 0xFu) != 0x5u) {
        fail(0x010);
    }
    if ((adc_cfg & 0xFFFFFFF0u) != 0u) {
        fail(0x011);
    }

    // --- CTRL: ENABLE should stick; START is write-1-to-pulse (reads as 0) ---
    USER_writeWord(0x00000001u, OFF_CTRL); // ENABLE=1
    const unsigned int ctrl1 = (unsigned int)USER_readWord(OFF_CTRL);
    if ((ctrl1 & 0x1u) != 0x1u) {
        fail(0x020);
    }

    USER_writeWord(0x00000003u, OFF_CTRL); // ENABLE=1, START=1
    const unsigned int ctrl2 = (unsigned int)USER_readWord(OFF_CTRL);
    if ((ctrl2 & 0x1u) != 0x1u) {
        fail(0x021);
    }
    if ((ctrl2 & 0x2u) != 0u) {
        fail(0x022);
    }

    // PASS: set user GPIO0 high and signal completion.
    set_gpio_user_l(0x00000001u);
    ManagmentGpio_write(1);

    return;
}
