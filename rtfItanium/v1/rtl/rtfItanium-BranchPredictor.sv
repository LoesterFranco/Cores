//=============================================================================
//        __
//   \\__/ o\    (C) 2013-2018  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//  
//	FT64_BranchPredictor.v
//
//  
// This source file is free software: you can redistribute it and/or modify 
// it under the terms of the GNU Lesser General Public License as published 
// by the Free Software Foundation, either version 3 of the License, or     
// (at your option) any later version.                                      
//                                                                          
// This source file is distributed in the hope that it will be useful,      
// but WITHOUT ANY WARRANTY; without even the implied warranty of           
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            
// GNU General Public License for more details.                             
//                                                                          
// You should have received a copy of the GNU General Public License        
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    
//                                                                          
//
//=============================================================================
//
module BranchPredictor(rst, clk, en,
    xisBranch0, xisBranch1, xisBranch2,
    pcA, pcB, pcC, xpc0, xpc1, xpc2, takb0, takb1, takb2,
    predict_takenA, predict_takenB, predict_takenC);
parameter AMSB=63;
parameter DBW=32;
input rst;
input clk;
input en;
input xisBranch0;
input xisBranch1;
input xisBranch2;
input [AMSB:0] pcA;
input [AMSB:0] pcB;
input [AMSB:0] pcC;
input [AMSB:0] xpc0;
input [AMSB:0] xpc1;
input [AMSB:0] xpc2;
input takb0;
input takb1;
input takb2;
output predict_takenA;
output predict_takenB;
output predict_takenC;

integer n;
reg [AMSB:0] pcs [0:31];
reg [AMSB:0] pc;
reg takb;
reg [4:0] pcshead,pcstail;
reg wrhist;
reg [2:0] gbl_branch_hist;
reg [1:0] branch_history_table [511:0];
// For simulation only, initialize the history table to zeros.
// In the real world we don't care.
initial begin
    gbl_branch_hist = 3'b000;
	for (n = 0; n < 512; n = n + 1)
		branch_history_table[n] = 3;
end
wire [8:0] bht_wa = {pc[7:1],gbl_branch_hist[2:1]};		// write address
wire [8:0] bht_raA = {pcA[7:1],gbl_branch_hist[2:1]};	// read address (IF stage)
wire [8:0] bht_raB = {pcB[7:1],gbl_branch_hist[2:1]};	// read address (IF stage)
wire [8:0] bht_raC = {pcC[7:1],gbl_branch_hist[2:1]};	// read address (IF stage)
wire [1:0] bht_xbits = branch_history_table[bht_wa];
wire [1:0] bht_ibitsA = branch_history_table[bht_raA];
wire [1:0] bht_ibitsB = branch_history_table[bht_raB];
wire [1:0] bht_ibitsC = branch_history_table[bht_raC];
assign predict_takenA = (bht_ibitsA==2'd0 || bht_ibitsA==2'd1) && en;
assign predict_takenB = (bht_ibitsB==2'd0 || bht_ibitsB==2'd1) && en;
assign predict_takenC = (bht_ibitsC==2'd0 || bht_ibitsC==2'd1) && en;

always @(posedge clk)
if (rst)
	pcstail <= 5'd0;
else begin
	case({xisBranch0,xisBranch1,xisBranch2})
	3'b000:	;
	3'b001:
		begin
		pcs[pcstail] <= {xpc2[31:1],takb2};
		pcstail <= pcstail + 5'd1;
		end
	3'b010:
		begin
		pcs[pcstail] <= {xpc1[31:1],takb1};
		pcstail <= pcstail + 5'd1;
		end
	3'b011:
		begin
		pcs[pcstail] <= {xpc1[31:1],takb1};
		pcs[pcstail+1] <= {xpc2[31:1],takb2};
		pcstail <= pcstail + 5'd2;
		end
	3'b100:
		begin
		pcs[pcstail] <= {xpc0[31:1],takb0};
		pcstail <= pcstail + 5'd1;
		end
	3'b101:
		begin
		pcs[pcstail] <= {xpc0[31:1],takb0};
		pcs[pcstail+1] <= {xpc2[31:1],takb2};
		pcstail <= pcstail + 5'd2;
		end
	3'b110:
		begin
		pcs[pcstail] <= {xpc0[31:1],takb0};
		pcs[pcstail+1] <= {xpc1[31:1],takb1};
		pcstail <= pcstail + 5'd2;
		end
	3'b111:
		begin
		pcs[pcstail] <= {xpc0[31:1],takb0};
		pcs[pcstail+1] <= {xpc1[31:1],takb1};
		pcs[pcstail+2] <= {xpc2[31:1],takb2};
		pcstail <= pcstail + 5'd3;
		end
	endcase
end

always @(posedge clk)
if (rst)
	pcshead <= 5'd0;
else begin
	wrhist <= 1'b0;
	if (pcshead != pcstail) begin
		pc <= pcs[pcshead];
		takb <= pcs[pcshead][0];
		wrhist <= 1'b1;
		pcshead <= pcshead + 5'd1;
	end
end

// Two bit saturating counter
// If taking a branch in commit0 then a following branch
// in commit1 is never encountered. So only update for
// commit1 if commit0 is not taken.
reg [1:0] xbits_new;
always @*
if (wrhist) begin
	if (takb) begin
		if (bht_xbits != 2'd1)
			xbits_new <= bht_xbits + 2'd1;
		else
			xbits_new <= bht_xbits;
	end
	else begin
		if (bht_xbits != 2'd2)
			xbits_new <= bht_xbits - 2'd1;
		else
			xbits_new <= bht_xbits;
	end
end
else
	xbits_new <= bht_xbits;

always @(posedge clk)
if (rst)
	gbl_branch_hist <= 3'b000;
else begin
  if (en) begin
    if (wrhist) begin
      gbl_branch_hist <= {gbl_branch_hist[1:0],takb};
      branch_history_table[bht_wa] <= xbits_new;
    end
	end
end

endmodule

