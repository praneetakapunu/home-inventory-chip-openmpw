# CI Strategy (Harness)

This repo is the OpenMPW/Caravel submission harness.

## Philosophy
- Keep **lightweight CI green** on every push/PR.
- Run **heavy OpenMPW/OpenLane/precheck** only when manually triggered, because it is slow and disk hungry.

## Always-on CI (default)
- `make rtl-compile-check`
  - installs iverilog
  - syncs the RTL filelist from the IP submodule
  - compiles the wrapper + user project RTL (default/stub mode)

- `make rtl-compile-check-real-adc`
  - same as above, but defines `USE_REAL_ADC_INGEST`
  - catches wrapper/IP drift in the real-ADC wiring without running full DV

## Heavy CI (manual)
Hardening / PDK downloads / OpenLane / mpw-precheck are disabled by default and should be executed only when:
- we have a machine/runner with sufficient disk (recommend **>= 30 GB free**), and
- we are at a submission checkpoint.

## Source of truth
- IP/spec repo: https://github.com/praneetakapunu/home-inventory-chip
