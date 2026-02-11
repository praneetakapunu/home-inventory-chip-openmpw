// Home Inventory Chip - OpenMPW user project (initial skeleton)
//
// This is intentionally minimal: it provides a place to integrate the real
// design from the source-of-truth repo (ip/home-inventory-chip).

`default_nettype none

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
    output reg         wbs_ack_o,
    output reg  [31:0] wbs_dat_o,

    // Logic Analyzer Signals
    input  wire [127:0] la_data_in,
    output wire [127:0] la_data_out,
    input  wire [127:0] la_oenb,

    // IOs
    input  wire [`MPRJ_IO_PADS-1:0] io_in,
    output reg  [`MPRJ_IO_PADS-1:0] io_out,
    output reg  [`MPRJ_IO_PADS-1:0] io_oeb,

    // User maskable interrupt signals
    output reg  [2:0] user_irq
);

    // Default: no LA driving
    assign la_data_out = 128'h0;

    // Minimal wishbone: ACK one cycle for any access, return 0.
    always @(posedge wb_clk_i) begin
        if (wb_rst_i) begin
            wbs_ack_o <= 1'b0;
            wbs_dat_o <= 32'h0;
        end else begin
            // single-cycle ack pulse when valid
            wbs_ack_o <= wbs_cyc_i & wbs_stb_i & ~wbs_ack_o;
            wbs_dat_o <= 32'h0;
        end
    end

    // IOs: high-Z by default
    always @(*) begin
        io_out = {`MPRJ_IO_PADS{1'b0}};
        io_oeb = {`MPRJ_IO_PADS{1'b1}}; // 1 => output disabled
        user_irq = 3'b000;
    end

    // TODO(next): integrate real RTL + register map + ADC interface.

endmodule

`default_nettype wire
