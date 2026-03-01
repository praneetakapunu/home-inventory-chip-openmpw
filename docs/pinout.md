# Home Inventory OpenMPW Harness Pinout (v1)

This file is the **top-level, human-reviewed pinout contract** for the harness repo.

Canonical (Sphinx) source:
- `docs/source/pinout.md`

If you are reviewing tapeout readiness, start here, then follow the links into:
- `docs/source/adc_pinout_plan.md` (ADC GPIO plan)
- `verilog/rtl/home_inventory_user_project.v` (actual wrapper wiring)

---

## v1: interfaces used

### Wishbone (WB MI A)
Used for all register reads/writes.

### Logic Analyzer (LA)
Not driven by v1 (tied off).

### GPIO (mprj_io)
Default: **all high-Z**.

Optional (compile-time): ADC GPIO routing can be enabled behind `HOMEINV_ENABLE_ADC_GPIO`, but **must not be enabled for tapeout** until the `io[*]` indices are explicitly locked in `docs/source/adc_pinout_plan.md` and updated in the wrapper.

### user_irq
Unused (tied to 0).

---

## Review checklist

Before hardening / precheck:
1) `docs/source/adc_pinout_plan.md` has **final** `io[*]` assignments (no `??`).
2) `verilog/rtl/home_inventory_user_project.v` matches those assignments.
3) Wrapper builds (at least `make rtl-compile-check`).
