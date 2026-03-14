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
- `adc_clkin` (ONLY if we decide to drive `CLKIN` from the SoC)
- `adc_sync_reset` (SYNC/RESET function)

### Critical note: CLKIN is non-optional for real conversions
The ADS131M08 requires a **continuous, free-running master clock on `CLKIN`** for normal operation.

That means we must close one of these before tapeout:
- The harness/PCB provides an oscillator into `CLKIN`, **or**
- We explicitly route and drive `adc_clkin` from a known SoC clock output.

Tracking decision record (IP repo): `ip/home-inventory-chip/decisions/011-adc-clkin-source-and-frequency.md`.

## CLKIN decision evidence (must include in the lock PR)
When we claim “ADC clocking is confirmed”, we must be able to point to a *committed* source in this harness repo.

Fill this block in the PR description and paste it into this doc (and the IP repo decision record) when the decision is made:

```text
Decision: (A) Board oscillator into CLKIN  OR  (B) SoC drives adc_clkin
Source: (schematic page/link OR docs path + line range)
Expected CLKIN frequency (Hz):
Is CLKIN present immediately after reset? (yes/no/unknown)
If SoC-driven: which clock generator/source net?
Bring-up verification point: (test pad / scope point / observable behavior)
```

Fast audit command (from repo root):
```bash
rg -n "adc_clkin|ADC_CLKIN|\bCLKIN\b|oscillator|xtal|crystal|CLKOUT" docs verilog rtl src openlane
```

If the block above cannot be filled with evidence, keep `adc_clkin` marked **optional** and treat clocking as a tapeout blocker.


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

| Signal | Dir | Caravel GPIO | Default/safe state (during reset + until FW config) |
|---|---:|---:|---|
| `adc_sclk` | out | `io[??]` | drive low (no toggles) |
| `adc_cs_n` | out | `io[??]` | drive high (deassert) |
| `adc_mosi` | out | `io[??]` | drive low |
| `adc_miso` | in  | `io[??]` | input; ignore until streaming enabled |
| `adc_drdy_n` | in | `io[??]` | input with pull-up if available; treat as active-low |
| `adc_rst_n` | out | `io[??]` | drive low during reset, then high |
| `adc_clkin` *(optional)* | out | `io[??]` | if used: drive continuous clock; otherwise **do not route** |

Notes:
- `adc_cs_n` must not glitch low during reset (would cause unintended frames if `adc_sclk` is active).
- If we do **not** route `adc_clkin`, the harness/PCB **must** provide an oscillator into `CLKIN` (see note above).

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
