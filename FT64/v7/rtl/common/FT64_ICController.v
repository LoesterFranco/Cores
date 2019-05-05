// ============================================================================
//        __
//   \\__/ o\    (C) 2017-2019  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
//	FT64_ICController.v
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
// ============================================================================
//
`include ".\FT64_config.vh"
`define HIGH	1'b1
`define LOW		1'b0;

module FT64_ICController(clk_i, asid, pc0, pc1, pc2, hit0, hit1, hit2, bstate, state,
	thread_en, ihitL2, selL2, L2_ld, L2_cnt, L2_adr, L2_xsel, L2_dato, L2_nxt,
	L1_selpc, L1_adr, L1_dat, L1_wr0, L1_wr1, L1_wr2, L1_en, L1_invline, icnxt, icwhich,
	icl_o, cti_o, bte_o, bok_i, cyc_o, stb_o, ack_i, err_i, tlbmiss_i, exv_i, sel_o, adr_o, dat_i);
parameter ABW = 64;
parameter AMSB = ABW-1;
parameter RSTPC = 64'hFFFFFFFFFFFC0100;
input clk_i;
input [7:0] asid;
input [AMSB:0] pc0;
input [AMSB:0] pc1;
input [AMSB:0] pc2;
input hit0;
input hit1;
input hit2;
input [4:0] bstate;
output reg [3:0] state = IDLE;
input thread_en;
input ihitL2;
output reg selL2 = 1'b0;
output L2_ld;
output [2:0] L2_cnt;
output reg [71:0] L2_adr = RSTPC;
output reg L2_xsel = 1'b0;
input [305:0] L2_dato;
output reg L2_nxt;
output L1_selpc;
output reg [71:0] L1_adr = RSTPC;
output reg [305:0] L1_dat = {2'b0,{38{8'h3D}}};	// NOP
output reg L1_wr0;
output reg L1_wr1;
output reg L1_wr2;
output reg [9:0] L1_en;
output reg L1_invline;
output reg icnxt;
output reg [1:0] icwhich;
output reg icl_o;
output reg [2:0] cti_o = 3'b000;
output reg [1:0] bte_o = 2'b00;
input bok_i;
output reg cyc_o = 1'b0;
output reg stb_o;
input ack_i;
input err_i;
input tlbmiss_i;
input exv_i;
output reg [7:0] sel_o;
output reg [71:0] adr_o;
input [63:0] dat_i;

parameter TRUE = 1'b1;
parameter FALSE = 1'b0;

reg [3:0] picstate;
`include ".\FT64_busStates.vh"

wire [AMSB:0] pc0plus6 = pc0 + 8'd7;
wire [AMSB:0] pc0plus12 = pc0 + 8'd14;

assign L2_ld = (state==IC_Ack) && (ack_i|err_i|tlbmiss_i|exv_i);
assign L1_selpc = state==IDLE||state==IC_Next;

wire clk = clk_i;
reg [2:0] iccnt;
assign L2_cnt = iccnt;

//BUFH uclkb (.I(clk_i), .O(clk));

always @(posedge clk)
begin
L1_wr0 <= FALSE;
L1_wr1 <= FALSE;
L1_wr2 <= FALSE;
L1_en <= 10'h000;
L1_invline <= FALSE;
icnxt <= FALSE;
L2_nxt <= FALSE;
// Instruction cache state machine.
// On a miss first see if the instruction is in the L2 cache. No need to go to
// the BIU on an L1 miss.
// If not the machine will wait until the BIU loads the L2 cache.

// Capture the previous ic state, used to determine how long to wait in
// icstate #4.
picstate <= state;
case(state)
IDLE:
	begin
		iccnt <= 3'd0;
		// If the bus unit is busy doing an update involving L1_adr or L2_adr
		// we have to wait.
		begin
			if (!hit0) begin
				L1_adr <= {asid,pc0[AMSB:5],5'h0};
				L1_invline <= TRUE;
				icwhich <= 2'b00;
				state <= IC2;
			end
			else if (!hit1 && `WAYS > 1) begin
				if (thread_en) begin
					L1_adr <= {asid,pc1[AMSB:5],5'h0};
				end
				else begin
					L1_adr <= {asid,pc0plus6[AMSB:5],5'h0};
				end
				L1_invline <= TRUE;
				icwhich <= 2'b01;
				state <= IC2;
			end
			else if (!hit2 && `WAYS > 2) begin
				if (thread_en) begin
					L1_adr <= {asid,pc2[AMSB:5],5'h0};
				end
				else begin
					L1_adr <= {asid,pc0plus12[AMSB:5],5'h0};
				end
				L1_invline <= TRUE;
				icwhich <= 2'b10;
				state <= IC2;
			end
		end
	end
	
IC2:     state <= IC3;
IC3:     state <= IC3a;
IC3a:     state <= IC_WaitL2;
// If data was in the L2 cache already there's no need to wait on the
// BIU to retrieve data. It can be determined if the hit signal was
// already active when this state was entered in which case waiting
// will do no good.
// The IC machine will stall in this state until the BIU has loaded the
// L2 cache. 
IC_WaitL2: 
	if (ihitL2 && picstate==IC3a) begin
		L1_en <= 10'h3FF;
		L1_wr0 <= TRUE;
		L1_wr1 <= TRUE && `WAYS > 1;
		L1_wr2 <= TRUE && `WAYS > 2;
//		L1_adr <= L2_adr;
		// L1_dati is loaded dring an L2 icache load operation
//		if (picstate==IC3a)
		L1_dat <= L2_dato;
		state <= IC5;
	end
	else begin
		if (bstate == B_WaitIC)
			state <= IC_Access;
	end
/*
	else if (state!=IC_Nack)
		;
	else begin
		L1_en <= 10'h3FF;
		L1_wr0 <= TRUE;
		L1_wr1 <= TRUE && `WAYS > 1;
		L1_wr2 <= TRUE && `WAYS > 2;
//		L1_adr <= L2_adr;
		// L1_dati set below while loading cache line
		//L1_dati <= L2_dato;
		state <= IC5;
	end
*/
IC5: 	state <= IC6;
IC6:  state <= IC7;
IC7:	state <= IC_Next;
IC_Next:
  begin
   state <= IDLE;
   icnxt <= TRUE;
	end
IC_Access:
	begin
		icl_o <= `HIGH;
		cti_o <= 3'b001;
		bte_o <= 2'b00;
		cyc_o <= `HIGH;
		stb_o <= `HIGH;
		sel_o <= 8'hFF;
		adr_o <= {L1_adr[AMSB:5],5'b0};
		L2_adr <= L1_adr;
		L2_adr[4:0] <= 5'd0;
		L2_xsel <= 1'b0;
		selL2 <= TRUE;
		state <= IC_Ack;
	end
IC_Ack:
  if (ack_i|err_i|tlbmiss_i|exv_i) begin
  	if (!bok_i) begin
  		stb_o <= `LOW;
			adr_o[AMSB:3] <= adr_o[AMSB:3] + 2'd1;
  		state <= IC_Nack2;
  	end
		if (tlbmiss_i) begin
			L1_dat[305:304] <= 2'd1;
			L1_dat[303:0] <= {38{8'h3D}};	// NOP
			nack();
	  end
		else if (exv_i) begin
			L1_dat[305:304] <= 2'd2;
			L1_dat[303:0] <= {38{8'h3D}};	// NOP
			nack();
		end
	  else if (err_i) begin
			L1_dat[305:304] <= 2'd3;
			L1_dat[303:0] <= {38{8'h3D}};	// NOP
			nack();
	  end
	  else
	  	case(iccnt)
	  	3'd0:	L1_dat[63:0] <= dat_i;
	  	3'd1:	L1_dat[127:64] <= dat_i;
	  	3'd2:	L1_dat[191:128] <= dat_i;
	  	3'd3:	L1_dat[255:192] <= dat_i;
	  	3'd4:	L1_dat[305:256] <= {2'b00,dat_i[47:0]};
	  	default:	L1_dat <= L1_dat;
	  	endcase
    iccnt <= iccnt + 3'd1;
    if (iccnt==3'd3)
      cti_o <= 3'b111;
    if (iccnt==3'd4)
    	nack();
    else begin
      L2_adr[4:3] <= L2_adr[4:3] + 2'd1;
      if (L2_adr[4:3]==2'b11)
      	L2_xsel <= 1'b1;
    end
  end
IC_Nack2:
	if (~ack_i) begin
		stb_o <= `HIGH;
		state <= IC_Ack;
	end
IC_Nack:
 	begin
		selL2 <= FALSE;
		if (~ack_i) begin
			//icl_ctr <= icl_ctr + 40'd1;
			state <= IDLE;
			L2_nxt <= TRUE;
		end
	end
default:
	begin
   	state <= IDLE;
  end
endcase
end

task nack;
begin
	icl_o <= `LOW;
	cti_o <= 3'b000;
	cyc_o <= `LOW;
	stb_o <= `LOW;
	L1_en <= 10'h3FF;
	L1_wr0 <= TRUE;
	L1_wr1 <= TRUE && `WAYS > 1;
	L1_wr2 <= TRUE && `WAYS > 2;
	state <= IC_Nack;
end
endtask

endmodule
