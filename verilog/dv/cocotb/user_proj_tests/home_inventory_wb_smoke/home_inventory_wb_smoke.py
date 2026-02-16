# SPDX-FileCopyrightText: 2026
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

import cocotb
from caravel_cocotb.caravel_interfaces import test_configure
from caravel_cocotb.caravel_interfaces import report_test


@cocotb.test()
@report_test
async def home_inventory_wb_smoke(dut):
    """Wishbone smoke test for home-inventory regblock.

    Firmware does the actual Wishbone reads/writes and reports pass/fail via:
      - mgmt_gpio == 1 when done
      - user GPIO[0] == 1 on pass
    """

    caravelEnv = await test_configure(dut, timeout_cycles=40000)

    cocotb.log.info("[TEST] Start home_inventory_wb_smoke")

    await caravelEnv.release_csb()
    await caravelEnv.wait_mgmt_gpio(1)

    # GPIO0 should be driven high by firmware on PASS.
    pass_flag = int(caravelEnv.monitor_gpio(0).value)
    assert pass_flag == 1, "home_inventory_wb_smoke failed (GPIO0 != 1)"
