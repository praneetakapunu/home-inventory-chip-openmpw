# Home Inventory user_project_wrapper pinout (v1)

This repo is the **OpenMPW/Caravel harness**. The actual IP RTL lives in the submodule:

- `ip/home-inventory-chip/`

This document is the *human-reviewed contract* for which Caravel interfaces are used by v1.

## What v1 uses

### Wishbone (WB MI A)
Used for register reads/writes.

- `wb_clk_i`
- `wb_rst_i`
- `wbs_*` (stb/cyc/we/sel/dat_i/adr_i → ack/dat_o)

### Logic Analyzer (LA)
Currently **not driven** by the design.

- `la_data_out` is tied to `0`
- `la_data_in/la_oenb` are ignored

### GPIO / analog IO
Currently **unused** by v1 (all GPIOs high-Z).

However, v1 will eventually need GPIO routing for the external ADS131M08 ADC (SPI + DRDY + reset).

Where the plan lives:
- `docs/source/adc_pinout_plan.md`

Current wrapper behavior (`verilog/rtl/home_inventory_user_project.v`):
- Default (no special defines):
  - `io_out` = 0
  - `io_oeb` = 1 (all GPIOs high-Z)
  - `analog_io` not used
- Optional (compile-time) ADC GPIO routing:
  - Guard: `HOMEINV_ENABLE_ADC_GPIO`
  - The wrapper exposes *parameterized* `io[*]` indices (`ADC_SCLK_IO`, `ADC_CSN_IO`, ...)
  - **Those default indices are placeholders** and must be explicitly locked/updated before any tapeout build enables this path.

### user_clock2
Currently **unused** by v1.

### user_irq
Currently **unused** by v1.

- `user_irq` = 0

## Notes

- The integrated module is `home_inventory_user_project` (see `verilog/rtl/home_inventory_user_project.v`).
- The IP register block is `home_inventory_wb` from the submodule filelist `ip/home-inventory-chip/rtl/ip_home_inventory.f`.
- When we start using GPIOs (SPI to external ADC, etc.), this doc must be updated *before* hardening/precheck.
