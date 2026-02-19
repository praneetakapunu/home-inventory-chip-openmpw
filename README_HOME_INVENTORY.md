# Home Inventory Chip — OpenMPW Submission Harness

This repository is the **OpenMPW / Caravel user-project harness** for the project.

## Source of truth
Design/spec decisions live here:
- https://github.com/praneetakapunu/home-inventory-chip

It is included in this repo as a submodule:
- `ip/home-inventory-chip/`

## How this repo will be used
- Keep OpenMPW-specific structure, Makefiles, and precheck CI here.
- Pull the actual user design RTL (and later, hardened macros) from the source-of-truth repo.

## Keep the harness RTL filelist in sync
The canonical RTL filelist lives in the IP submodule:
- `ip/home-inventory-chip/rtl/ip_home_inventory.f`

The harness consumes:
- `verilog/rtl/ip_home_inventory.f`

To sync it (recommended before any compile/sim):

```bash
make sync-ip-filelist
```

## Quick sanity check (no OpenLane / low disk)
From the repo root:

```bash
make rtl-compile-check
```

This runs a fast Icarus compile of:
- `verilog/rtl/user_project_wrapper.v`
- `verilog/rtl/home_inventory_user_project.v`
- the submodule RTL listed in `verilog/rtl/ip_home_inventory.f`

## Next steps
- Add a tiny cocotb test that smoke-reads ID/VERSION over Wishbone.
- Configure OpenLane for SKY130A and run mpw-precheck when disk/tooling allows.
