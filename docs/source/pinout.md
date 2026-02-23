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
Currently **unused** by v1.

- `io_out` = 0
- `io_oeb` = 1 (all GPIOs high-Z)
- `analog_io` not used

### user_clock2
Currently **unused** by v1.

### user_irq
Currently **unused** by v1.

- `user_irq` = 0

## Notes

- The integrated module is `home_inventory_user_project` (see `verilog/rtl/home_inventory_user_project.v`).
- The IP register block is `home_inventory_wb` from the submodule filelist `ip/home-inventory-chip/rtl/ip_home_inventory.f`.
- When we start using GPIOs (SPI to external ADC, etc.), this doc must be updated *before* hardening/precheck.
