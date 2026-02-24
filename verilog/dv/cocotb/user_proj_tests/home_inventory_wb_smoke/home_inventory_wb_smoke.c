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
    // Keep these consistent with chip-inventory/spec/regmap_v1.yaml.
    const int OFF_ID              = 0x0000 / 4;
    const int OFF_VERSION         = 0x0004 / 4;

    const int OFF_CTRL            = 0x0100 / 4;
    const int OFF_IRQ_EN          = 0x0104 / 4;
    const int OFF_STATUS          = 0x0108 / 4;

    const int OFF_ADC_CFG         = 0x0200 / 4;
    const int OFF_ADC_CMD         = 0x0204 / 4;
    const int OFF_ADC_FIFO_STATUS = 0x0208 / 4;
    const int OFF_ADC_FIFO_DATA   = 0x020C / 4;
    const int OFF_ADC_RAW_CH0         = 0x0210 / 4;
    const int OFF_ADC_SNAPSHOT_COUNT  = 0x0230 / 4;

    const int OFF_TARE_CH0            = 0x0300 / 4;
    const int OFF_SCALE_CH0           = 0x0320 / 4;

    // --- Reset defaults (normative, from chip-inventory/spec/regmap_v1.yaml) ---
    if (((unsigned int)USER_readWord(OFF_CTRL)) != 0x00000000u) {
        fail(0x100);
    }
    if (((unsigned int)USER_readWord(OFF_IRQ_EN)) != 0x00000000u) {
        fail(0x101);
    }
    if (((unsigned int)USER_readWord(OFF_ADC_CFG)) != 0x00000000u) {
        fail(0x102);
    }
    if (((unsigned int)USER_readWord(OFF_TARE_CH0)) != 0x00000000u) {
        fail(0x103);
    }
    if (((unsigned int)USER_readWord(OFF_SCALE_CH0)) != 0x00010000u) {
        fail(0x104);
    }

    // --- Basic ID/version (read-only) ---
    const unsigned int id = (unsigned int)USER_readWord(OFF_ID);
    const unsigned int ver = (unsigned int)USER_readWord(OFF_VERSION);

    if (id != 0x48494348u) { // "HICH"
        fail(0x001);
    }
    if (ver != 0x00000001u) {
        fail(0x002);
    }

    // Attempt to clobber RO regs; reads must remain unchanged.
    USER_writeWord(0x00000000u, OFF_ID);
    USER_writeWord(0xFFFFFFFFu, OFF_VERSION);
    if (((unsigned int)USER_readWord(OFF_ID)) != id) {
        fail(0x003);
    }
    if (((unsigned int)USER_readWord(OFF_VERSION)) != ver) {
        fail(0x004);
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
    // Reserved bits should read as 0.
    if ((ctrl1 & 0xFFFFFFFEu) != 0u) {
        fail(0x023);
    }

    // Try writing garbage into reserved bits; should be ignored.
    USER_writeWord(0x80000003u, OFF_CTRL); // ENABLE=1, START=1, RESERVED=1
    const unsigned int ctrl2 = (unsigned int)USER_readWord(OFF_CTRL);
    if ((ctrl2 & 0x1u) != 0x1u) {
        fail(0x021);
    }
    if ((ctrl2 & 0x2u) != 0u) {
        fail(0x022);
    }
    if ((ctrl2 & 0xFFFFFFFEu) != 0u) {
        fail(0x024);
    }

    // --- IRQ_EN: only low 3 bits should stick ---
    USER_writeWord(0xFFFFFFFFu, OFF_IRQ_EN);
    const unsigned int irq_en = (unsigned int)USER_readWord(OFF_IRQ_EN);
    if ((irq_en & 0x7u) != 0x7u) {
        fail(0x030);
    }
    if ((irq_en & 0xFFFFFFF8u) != 0u) {
        fail(0x031);
    }

    // STATUS is RO; it should always have reserved bits == 0.
    const unsigned int status = (unsigned int)USER_readWord(OFF_STATUS);
    if ((status & 0xFFFFFF00u) != 0u) {
        fail(0x032);
    }

    // --- Calibration regs: TARE and SCALE should be full RW ---
    USER_writeWord(0xA5A55A5Au, OFF_TARE_CH0);
    if (((unsigned int)USER_readWord(OFF_TARE_CH0)) != 0xA5A55A5Au) {
        fail(0x040);
    }

    USER_writeWord(0x00020000u, OFF_SCALE_CH0); // Q16.16 = 2.0
    if (((unsigned int)USER_readWord(OFF_SCALE_CH0)) != 0x00020000u) {
        fail(0x041);
    }

    // --- Events regs (stubbed-by-snapshot): basic semantics ---
    const int OFF_EVT_COUNT_CH0      = 0x0400 / 4;
    const int OFF_EVT_LAST_DELTA_CH0 = 0x0420 / 4;
    const int OFF_EVT_LAST_TS        = 0x0440 / 4;
    const int OFF_EVT_CFG            = 0x0444 / 4;
    const int OFF_EVT_THRESH_CH0     = 0x0480 / 4;

    // Reset defaults
    if (((unsigned int)USER_readWord(OFF_EVT_CFG)) != 0x00000000u) {
        fail(0x060);
    }

    // Program threshold so our deterministic snapshot sample will always hit.
    USER_writeWord(0x00000000u, OFF_EVT_THRESH_CH0);

    // Enable CH0.
    USER_writeWord(0x00000001u, OFF_EVT_CFG);

    // First snapshot should fire an event: count increments, delta = 0.
    USER_writeWord(0x00000001u, OFF_ADC_CMD); // SNAPSHOT pulse

    const unsigned int evt_cnt1 = (unsigned int)USER_readWord(OFF_EVT_COUNT_CH0);
    const unsigned int evt_delta1 = (unsigned int)USER_readWord(OFF_EVT_LAST_DELTA_CH0);
    const unsigned int evt_last_ts1 = (unsigned int)USER_readWord(OFF_EVT_LAST_TS);

    if (evt_cnt1 != 1u) {
        fail(0x061);
    }
    if (evt_delta1 != 0u) {
        fail(0x062);
    }
    if (evt_last_ts1 == 0u) {
        // With our stub, ts increments starting at 1, so this should be nonzero.
        fail(0x063);
    }

    // Second snapshot should fire again: count increments, delta = 1 (ts_now increments by 1).
    USER_writeWord(0x00000001u, OFF_ADC_CMD); // SNAPSHOT pulse

    const unsigned int evt_cnt2 = (unsigned int)USER_readWord(OFF_EVT_COUNT_CH0);
    const unsigned int evt_delta2 = (unsigned int)USER_readWord(OFF_EVT_LAST_DELTA_CH0);
    const unsigned int evt_last_ts2 = (unsigned int)USER_readWord(OFF_EVT_LAST_TS);

    if (evt_cnt2 != 2u) {
        fail(0x064);
    }
    if (evt_delta2 != 1u) {
        fail(0x065);
    }
    if (evt_last_ts2 != (evt_last_ts1 + 1u)) {
        fail(0x066);
    }

    // Disable + re-enable should clear history so the next event has delta = 0.
    USER_writeWord(0x00000000u, OFF_EVT_CFG);
    USER_writeWord(0x00000001u, OFF_EVT_CFG);

    USER_writeWord(0x00000001u, OFF_ADC_CMD); // SNAPSHOT pulse

    const unsigned int evt_cnt3 = (unsigned int)USER_readWord(OFF_EVT_COUNT_CH0);
    const unsigned int evt_delta3 = (unsigned int)USER_readWord(OFF_EVT_LAST_DELTA_CH0);

    if (evt_cnt3 != 3u) {
        fail(0x067);
    }
    if (evt_delta3 != 0u) {
        fail(0x068);
    }

    // Regression: "glitchy" enable pulse should NOT clear history if we never
    // take a sample while enabled.
    // Sequence: briefly raise EVT_EN then disable again before snapshot.
    const unsigned int evt_last_ts3 = (unsigned int)USER_readWord(OFF_EVT_LAST_TS);

    USER_writeWord(0x00000000u, OFF_EVT_CFG);
    USER_writeWord(0x00000001u, OFF_EVT_CFG); // 0->1 edge (pending)
    USER_writeWord(0x00000000u, OFF_EVT_CFG); // back to disabled before sample

    USER_writeWord(0x00000001u, OFF_ADC_CMD); // SNAPSHOT pulse (while disabled)

    const unsigned int evt_cnt_glitch = (unsigned int)USER_readWord(OFF_EVT_COUNT_CH0);
    const unsigned int evt_delta_glitch = (unsigned int)USER_readWord(OFF_EVT_LAST_DELTA_CH0);
    const unsigned int evt_last_ts_glitch = (unsigned int)USER_readWord(OFF_EVT_LAST_TS);

    if (evt_cnt_glitch != evt_cnt3) {
        fail(0x069);
    }
    if (evt_delta_glitch != evt_delta3) {
        fail(0x06A);
    }
    if (evt_last_ts_glitch != evt_last_ts3) {
        fail(0x06B);
    }

    // True re-enable + snapshot should clear history: delta back to 0.
    USER_writeWord(0x00000001u, OFF_EVT_CFG);
    USER_writeWord(0x00000001u, OFF_ADC_CMD); // SNAPSHOT pulse

    const unsigned int evt_cnt4 = (unsigned int)USER_readWord(OFF_EVT_COUNT_CH0);
    const unsigned int evt_delta4 = (unsigned int)USER_readWord(OFF_EVT_LAST_DELTA_CH0);

    if (evt_cnt4 != (evt_cnt3 + 1u)) {
        fail(0x06C);
    }
    if (evt_delta4 != 0u) {
        fail(0x06D);
    }

    // --- ADC snapshot path (stub) + FIFO pop behavior ---
    const unsigned int snap_cnt0 = (unsigned int)USER_readWord(OFF_ADC_SNAPSHOT_COUNT);
    const unsigned int raw0_before = (unsigned int)USER_readWord(OFF_ADC_RAW_CH0);

    USER_writeWord(0x00000001u, OFF_ADC_CMD); // SNAPSHOT pulse

    const unsigned int snap_cnt1 = (unsigned int)USER_readWord(OFF_ADC_SNAPSHOT_COUNT);
    const unsigned int raw0_after = (unsigned int)USER_readWord(OFF_ADC_RAW_CH0);

    // Snapshot should update raw regs (at least CH0) and increment the counter.
    if (raw0_after == raw0_before) {
        fail(0x050);
    }
    if (snap_cnt1 != (snap_cnt0 + 1u)) {
        fail(0x053);
    }

    const unsigned int fifo_status0 = (unsigned int)USER_readWord(OFF_ADC_FIFO_STATUS);
    const unsigned int level0 = (fifo_status0 & 0xFFFFu);
    if (level0 == 0u) {
        fail(0x051);
    }

    // Pop one word and confirm level decrements by 1.
    (void)USER_readWord(OFF_ADC_FIFO_DATA);
    const unsigned int fifo_status1 = (unsigned int)USER_readWord(OFF_ADC_FIFO_STATUS);
    const unsigned int level1 = (fifo_status1 & 0xFFFFu);
    if (level1 != (level0 - 1u)) {
        fail(0x052);
    }

    // PASS: set user GPIO0 high and signal completion.
    set_gpio_user_l(0x00000001u);
    ManagmentGpio_write(1);

    return;
}
