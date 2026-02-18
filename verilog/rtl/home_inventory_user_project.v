// Home Inventory Chip - OpenMPW user project
//
// This wrapper integrates the source-of-truth RTL from the submodule:
//   ip/home-inventory-chip
//
// For now, we hook up the Wishbone register block (home_inventory_wb) so that
// firmware + harness bring-up can enumerate a stable regfile immediately.

`default_nettype none

// Pull in the regblock RTL from the source-of-truth repo.
// NOTE: This is a temporary, explicit include to keep the harness build simple.
//       Longer-term, we should plumb the submodule RTL into the build filelist.
`include "ip/home-inventory-chip/rtl/home_inventory_wb.v"

module home_inventory_user_project #(
    parameter BITS = 32
) (
`ifdef USE_POWER_PINS
    inout vccd1,
    inout vssd1,
`endif

    // Wishbone Slave ports (WB MI A)
    input  wire        wb_clk_i,
    input  wire        wb_rst_i,
    input  wire        wbs_stb_i,
    input  wire        wbs_cyc_i,
    input  wire        wbs_we_i,
    input  wire [3:0]  wbs_sel_i,
    input  wire [31:0] wbs_dat_i,
    input  wire [31:0] wbs_adr_i,
    output wire        wbs_ack_o,
    output wire [31:0] wbs_dat_o,

    // Logic Analyzer Signals
    input  wire [127:0] la_data_in,
    output wire [127:0] la_data_out,
    input  wire [127:0] la_oenb,

    // IOs
    input  wire [`MPRJ_IO_PADS-1:0] io_in,
    output wire [`MPRJ_IO_PADS-1:0] io_out,
    output wire [`MPRJ_IO_PADS-1:0] io_oeb,

    // User maskable interrupt signals
    output wire [2:0] user_irq
);

    // Default: no LA driving
    assign la_data_out = 128'h0;

    // IOs: high-Z by default
    assign io_out = {`MPRJ_IO_PADS{1'b0}};
    assign io_oeb = {`MPRJ_IO_PADS{1'b1}}; // 1 => output disabled

    // No interrupts yet (regfile exposes IRQ_EN as config only)
    assign user_irq = 3'b000;

    // Core status is not wired yet; keep at 0 until the core is integrated.
    wire [7:0] core_status = 8'h00;

    // Control plane outputs (for future integration)
    wire ctrl_enable;
    wire ctrl_start;
    wire [2:0] irq_en;

    home_inventory_wb u_wb (
        .wb_clk_i(wb_clk_i),
        .wb_rst_i(wb_rst_i),
        .wbs_stb_i(wbs_stb_i),
        .wbs_cyc_i(wbs_cyc_i),
        .wbs_we_i (wbs_we_i),
        .wbs_sel_i(wbs_sel_i),
        .wbs_dat_i(wbs_dat_i),
        .wbs_adr_i(wbs_adr_i),
        .wbs_ack_o(wbs_ack_o),
        .wbs_dat_o(wbs_dat_o),

        .core_status(core_status),

        .ctrl_enable(ctrl_enable),
        .ctrl_start (ctrl_start),
        .irq_en     (irq_en)
    );

    // TODO(next): integrate the sampling core and drive status/IRQs.

endmodule

`default_nettype wire
