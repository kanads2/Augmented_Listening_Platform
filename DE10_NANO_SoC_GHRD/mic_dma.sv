module mic_dma #(
	parameter NUM_MIC_PAIRS=4
)(
    	// Avalon Clock Input
	input logic CLK,
	
	// Avalon Reset Input
	input logic RESET,

	// Avalon-MM Master signals
	output logic [31:0] AM_ADDR,
	output logic [2:0]	AM_BURSTCOUNT,
	output logic		AM_WRITE,
	output logic [31:0] AM_WRITEDATA,
	output logic [3:0]	AM_BYTEENABLE,
	input logic			AM_WAITREQUEST,

    // Mic input
    input logic [31:0] mic_data,
	 output logic [2:0] select,

    // Avalon-MM Slave signals
    input logic start,
    input logic read_ready,
    input logic [31:0] start_address,
    input logic [31:0] number_samples,
	input logic half_way_ack,
	input logic end_ack,
	
	// Signal to slave
	output logic half_way_latch,
	output logic end_latch,
	output logic FINISHED
);

logic done;
//logic [31:0][NUM_MIC_PAIRS-1:0] starting_address ;
logic [31:0] starting_address [NUM_MIC_PAIRS];
logic finish_signal;
logic [2:0] mic_count;

int unsigned num_samples, counter;

assign FINISHED = finish_signal;

initial begin // initialize some registers
	finish_signal = 1'b0;
	counter = 0;
	half_way_latch = 0;
	end_latch = 0;
	state = IDLE;
end

enum logic [2:0]{
    IDLE 		= 3'd0,
    WAITDATA 	= 3'd1,
    WAITDATA2 	= 3'd2,
	 LD_X 		= 3'd3,
	 WD_X 	  	= 3'd4,
    FIN       	= 3'd5
} state, next_state;

always_ff @(posedge CLK) // state reset and shifting block
begin
    if (RESET)
    begin
        state <= IDLE;
    end
    else
    begin
        state <= next_state;
    end
end


initial begin
	for (int i = 0; i < NUM_MIC_PAIRS; i++) begin
		starting_address[i] = 0;
	end
	//starting_address = 0;
	num_samples = 0;
	AM_ADDR = 0;
	AM_WRITE = 0;
	done = 0;
	finish_signal = 0;
	mic_count = 0;
	counter = 0;
end // end reset logic and registers block


always_ff @ (posedge CLK) begin
	
end

always_comb // state transition logic block
begin
    next_state = state;
    unique case (state)
        IDLE: // wait for start bit to go high
            if (start)
            begin
                next_state = WAITDATA;
            end
            else
            begin
                next_state = IDLE;
            end
        WAITDATA: // clear finish bit and init counter to 0
            next_state = WAITDATA2;
        WAITDATA2: // initialize starting address for each buffer
            next_state = LD_X;
        LD_X:
				if (!AM_WAITREQUEST)
				begin
					if (counter == 0) begin
						if (starting_address[NUM_MIC_PAIRS - 1] == start_address + 32'd38400000)
						begin
							next_state = WAITDATA2;
						end
						else begin
							next_state = read_ready ? WD_X : LD_X;
						end
					end
					else begin
						next_state = WD_X;
					end
				end
				else
				begin
					next_state = LD_X;
				end
			WD_X:
				if (!AM_WAITREQUEST)
				begin
					if (counter == (NUM_MIC_PAIRS - 1)) begin
						next_state = done ? FIN : LD_X;
					end
					else begin
						next_state = LD_X;
					end
				end
				else
            	begin
                	next_state = WD_X;
            	end
			FIN:
	            if (start)
	            begin
	                next_state = FIN;
	            end
	            else
	            begin
	                next_state = IDLE;
	            end
        default: next_state = IDLE; 
    endcase
end // end state transition logic block

always_ff @ (posedge CLK) begin // state machine sequential block
	if (RESET) begin
		for (int i = 0; i < NUM_MIC_PAIRS; i++) begin
			starting_address[i] <= 0;
		end
		//starting_address = 0;
		num_samples <= 0;
		AM_ADDR <= 0;
		AM_WRITE <= 0;
		done <= 0;
		finish_signal <= 0;
		mic_count <= 0;
		counter <= 0;
	end
	else begin
	    case(state)
	        IDLE: // wait for start bit to go high
					begin
					if (start)
					begin
						 num_samples <= number_samples;
						 mic_count <= 2'b10;
					end
					else
					begin // clear AM signals while waiting for start bit
						 AM_ADDR <= 32'd0;
						 AM_WRITE <= 1'b0;
						 num_samples <= 32'd0;
						 for (int i = 0; i < NUM_MIC_PAIRS; i++) begin
							starting_address[i] <= 32'd0;
						 end
					end
					done <= 1'b0;
					end	
	        WAITDATA: // clear finish bit and init counter to 0
			  begin
					finish_signal <= 1'b0;
					counter <= 32'd0;
			  end
	        WAITDATA2: // initialize starting address for each buffer
			  begin
					 for (int i = 0; i < NUM_MIC_PAIRS; i++) begin
						starting_address[i] <= start_address + ((32'd7680000) * i); // We determine this by the fact that we can only record for a max of 40 seconds
					 end
			  end
			LD_X:
			  begin
					mic_count <= counter[2:0] + 3'd1;
					if (counter == 0) begin
						if (read_ready) begin
							if (!AM_WAITREQUEST) begin
								AM_WRITE <= 1'b1;
								AM_ADDR <= starting_address[counter];
								AM_BYTEENABLE <= 4'hF;
								AM_BURSTCOUNT <= 3'b001;
							end
							else begin
								AM_WRITE <= 1'b0;
								AM_ADDR <= 32'b0;
								AM_BYTEENABLE <= 4'hF;
								AM_BURSTCOUNT <= 3'd0;
							end
						end
						else begin
							AM_WRITE <= 1'd0;
							AM_ADDR <= 32'd0;
							AM_BYTEENABLE <= 4'hF;
							AM_BURSTCOUNT <= 3'd0;
						end
					end
					else begin
						if (!AM_WAITREQUEST) begin
							AM_WRITE <= 1'b1;
							AM_ADDR <= starting_address[counter];
							AM_BYTEENABLE <= 4'hF;
							AM_BURSTCOUNT <= 3'b001;
						end
						else begin
							AM_WRITE <= 1'b0;
							AM_ADDR <= 32'b0;
							AM_BYTEENABLE <= 4'hF;
							AM_BURSTCOUNT <= 3'd0;
						end
					end
			  end
			  WD_X:
			  begin
					mic_count <= counter[2:0] + 3'd1;
					if (!AM_WAITREQUEST)
					begin
						if (num_samples > 0)
						begin
							starting_address[counter] <= starting_address[counter] + 4;
							num_samples <= (counter == NUM_MIC_PAIRS - 1) ? num_samples - 1 : num_samples;
							AM_BYTEENABLE <= 4'hF;
						end
						else if (num_samples == 0) begin
							done <= 1'b1;
							AM_WRITE <= 1'b0;
							AM_ADDR <= 32'd0;
							AM_BYTEENABLE <= 4'hF;
							AM_BURSTCOUNT <= 3'd0;	
						end
						counter = (counter == NUM_MIC_PAIRS - 1) ? 0 : (counter + 1);
					end
					else
					begin
					end
			  end
	        FIN:
			  begin
			  		num_samples <= number_samples;
					//starting_address <= start_address;
					counter <= 32'd0;
					done <= 1'b0;
					finish_signal <= 1'b1;
	            AM_WRITE <= 1'b0;
	            AM_ADDR <= start_address;
				end
	        default: ;
	    endcase
	end
end // end state machine sequential block

always_ff @(posedge CLK) begin
	if (half_way_ack | RESET) begin
		half_way_latch <= 0;
	end
	else if (starting_address[NUM_MIC_PAIRS - 1] == start_address + 32'd34560000) begin
		half_way_latch <= 1;
	end
end

always_ff @(posedge CLK) begin
	if (end_ack | RESET) begin
		end_latch <= 0;
	end
	else if (starting_address[NUM_MIC_PAIRS - 1] == start_address + 32'd38400000) begin
		end_latch <= 1;
	end
end

assign select = mic_count;
assign AM_WRITEDATA = mic_data;

endmodule
