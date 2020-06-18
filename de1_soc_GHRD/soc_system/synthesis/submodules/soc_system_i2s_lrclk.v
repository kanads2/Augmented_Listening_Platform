// (C) 2001-2019 Intel Corporation. All rights reserved.
// Your use of Intel Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files from any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Intel Program License Subscription 
// Agreement, Intel FPGA IP License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Intel and sold by 
// Intel or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


module soc_system_i2s_lrclk (
	// Globals
	clk,
	reset,

	// Bit Clock
	AUD_BCLK,

	// Left-Right Clock
	AUD_LRCLK
);

/*****************************************************************************
 *                           Parameter Declarations                          *
 *****************************************************************************/


/*****************************************************************************
 *                             Port Declarations                             *
 *****************************************************************************/
// Globals
input				clk;
input				reset;

// Bit Clock
input				AUD_BCLK;

// Left-Right Clock
output reg			AUD_LRCLK;

/*****************************************************************************
 *                           Constant Declarations                           *
 *****************************************************************************/

localparam BIT_COUNTER_INIT	= 5'd31;

/*****************************************************************************
 *                 Internal Wires and Registers Declarations                 *
 *****************************************************************************/

reg			[ 4: 0]	bclk_counter;

/*****************************************************************************
 *                         Finite State Machine(s)                           *
 *****************************************************************************/


/*****************************************************************************
 *                             Sequential Logic                              *
 *****************************************************************************/

always @(negedge AUD_BCLK)
begin
	if (reset) begin
		AUD_LRCLK <= 1'b0;
		bclk_counter <= 5'b0;
	end else if (bclk_counter == BIT_COUNTER_INIT) begin
		AUD_LRCLK <= ~AUD_LRCLK;
		bclk_counter <= 5'b0;
	end else
		bclk_counter <= bclk_counter + 1;
end

/*****************************************************************************
 *                            Combinational Logic                            *
 *****************************************************************************/


/*****************************************************************************
 *                              Internal Modules                             *
 *****************************************************************************/

endmodule

