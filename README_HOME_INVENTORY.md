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

## Next steps
- Create a minimal user-project RTL wrapper that instantiates our top.
- Configure OpenLane for SKY130A and run mpw-precheck early.
