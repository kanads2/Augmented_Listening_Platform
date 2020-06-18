module i2s_fifo 
	# (	parameter LENGTH = 10,
		parameter WIDTH = 32)
	(
		input logic CLK, RST,
		input logic pop, // signal to pop from fifo
		input logic i2s_sample_ready, // signal to load
		input logic [WIDTH-1:0] fifo_in,
		output logic [WIDTH-1:0] fifo_out,
		output logic fifo_empty
	);

	logic [LENGTH-1:0][31:0] fifo; // array for values in fifo 
	logic [5:0] fifo_count; // number of values in fifo
	enum logic [2:0] {idle, ready, waiting} state, next_state;

	initial begin 
		fifo = 0;
	end

	always_ff @ (posedge CLK) begin // state reset block
		if (RST) begin
			state <= idle;
		end
		else
			state <= next_state;
	end

	always_comb begin // state transition logic for pop
		case (state)
			idle : next_state = (i2s_sample_ready) ? ready : idle;
			ready : next_state = waiting;
			waiting : next_state = (!i2s_sample_ready) ? idle : waiting;
		endcase 

		fifo_empty = (fifo_count == 0) ? 1 : 0; // empty goes high when count is zero
		fifo_out = fifo[0];

	end

	always_ff @ (posedge CLK) begin
		if (RST) begin
			fifo_count <= 0;
			fifo <= 0;
		end
		else begin
			if (state == ready) begin
				fifo <= {fifo_in, fifo[8:0]};
				fifo_count <= fifo_count + 1;
			end
			if (pop) begin
				fifo <= {fifo[8:0], 32'd0};
			end
		end
	end

endmodule