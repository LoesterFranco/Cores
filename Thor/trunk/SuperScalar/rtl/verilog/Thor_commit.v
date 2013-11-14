// ============================================================================
//        __
//   \\__/ o\    (C) 2013  Robert Finch, Stratford
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
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
// Thor SuperScalar
// Commit phase logic
//
// ============================================================================
//

// It didn't work in simulation when the following was declared under an
// independant always clk block
//
if (commit0_v && commit0_tgt[8:4]==5'h11) begin
	bregs[commit0_tgt[3:0]] <= commit0_bus;
	$display("bregs[%d]<=%h", commit0_tgt[3:0], commit0_bus);
end
if (commit1_v && commit1_tgt[8:4]==5'h11) begin
	$display("bregs[%d]<=%h", commit1_tgt[3:0], commit1_bus);
	bregs[commit1_tgt[3:0]] <= commit1_bus;
end

`ifdef SEGMENTATION
if (commit0_v && commit0_tgt[8:4]==5'h12) begin
	$display("sregs[%d]<=%h", commit0_tgt[3:0], commit0_bus);
	sregs[commit0_tgt[3:0]] <= commit0_bus[DBW-1:12];
end
if (commit1_v && commit1_tgt[8:4]==5'h12) begin
	$display("sregs[%d]<=%h", commit1_tgt[3:0], commit1_bus);
	sregs[commit1_tgt[3:0]] <= commit1_bus[DBW-1:12];
end
`endif

if (commit0_v && commit0_tgt[8:4]==5'h10)
	pregs[commit0_tgt[3:0]] <= commit0_bus[3:0];
if (commit1_v && commit1_tgt[8:4]==5'h10)
	pregs[commit1_tgt[3:0]] <= commit1_bus[3:0];

//	if (commit1_v && commit1_tgt[8:4]==5'h10)
//		pregs[commit1_tgt[3:0]] <= commit1_bus[3:0];
if (commit0_v && commit0_tgt==9'h130) begin
	pregs[0] <= commit0_bus[3:0];
	pregs[1] <= commit0_bus[7:4];
	pregs[2] <= commit0_bus[11:8];
	pregs[3] <= commit0_bus[15:12];
	pregs[4] <= commit0_bus[19:16];
	pregs[5] <= commit0_bus[23:20];
	pregs[6] <= commit0_bus[27:24];
	pregs[7] <= commit0_bus[31:28];
	if (DBW==64) begin
		pregs[8] <= commit0_bus[35:32];
		pregs[9] <= commit0_bus[39:36];
		pregs[10] <= commit0_bus[43:40];
		pregs[11] <= commit0_bus[47:44];
		pregs[12] <= commit0_bus[51:48];
		pregs[13] <= commit0_bus[55:52];
		pregs[14] <= commit0_bus[59:56];
		pregs[15] <= commit0_bus[63:60];
	end
end
if (commit1_v && commit1_tgt==9'h130) begin
	pregs[0] <= commit1_bus[3:0];
	pregs[1] <= commit1_bus[7:4];
	pregs[2] <= commit1_bus[11:8];
	pregs[3] <= commit1_bus[15:12];
	pregs[4] <= commit1_bus[19:16];
	pregs[5] <= commit1_bus[23:20];
	pregs[6] <= commit1_bus[27:24];
	pregs[7] <= commit1_bus[31:28];
	if (DBW==64) begin
		pregs[8] <= commit1_bus[35:32];
		pregs[9] <= commit1_bus[39:36];
		pregs[10] <= commit1_bus[43:40];
		pregs[11] <= commit1_bus[47:44];
		pregs[12] <= commit1_bus[51:48];
		pregs[13] <= commit1_bus[55:52];
		pregs[14] <= commit1_bus[59:56];
		pregs[15] <= commit1_bus[63:60];
	end
end

// When the INT instruction commits set the hardware interrupt status to disable further interrupts.
if (int_commit)
begin
	$display("*********************");
	$display("*********************");
	$display("Interrupt committing");
	$display("*********************");
	$display("*********************");
	StatusHWI <= `TRUE;
	imb <= im;
	im <= 1'b0;
	// Reset the nmi edge sense circuit but only for an NMI
	if ((iqentry_a0[head0][7:0]==8'hFE && commit0_v && iqentry_op[head0]==`INT) ||
	    (iqentry_a0[head1][7:0]==8'hFE && commit1_v && iqentry_op[head1]==`INT))
		nmi_edge <= 1'b0;
	string_pc <= 64'd0;
end

if (commit0_v) begin
	case(iqentry_op[head0])
	`CLI:	im <= 1'b0;
	`SEI:	im <= 1'b1;
	// When the RTI instruction commits clear the hardware interrupt status to enable interrupts.
	`RTI:	begin
			StatusHWI <= `FALSE;
			im <= imb;
			end
	`LOOP:
		if (lc != 64'd0)
			lc <= lc - 64'd1;
	`MTSPR:
		begin
			if (iqentry_tgt[head0]==`LCTR)
				lc <= commit0_bus;
			if (iqentry_tgt[head1]==`MISSADR)
				miss_addr <= commit0_bus;
		end
	default:	;
	endcase
end

if (commit0_v && commit1_v) begin
	case(iqentry_op[head1])
	`CLI:	im <= 1'b0;
	`SEI:	im <= 1'b1;
	`RTI:	begin
			StatusHWI <= `FALSE;
			im <= imb;
			end
	`LOOP:
		if (lc != 64'd0)
			lc <= lc - 64'd1;
	`MTSPR:
		begin
			if (iqentry_tgt[head1]==`LCTR)
				lc <= commit1_bus;
			if (iqentry_tgt[head1]==`MISSADR)
				miss_addr <= commit1_bus;
		end
	default:	;
	endcase
end

//
// COMMIT PHASE (dequeue only ... not register-file update)
//
// look at head0 and head1 and let 'em write to the register file if they are ready
//
if (~|panic)
case ({ iqentry_v[head0],
	iqentry_done[head0],
	iqentry_v[head1],
	iqentry_done[head1] })

	// 4'b00_00	- neither valid; skip both
	// 4'b00_01	- neither valid; skip both
	// 4'b00_10	- skip head0, wait on head1
	// 4'b00_11	- skip head0, commit head1
	// 4'b01_00	- neither valid; skip both
	// 4'b01_01	- neither valid; skip both
	// 4'b01_10	- skip head0, wait on head1
	// 4'b01_11	- skip head0, commit head1
	// 4'b10_00	- wait on head0
	// 4'b10_01	- wait on head0
	// 4'b10_10	- wait on head0
	// 4'b10_11	- wait on head0
	// 4'b11_00	- commit head0, skip head1
	// 4'b11_01	- commit head0, skip head1
	// 4'b11_10	- commit head0, wait on head1
	// 4'b11_11	- commit head0, commit head1

	//
	// retire 0
	4'b10_00,
	4'b10_01,
	4'b10_10,
	4'b10_11: ;

	//
	// retire 1
	4'b00_10,
	4'b01_10,
	4'b11_10: begin
		if (iqentry_v[head0] || head0 != tail0) begin
			iqentry_v[head0] <= `INV;	// may conflict with STOMP, but since both are setting to 0, it is okay
			head0 <= head0 + 1;
			head1 <= head1 + 1;
			head2 <= head2 + 1;
			head3 <= head3 + 1;
			head4 <= head4 + 1;
			head5 <= head5 + 1;
			head6 <= head6 + 1;
			head7 <= head7 + 1;
//			if (iqentry_v[head0] && iqentry_exc[head0])	panic <= `PANIC_HALTINSTRUCTION;
			I <= I + 1;
		end
	end

	//
	// retire 2
	default: begin
		if (((iqentry_v[head0] && iqentry_v[head1]) || (head0 != tail0 && head1 != tail0)) && !limit_cmt) begin
			iqentry_v[head0] <= `INV;	// may conflict with STOMP, but since both are setting to 0, it is okay
			iqentry_v[head1] <= `INV;	// may conflict with STOMP, but since both are setting to 0, it is okay
			head0 <= head0 + 2;
			head1 <= head1 + 2;
			head2 <= head2 + 2;
			head3 <= head3 + 2;
			head4 <= head4 + 2;
			head5 <= head5 + 2;
			head6 <= head6 + 2;
			head7 <= head7 + 2;
//			if (iqentry_v[head0] && iqentry_exc[head0])	panic <= `PANIC_HALTINSTRUCTION;
//			if (iqentry_v[head1] && iqentry_exc[head1])	panic <= `PANIC_HALTINSTRUCTION;
			I <= I + 2;
		end
		else if (iqentry_v[head0] || head0 != tail0) begin
			iqentry_v[head0] <= `INV;	// may conflict with STOMP, but since both are setting to 0, it is okay
			head0 <= head0 + 1;
			head1 <= head1 + 1;
			head2 <= head2 + 1;
			head3 <= head3 + 1;
			head4 <= head4 + 1;
			head5 <= head5 + 1;
			head6 <= head6 + 1;
			head7 <= head7 + 1;
//			if (iqentry_v[head0] && iqentry_exc[head0])	panic <= `PANIC_HALTINSTRUCTION;
			I <= I + 1;
		end
	end
endcase