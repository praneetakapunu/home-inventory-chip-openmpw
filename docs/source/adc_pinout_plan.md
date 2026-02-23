# ADC (ADS131M08) GPIO Pinout Plan — v1 (DRAFT)

This document is the **actionable plan** to assign Caravel GPIO pads for the external ADC interface.

It exists to unblock tapeout integration work *before* the ADC core RTL is fully wired, while keeping the final mapping explicit and reviewable.

Related:
- Harness pinout contract: `docs/source/pinout.md`
- ADC interface assumptions (IP repo): `ip/home-inventory-chip/spec/ads131m08_interface.md`
- Clocking unknowns (IP repo): `ip/home-inventory-chip/docs/ADC_CLOCKING_PLAN.md`

## Interface signals we must route
Minimum viable external interface for ADS131M08 data capture:

**SoC → ADC (outputs)**
- `adc_sclk`
- `adc_cs_n`
- `adc_mosi` (DIN)
- `adc_rst_n` (recommended)

**ADC → SoC (inputs)**
- `adc_miso` (DOUT)
- `adc_drdy_n` (DRDY asserts low)

Optional / future:
- `adc_clkin` (if we decide to drive CLKIN from SoC)
- `adc_sync_reset` (SYNC/RESET function)

## Constraints to verify (before we lock pin numbers)
1) Which `io[*]` pads are available on the OpenMPW harness for user projects.
2) Whether any pads are pre-used by:
   - flash / housekeeping
   - management SPI
   - reserved analog pads
3) Voltage domain / pad type constraints for the chosen pads.
4) Whether we need a dedicated, low-skew pad for `adc_sclk`.

## Proposed mapping (placeholders; DO NOT TAPEOUT AS-IS)
We will lock exact pad indices only after confirming harness constraints.

- `adc_sclk`   → `io[??]` (output)
- `adc_cs_n`   → `io[??]` (output)
- `adc_mosi`   → `io[??]` (output)
- `adc_miso`   → `io[??]` (input)
- `adc_drdy_n` → `io[??]` (input)
- `adc_rst_n`  → `io[??]` (output)

## Lock procedure (what “done” means)
When we lock the mapping, we must do all of the following in the same PR (or tightly coupled PRs):

1) Update `docs/source/pinout.md` with the final `io[*]` assignments.
2) Update `verilog/rtl/home_inventory_user_project.v` to:
   - define named wires for these signals
   - connect them to the chosen `io_in/io_out/io_oeb` bits
   - keep safe defaults during reset
3) Add a small directed sim or lint-style check (even a simple `iverilog` compile) that proves the wrapper still builds.
4) Add/Update a decision record in the IP repo (`chip-inventory/decisions/`) linking to this mapping.

## Open questions
- Clocking plan: will the board provide `CLKIN`, or do we need to synthesize/route one from Caravel?
- Do we want to reserve 1–2 extra GPIOs for debug (e.g., “frame_seen” pulse)?
