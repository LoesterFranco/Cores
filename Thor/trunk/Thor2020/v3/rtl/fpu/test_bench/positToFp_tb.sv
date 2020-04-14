`timescale 1ns / 1ps
module positToFp_tb;

function [31:0] log2;
input reg [31:0] value;
	begin
	value = value-1;
	for (log2=0; value>0; log2=log2+1)
        	value = value>>1;
      	end
endfunction

parameter N=32;
parameter E=8;
parameter Bs=log2(N);
parameter es = 3;

reg [N-1:0] in;
reg clk;

wire [N-1:0] out;

// Instantiate the Unit Under Test (UUT)
positToFp #(.FPWID(N), .PSTWID(N), .es(es)) u1 (
	.i(in), 
	.o(out) 
);


	initial begin
		// Initialize Inputs
		clk = 1;
		// Wait 100 ns for global reset to finish
		#101 in = 65535;

		#655360

		$fclose(outfile);
		$finish;
	end
	
always #5 clk=~clk;
always @(posedge clk) begin
	if (in < 32'hffffffff) 
		in <= in + 65535;
end

integer outfile;
initial outfile = $fopen("d:/cores5/Gambit/v5/rtl/cpu/fpu/test_bench/positToFp_tvo32.txt", "wb");
  always @(negedge clk) begin
     $fwrite(outfile, "%h\t%h\n",in,out);
  end

endmodule

