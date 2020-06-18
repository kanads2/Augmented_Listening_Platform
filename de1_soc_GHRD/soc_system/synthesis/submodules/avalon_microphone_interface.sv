`define FIFO_WIDTH 32

module avalon_microphone_interface 
	# (parameter WIDTH = 16) 
	(
	// I2S signals
	input logic sck, // i2s bit clock
	input logic ws, // i2s lrclk
	input logic sd, // i2s bitstream

	// Avalon-MM Slave Signals
	input  logic AVL_READ,					// Avalon-MM Read
	input  logic AVL_WRITE,					// Avalon-MM Write
	input  logic AVL_CS,						// Avalon-MM Chip Select
	input  logic [2:0] AVL_ADDR,					// Avalon-MM Address
	input  logic [31:0] AVL_WRITEDATA,	// Avalon-MM Write Data
	output logic [31:0] AVL_READDATA,	// Avalon-MM Read Data

	input logic CLK, RST
	);

	// LOCAL VARIABLES
	logic [WIDTH-1:0] data_left; // i2s samples
	logic [WIDTH-1:0] data_right;
	logic i2s_sample_ready;
	logic i2s_sample_ready_latch;
	logic pop; // signal to pop value from fifo
	logic fifo_empty;
	logic [`FIFO_WIDTH-1:0] fifo_in, fifo_out;

	assign fifo_in = {data_left, data_right};

	i2s_master 	m0 	(
					.sck, 
					.ws, 
					.sd, 
					.data_right, 
					.data_left,
					.i2s_sample_ready
					); // i2s receiver

	i2s_fifo fifo0 	(
					.CLK,
					.RST,
					.pop,
					.i2s_sample_ready,
					.fifo_in,
					.fifo_out,
					.fifo_empty
					);

	always_comb begin
		AVL_READDATA = 0;
		pop = 0;
		if (AVL_READ && AVL_CS) begin
			case (AVL_ADDR)
				0 : AVL_READDATA = i2s_sample_ready_latch;
				1 : begin
						AVL_READDATA = fifo_out;
						pop = 1;
					end
				default : begin
								AVL_READDATA = 0; 
								pop = 0;
							end
			endcase
		end
	end

endmodule