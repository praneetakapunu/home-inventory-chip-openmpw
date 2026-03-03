// Home Inventory Chip - OpenMPW user project
//
// This wrapper integrates the source-of-truth RTL from the submodule:
//   ip/home-inventory-chip
//
// For now, we hook up the Wishbone register block (home_inventory_wb) so that
// firmware + harness bring-up can enumerate a stable regfile immediately.

`default_nettype none

// Source-of-truth RTL is compiled from the submodule via the harness filelist:
//   verilog/rtl/ip_home_inventory.f
//
// In particular, this wrapper expects `home_inventory_wb` to be available
// (compiled from: ip/home-inventory-chip/rtl/home_inventory_wb.v).

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

    localparam integer IO_PADS = `MPRJ_IO_PADS;

    // IOs: high-Z by default
    // (v1 starts WB-only; GPIO routing is opt-in behind a compile-time guard)
`ifndef HOMEINV_ENABLE_ADC_GPIO
    assign io_out = {IO_PADS{1'b0}};
    assign io_oeb = {IO_PADS{1'b1}}; // 1 => output disabled
`else
    // -----------------------------------------------------------------
    // ADC GPIO routing (opt-in)
    // -----------------------------------------------------------------
    // This wrapper does NOT yet implement the ADC core; this section only
    // reserves a clean, reviewable place to wire Caravel io[*] pads for the
    // external ADS131M08 interface.
    //
    // IMPORTANT: the default indices below are placeholders.
    // Do not tapeout with these values; set them explicitly when the mapping
    // is locked (see docs/source/adc_pinout_plan.md).

    parameter integer ADC_SCLK_IO  = 0;
    parameter integer ADC_CSN_IO   = 1;
    parameter integer ADC_MOSI_IO  = 2;
    parameter integer ADC_MISO_IO  = 3;
    parameter integer ADC_DRDYN_IO = 4;
    parameter integer ADC_RSTN_IO  = 5;

    // One-hot masks.
    localparam [IO_PADS-1:0] MASK_ADC_SCLK  = {{(IO_PADS-1){1'b0}}, 1'b1} << ADC_SCLK_IO;
    localparam [IO_PADS-1:0] MASK_ADC_CSN   = {{(IO_PADS-1){1'b0}}, 1'b1} << ADC_CSN_IO;
    localparam [IO_PADS-1:0] MASK_ADC_MOSI  = {{(IO_PADS-1){1'b0}}, 1'b1} << ADC_MOSI_IO;
    localparam [IO_PADS-1:0] MASK_ADC_RSTN  = {{(IO_PADS-1){1'b0}}, 1'b1} << ADC_RSTN_IO;

    localparam [IO_PADS-1:0] MASK_ADC_OUTS = MASK_ADC_SCLK | MASK_ADC_CSN | MASK_ADC_MOSI | MASK_ADC_RSTN;

    // Default: all pads are high-Z.
    // When enabled: drive the output pads, leave inputs as high-Z.
    assign io_out =
        (({IO_PADS{1'b0}})) |
        (({IO_PADS{adc_sclk}} & MASK_ADC_SCLK)) |
        (({IO_PADS{adc_cs_n}} & MASK_ADC_CSN)) |
        (({IO_PADS{adc_mosi}} & MASK_ADC_MOSI)) |
        (({IO_PADS{adc_rst_n}} & MASK_ADC_RSTN));

    assign io_oeb =
        (({IO_PADS{1'b1}}) & ~MASK_ADC_OUTS);
`endif

    // No interrupts yet (regfile exposes IRQ_EN as config only)
    assign user_irq = 3'b000;

    // Core status is not wired yet; keep at 0 until the core is integrated.
    wire [7:0] core_status = 8'h00;

    // -----------------------------------------------------------------
    // External ADC interface wires
    // -----------------------------------------------------------------
    // These wires serve *two* purposes:
    // 1) Optional GPIO routing to Caravel pads (HOMEINV_ENABLE_ADC_GPIO)
    // 2) Optional real ADC SPI capture inside the IP (USE_REAL_ADC_INGEST)
    //
    // Safety: when USE_REAL_ADC_INGEST is not enabled, keep safe defaults
    // so that enabling HOMEINV_ENABLE_ADC_GPIO is non-destructive.
    wire adc_sclk;
    wire adc_cs_n;
    wire adc_mosi;
    wire adc_rst_n;
    wire adc_miso;
    wire adc_drdy_n;

`ifndef USE_REAL_ADC_INGEST
    assign adc_sclk  = 1'b0;
    assign adc_cs_n  = 1'b1;
    assign adc_mosi  = 1'b0;
`endif

    // Reset is not yet owned by the IP; keep it deasserted.
    assign adc_rst_n = 1'b1;

`ifdef HOMEINV_ENABLE_ADC_GPIO
    // Input pads: sample from io_in (still high-Z from our side).
    assign adc_miso   = io_in[ADC_MISO_IO];
    assign adc_drdy_n = io_in[ADC_DRDYN_IO];
`else
    assign adc_miso   = 1'b0;
    assign adc_drdy_n = 1'b1;
`endif

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

`ifdef USE_REAL_ADC_INGEST
        ,
        // Real ADC SPI pins are only present on the IP block when
        // USE_REAL_ADC_INGEST is enabled.
        .adc_sclk(adc_sclk),
        .adc_cs_n(adc_cs_n),
        .adc_mosi(adc_mosi),
        .adc_miso(adc_miso)
`endif
    );

    // TODO(next): integrate the sampling core and drive status/IRQs.

endmodule

`default_nettype wire
