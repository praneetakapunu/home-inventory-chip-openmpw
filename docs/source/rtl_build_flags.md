# RTL build flags (compile-time defines)

This repo integrates the **source-of-truth IP** from the `ip/home-inventory-chip` submodule.
Some integration points are controlled by small, explicit **Verilog compile-time defines**.

The intent is:
- default build is safe + minimal (Wishbone-only, no external pads driven)
- optional flags let us compile/verify the real ADC ingest path early

## `USE_REAL_ADC_INGEST`
**Purpose:** Enable the real ADS131M08 SPI ingest path inside the IP (`adc_streaming_ingest`) and expose the SPI pins at the Wishbone block boundary.

### Effects (at `home_inventory_user_project`)
- When **not** defined (default):
  - the wrapper forces safe defaults for the ADC outputs:
    - `adc_sclk=0`, `adc_cs_n=1`, `adc_mosi=0`
  - the IP (`home_inventory_wb`) builds in **stub SNAPSHOT mode** (FIFO + deterministic ramp pattern)

- When **defined**:
  - the wrapper connects the IP's SPI pins:
    - `adc_sclk`, `adc_cs_n`, `adc_mosi`, `adc_miso`
  - firmware can use `CTRL.START` to request one captured ADC frame

### Notes
- This flag does **not** automatically drive Caravel IO pads; that is controlled separately by `HOMEINV_ENABLE_ADC_GPIO`.

## `HOMEINV_ENABLE_ADC_GPIO`
**Purpose:** Opt-in routing of the ADC interface to Caravel `io[*]` pads.

### Effects
- When **not** defined (default):
  - all `io_out` are 0
  - all pads are high-Z via `io_oeb=1`

- When **defined**:
  - selected pads are driven for outputs (`SCLK`, `CSN`, `MOSI`, `RSTN`)
  - selected pads are sampled for inputs (`MISO`, `DRDY_N`)

### IMPORTANT (placeholders)
The current `ADC_*_IO` indices in `verilog/rtl/home_inventory_user_project.v` are **placeholders**.
Do not tapeout with defaults.

Canonical mapping plan:
- `docs/source/adc_pinout_plan.md`
- (IP-side contract reference) `ip/home-inventory-chip/docs/ADC_PINOUT_CONTRACT.md`

## Recommended modes
- **Default / bring-up safe:** (no defines)
  - Wishbone regfile enumerates
  - ADC path is stubbed (SNAPSHOT works)
  - no pads are driven

- **Compile-check the real ADC path (still pad-safe):**
  - `USE_REAL_ADC_INGEST` only

- **Full pad routing (requires pin map lock):**
  - `HOMEINV_ENABLE_ADC_GPIO` (optionally + `USE_REAL_ADC_INGEST`)
