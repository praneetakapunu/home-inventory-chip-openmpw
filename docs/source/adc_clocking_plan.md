# ADC clocking plan (ADS131M08) — harness contract (v1)

This document lives in the **harness repo** because the ADS131M08 clock source is a *board/harness decision*.

It exists to prevent a recurring failure mode: RTL + firmware look “fine” in simulation, but bring-up fails because **CLKIN is missing or has the wrong frequency**.

Related:
- GPIO mapping plan: `docs/source/adc_pinout_plan.md`
- Overall wrapper pinout contract: `docs/source/pinout.md`
- Chip/IP-side clocking notes: `ip/home-inventory-chip/docs/ADC_CLOCKING_PLAN.md`
- Decision record (IP repo): `ip/home-inventory-chip/decisions/011-adc-clkin-source-and-frequency.md`

## Non-negotiable requirement (datasheet)
ADS131M08 requires a **continuous, free-running master clock** on `CLKIN` for normal conversions.

If `CLKIN` is not toggling, `DRDY` behavior and conversion data will not be meaningful. Do **not** debug RTL first if the clock is absent.

## Allowed v1 options (pick exactly one)
### Option A — Board oscillator drives ADC `CLKIN` (preferred)
- The harness/PCB provides an oscillator into the ADC `CLKIN` pin.
- SoC does **not** route `adc_clkin`.

### Option B — SoC drives `adc_clkin` into ADC `CLKIN`
- We route a dedicated GPIO (`adc_clkin`) from Caravel to the ADC `CLKIN` pin.
- The SoC must provide a *known, stable* clock on that pad early enough for bring-up.

## What “confirmed” means (evidence checklist)
We only claim clocking is confirmed when the following fields are filled with **committed evidence** in this harness repo:

```text
Decision: (A) Board oscillator into CLKIN  OR  (B) SoC drives adc_clkin
Source: (schematic page/link OR committed docs path + line range)
Expected CLKIN frequency (Hz):
Is CLKIN present immediately after reset? (yes/no/unknown)
If SoC-driven: which clock generator/source net?
Bring-up verification point: (test pad / scope point / observable behavior)
```

After the harness decision is made, copy the same information into:
- `ip/home-inventory-chip/decisions/011-adc-clkin-source-and-frequency.md`

## Practical bring-up check (scope-first)
At first hardware bring-up (before trusting any samples):
1) Scope/measure `CLKIN` at the ADC pin (or at a known test point).
2) Confirm the frequency matches the expected value (Hz).
3) Only then: verify `DRDY` toggles after conversions start.

## Repo-local audit helper
To quickly locate any existing clocking breadcrumbs (and prevent assumptions from drifting):

```bash
# from the IP repo:
#   tools/harness_adc_clocking_audit.sh ../home-inventory-chip-openmpw

# from this harness repo:
rg -n "adc_clkin|ADC_CLKIN|\\bCLKIN\\b|oscillator|xtal|crystal|CLKOUT|clkout" docs verilog rtl src openlane
```

## Status (fill in when locked)
- Decision: TBD
- Expected CLKIN frequency (Hz): TBD
- Evidence source: TBD
