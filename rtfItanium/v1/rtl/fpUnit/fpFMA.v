`timescale 1ns / 1ps
// ============================================================================
//        __
//   \\__/ o\    (C) 2019  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
//	fpFMA.v
//		- floating point fused multiplier + adder
//		- can issue every clock cycle
//		- parameterized width
//		- IEEE 754 representation
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
//	Floating Point Multiplier / Divider
//
//	This multiplier/divider handles denormalized numbers.
//	The output format is of an internal expanded representation
//	in preparation to be fed into a normalization unit, then
//	rounding. Basically, it's the same as the regular format
//	except the mantissa is doubled in size, the leading two
//	bits of which are assumed to be whole bits.
//
//
//	Floating Point Multiplier
//
//	Properties:
//	+-inf * +-inf = -+inf	(this is handled by exOver)
//	+-inf * 0     = QNaN
//	
// ============================================================================

module fpFMA (clk, ce, op, rm, a, b, c, o, inf, overflow, underflow);
parameter WID = 32;
localparam MSB = WID-1;
localparam EMSB = WID==128 ? 14 :
                  WID==96 ? 14 :
                  WID==80 ? 14 :
                  WID==64 ? 10 :
				  WID==52 ? 10 :
				  WID==48 ? 11 :
				  WID==44 ? 10 :
				  WID==42 ? 10 :
				  WID==40 ?  9 :
				  WID==32 ?  7 :
				  WID==24 ?  6 : 4;
localparam FMSB = WID==128 ? 111 :
                  WID==96 ? 79 :
                  WID==80 ? 63 :
                  WID==64 ? 51 :
				  WID==52 ? 39 :
				  WID==48 ? 34 :
				  WID==44 ? 31 :
				  WID==42 ? 29 :
				  WID==40 ? 28 :
				  WID==32 ? 22 :
				  WID==24 ? 15 : 9;

localparam FX = (FMSB+2)*2-1;	// the MSB of the expanded fraction
localparam EX = FX + 1 + EMSB + 1 + 1 - 1;

input clk;
input ce;
input op;		// operation 0 = add, 1 = subtract
input [2:0] rm;
input  [WID:1] a, b, c;
output [EX:0] o;
output inf;
output overflow;
output underflow;

// constants
wire [EMSB:0] infXp = {EMSB+1{1'b1}};	// infinite / NaN - all ones
// The following is the value for an exponent of zero, with the offset
// eg. 8'h7f for eight bit exponent, 11'h7ff for eleven bit exponent, etc.
wire [EMSB:0] bias = {1'b0,{EMSB{1'b1}}};	//2^0 exponent
// The following is a template for a quiet nan. (MSB=1)
wire [FMSB:0] qNaN  = {1'b1,{FMSB{1'b0}}};

// -----------------------------------------------------------
// Clock #1
// - decode the input operands
// - derive basic information
// -----------------------------------------------------------

wire sa1, sb1, sc1;			// sign bit
wire [EMSB:0] xa1, xb1, xc1;	// exponent bits
wire [FMSB+1:0] fracta1, fractb1, fractc1;	// includes unhidden bit
wire a_dn1, b_dn1, c_dn1;			// a/b is denormalized
wire aNan1, bNan1, cNan1;
wire az1, bz1, cz1;
wire aInf1, bInf1, cInf1;
reg op1;

fpDecompReg #(WID) u1a (.clk(clk), .ce(ce), .i(a), .sgn(sa1), .exp(xa1), .fract(fracta1), .xz(a_dn1), .vz(az1), .inf(aInf1), .nan(aNan1) );
fpDecompReg #(WID) u1b (.clk(clk), .ce(ce), .i(b), .sgn(sb1), .exp(xb1), .fract(fractb1), .xz(b_dn1), .vz(bz1), .inf(bInf1), .nan(bNan1) );
fpDecompReg #(WID) u1c (.clk(clk), .ce(ce), .i(c), .sgn(sc1), .exp(xc1), .fract(fractc1), .xz(c_dn1), .vz(cz1), .inf(cInf1), .nan(cNan1) );

always @(posedge clk)
	if (ce) op1 <= op;

// -----------------------------------------------------------
// Clock #2
// Compute the sum of the exponents.
// correct the exponent for denormalized operands
// adjust the sum by the exponent offset (subtract 127)
// mul: ex1 = xa + xb,	result should always be < 1ffh
// Form partial products (clocks 2 to 5)
// -----------------------------------------------------------

reg abz2;
reg [EMSB+2:0] ex2;
reg [EMSB:0] xc2;
reg realOp2;

always @(posedge clk)
	if (ce) abz2 <= az1|bz1;
always @(posedge clk)
	if (ce) ex2 <= (xa1|a_dn1) + (xb1|b_dn1) - bias;
always @(posedge clk)
	if (ce) xc2 <= (xc1|c_dn1);

// Figure out which operation is really needed an add or
// subtract ?
// If the signs are the same, use the orignal op,
// otherwise flip the operation
//  a +  b = add,+
//  a + -b = sub, so of larger
// -a +  b = sub, so of larger
// -a + -b = add,-
//  a -  b = sub, so of larger
//  a - -b = add,+
// -a -  b = add,-
// -a - -b = sub, so of larger
always @(posedge clk)
	if (ce) realOp2 = op1 ^ (sa1 ^ sb1) ^ sc1;


reg [(FMSB+1)*2-1):0] fract5;
generate
if (WID==80) begin
reg [31:0] p00,p01,p02,p03;
reg [31:0] p10,p11,p12,p13;
reg [31:0] p20,p21,p22,p23;
reg [31:0] p30,p31,p32,p33;
reg [127:0] fract3a;
reg [127:0] fract3b;
reg [127:0] fract3c;
reg [127:0] fract3d;
reg [127:0] fract4a;
reg [127:0] fract4b;

	always @(posedge clk)
	if (ce) begin
		p00 <= fracta1[15: 0] * fractb1[15: 0];
		p01 <= fracta1[31:16] * fractb1[15: 0];
		p02 <= fracta1[47:32] * fractb1[15: 0];
		p03 <= fracta1[63:48] * fractb1[15: 0];
		
		p10 <= fracta1[15: 0] * fractb1[31:16];
		p11 <= fracta1[31:16] * fractb1[31:16];
		p12 <= fracta1[47:32] * fractb1[31:16];
		p13 <= fracta1[63:48] * fractb1[31:16];

		p20 <= fracta1[15: 0] * fractb1[47:32];
		p21 <= fracta1[31:16] * fractb1[47:32];
		p22 <= fracta1[47:32] * fractb1[47:32];
		p23 <= fracta1[63:48] * fractb1[47:32];

		p30 <= fracta1[15: 0] * fractb1[63:48];
		p31 <= fracta1[31:16] * fractb1[63:48];
		p32 <= fracta1[47:32] * fractb1[63:48];
		p33 <= fracta1[63:48] * fractb1[63:48];
	end
	always @(posedge clk)
	if (ce) begin
		fract3a <= {p33,p31,p20,p00};
		fract3b <= {p32,p12,p10,16'b0} + {p23,p03,p01,16'b0};
		fract3c <= {p22,p11,32'b0} + {p13,p02,32'b0};
		fract3d <= {p12,48'b0} + {p03,48'b0};
	end
	always @(posedge clk)
	if (ce) begin
		fract4a <= fract3a + fract3b;
		fract4b <= fract3c + fract3d;
	end		
	always @(posedge clk)
	if (ce) begin
		fract5 <= fract4a + fract4b;
	end
end
else if (WID==64) begin
reg [35:0] p00,p01,p02;
reg [35:0] p10,p11,p12;
reg [35:0] p20,p21,p22;
reg [71:0] fract3a;
reg [71:0] fract3b;
reg [71:0] fract3c;
reg [71:0] fract4a;
reg [71:0] fract4b;

	always @(posedge clk)
	if (ce) begin
		p00 <= fracta1[17: 0] * fractb1[17: 0];
		p01 <= fracta1[35:18] * fractb1[17: 0];
		p02 <= fracta1[52:36] * fractb1[17: 0];
		p10 <= fracta1[17: 0] * fractb1[35:18];
		p11 <= fracta1[35:18] * fractb1[35:18];
		p12 <= fracta1[52:36] * fractb1[35:18];
		p20 <= fracta1[17: 0] * fractb1[52:36];
		p21 <= fracta1[35:18] * fractb1[52:36];
		p22 <= fracta1[52:36] * fractb1[52:36];
	end
	always @(posedge clk)
	if (ce) begin
		fract3a <= {p02,p00};
		fract3b <= {p21,p10,18'b0} + {p12,p01,18'b0};
		fract3c <= {p22,p20,36'b0} + {p11,36'b0};
	end
	always @(posedge clk)
	if (ce) begin
		fract4a <= fract3a + fract3b;
		fract4b <= fract3c;
	end
	always @(posedge clk)
	if (ce) begin
		fract5 <= fract4a + fract4b;
	end
end
else if (WID==40) begin
reg [27:0] p00,p01,p02;
reg [27:0] p10,p11,p12;
reg [27:0] p20,p21,p22;
reg [53:0] fract3a;
reg [53:0] fract3b;
reg [53:0] fract3c;
reg [53:0] fract4a;
reg [53:0] fract4b;
	always @(posedge clk)
	if (ce) begin
		p00 <= fracta1[13: 0] * fractb1[13: 0];
		p01 <= fracta1[27:14] * fractb1[13: 0];
		p02 <= fracta1[39:28] * fractb1[13: 0];
		p10 <= fracta1[13: 0] * fractb1[27:14];
		p11 <= fracta1[27:14] * fractb1[27:14];
		p12 <= fracta1[39:28] * fractb1[27:14];
		p20 <= fracta1[13: 0] * fractb1[39:28];
		p21 <= fracta1[27:14] * fractb1[39:28];
		p22 <= fracta1[39:28] * fractb1[39:28];
	end
	always @(posedge clk)
	if (ce) begin
		fract3a <= {p02,p00};
		fract3b <= {p21,p10,18'b0} + {p12,p01,18'b0};
		fract3c <= {p22,p20,36'b0} + {p11,36'b0};
	end
	always @(posedge clk)
	if (ce) begin
		fract4a <= fract3a + fract3b;
		fract4b <= fract3c;
	end
	always @(posedge clk)
	if (ce) begin
		fract5 <= fract4a + fract4b;
	end
end
else if (WID==32) begin
reg [23:0] p00,p01,p02;
reg [23:0] p10,p11,p12;
reg [23:0] p20,p21,p22;
reg [47:0] fract3a;
reg [47:0] fract3b;
reg [47:0] fract4;

	always @(posedge clk)
	if (ce) begin
		p00 <= fracta1[11: 0] * fractb1[11: 0];
		p01 <= fracta1[23:12] * fractb1[11: 0];
		p10 <= fracta1[11: 0] * fractb1[23:12];
		p11 <= fracta1[23:12] * fractb1[23:12];
	end
	always @(posedge clk)
	if (ce) begin
		fract3a <= {p11,p00};
		fract3b <= {p01,12'b0} + {p10,12'b0};
	end
	always @(posedge clk)
	if (ce) begin
		fract4 <= fract3a + fract3b;
	end
	always @(posedge clk)
	if (ce) begin
		fract5 <= fract4;
	end
end
else begin
reg [(FMSB+1)*2-1:0] p00;
reg [(FMSB+1)*2-1:0] fract3;
reg [(FMSB+1)*2-1:0] fract4;
	always @(posedge clk)
    if (ce) begin
    	p00 <= fracta1 * fractb1;
    end
	always @(posedge clk)
    if (ce)
    	fract3 <= p00;
	always @(posedge clk)
    if (ce)
    	fract4 <= fract3;
	always @(posedge clk)
    if (ce)
    	fract5 <= fract4;
end
endgenerate

// -----------------------------------------------------------
// Clock #3
// Select zero exponent
// -----------------------------------------------------------

reg [EMSB+2:0] ex3;
reg [EMSB:0] xc3;
always @(posedge clk)
	if (ce) ex3 = abz2 ? 1'd0 : ex2;
always @(posedge clk)
	if (ce) xc3 <= xc2;

// -----------------------------------------------------------
// Clock #4
// Generate partial products.
// -----------------------------------------------------------

reg [EMSB+2:0] ex4;
reg [EMSB:0] xc4;

always @(posedge clk)
	if (ce) ex4 = ex3;
always @(posedge clk)
	if (ce) xc4 <= xc3;

// -----------------------------------------------------------
// Clock #5
// Sum partial products (above)
// compute multiplier overflow and underflow
// -----------------------------------------------------------

// Status
reg under5;
reg over5;
wire [EMSB:0] ex5;

wire under5, over5;
always @(posedge clk)
	if (ce) under5 <= ex4[EMSB+2];
always @(posedge clk)
	if (ce) over5 <= (&ex4[EMSB:0] | ex4[EMSB+1]) & !ex4[EMSB+2];
always @(posedge clk)
	if (ce) ex5 <= ex4[EMSB:0];
always @(posedge clk)
	if (ce) xc5 <= xc4;

delay4 u2a (.clk(clk), .ce(ce), .i(aInf1), .o(aInf5) );
delay4 u2b (.clk(clk), .ce(ce), .i(bInf1), .o(bInf5) );

// determine when a NaN is output
wire qNaNOut5;
wire [WID-1:0] a5,b5;
delay4 u5 (.clk(clk), .ce(ce), .i((aInf1&bz1)|(bInf1&az1)), .o(qNaNOut5) );
delay4 u14 (.clk(clk), .ce(ce), .i(aNan1), .o(aNan5) );
delay4 u15 (.clk(clk), .ce(ce), .i(bNan1), .o(bNan5) );
delay5 #(WID) u16 (.clk(clk), .ce(ce), .i(a), .o(a5) );
delay5 #(WID) u17 (.clk(clk), .ce(ce), .i(b), .o(b5) );

// -----------------------------------------------------------
// Clock #6
// - figure multiplier mantissa output
// - figure multiplier exponent output
// - correct xponent and mantissa for exceptional conditions
// -----------------------------------------------------------

reg [(FMSB+1)*2+2):0] mo6;
reg [EMSB:0] ex6;
reg [EMSB:0] xc6;
reg exinf6;
wire [FMSB+1] fractc6;
delay5 #(FMSB+2) u61 (.clk(clk), .ce(ce), .i(fractc1), .o(fractc6) );
always @(posedge clk)
	if (ce) xc6 <= xc5;

always @(posedge clk)
	if (ce)
		casez({aNan5,bNan5,qNaNOut5,aInf5,bInf5,over5})
		6'b1?????:  mo6 = {1'b1,a5[FMSB:0],{FMSB+1{1'b0}}};
    6'b01????:  mo6 = {1'b1,b5[FMSB:0],{FMSB+1{1'b0}}};
		6'b001???:	mo6 = {1'b1,qNaN|3'd4,{FMSB+1{1'b0}}};	// multiply inf * zero
		6'b0001??:	mo6 = 0;	// mul inf's
		6'b00001?:	mo6 = 0;	// mul inf's
		6'b000001:	mo6 = 0;	// mul overflow
		default:	mo6 = fract5;
		endcase

always @(posedge clk)
	if (ce)
		casez({qNaNOut5|aNan5|bNan5,aInf5,bInf5,over5,under5})
		5'b1????:	ex6 = infXp;	// qNaN - infinity * zero
		5'b01???:	ex6 = infXp;	// 'a' infinite
		5'b001??:	ex6 = infXp;	// 'b' infinite
		5'b0001?:	ex6 = infXp;	// result overflow
		5'b00001:	ex6 = ex5[EMSB:0];//0;		// underflow
		default:	ex6 = ex5[EMSB:0];	// situation normal
		endcase

always @(posedge clk)
	if (ce)
		casez({qNaNOut5|aNan5|bNan5,aInf5,bInf5,over5,under5})
		5'b1????:	exinf6 = 1'b1;	// qNaN - infinity * zero
		5'b01???:	exinf6 = 1'b1;	// 'a' infinite
		5'b001??:	exinf6 = 1'b1;	// 'b' infinite
		5'b0001?:	exinf6 = 1'b1;	// result overflow
		5'b00001:	exinf6 = |ex5[EMSB:0];//0;		// underflow
		default:	exinf6 = |ex5[EMSB:0];	// situation normal
		endcase

// -----------------------------------------------------------
// Clock #7
// - prep for addition, determine greater operand
// -----------------------------------------------------------
reg ex_gt_xc7;
reg xeq7;
reg ma_gt_mc7;
reg meq7;
reg exinf7;
wire az7, bz7, cz7;

always @(posedge clk)
	if (ce) exinf7 <= exinf6;
// which has greater magnitude ? Used for sign calc
always @(posedge clk)
	if (ce) ex_gt_xc7 = ex6 > xc6;
always @(posedge clk)
	if (ce) xeq7 = ex6==xc6;
always @(posedge clk)
	if (ce) ma_gt_mc7 = mo6 > {fractc6,{FMSB+1{1'b0}}};
always @(posedge clk)
	if (ce) meq7 = mo6 == {fractc6,{FMSB+1{1'b0}}};
vtdl u71 (.clk(clk), .ce(ce), .a(4'd6), .d(az1), .q(az7));
vtdl u72 (.clk(clk), .ce(ce), .a(4'd6), .d(bz1), .q(bz7));
vtdl u73 (.clk(clk), .ce(ce), .a(4'd6), .d(cz1), .q(cz7));

// -----------------------------------------------------------
// Clock #8
// - prep for addition, determine greater operand
// - determine if result will be zero
// -----------------------------------------------------------

reg a_gt_b8;
reg resZero8;
reg ex_gt_xc8;
wire [EMSB:0] ex8;
wire [EMSB:0] xc8;
reg exinf8;
wire xcInf8;
wire [2:0] rm8;

delay2 #(EMSB+1) u81 (.clk(clk), .ce(ce), .i(ex6), .o(ex8));
delay2 #(EMSB+1) u82 (.clk(clk), .ce(ce), .i(xc6), .o(xc8));
vtdl u83 (.clk(clk), .ce(ce), .a(4'd7), .d(xcInf1), .q(xcInf8));
vtdl #(3) u84 (.clk(clk), .ce(ce), .a(4'd8), .d(rm), .q(rm8));

always @(posedge clk)
	if (ce) exinf8 <= exinf7;
always @(posedge clk)
	if (ce) ex_gt_xc8 = ex_gt_xc7;
always @(posedge clk)
	if (ce)
		a_gt_b8 = ex_gt_xc7 || (xeq7 && ma_gt_mc7);

// Find out if the result will be zero.
always @(posedge clk)
	if (ce)
		resZero8 = (realOp7 & xeq7 & meq7) ||	// subtract, same magnitude
			   (az7 | bz7) & cz7;		// a or b zero and c zero

// -----------------------------------------------------------
// CLock #9
// Compute output exponent and sign
//
// The output exponent is the larger of the two exponents,
// unless a subtract operation is in progress and the two
// numbers are equal, in which case the exponent should be
// zero.
// -----------------------------------------------------------

reg so9;
reg [EMSB:0] ex9;
reg ex_gt_xc9;
wire sa8, sb8;
reg [EMSB:0] xc9;
wire [(FMSB+1)*2+2:0] mo9, fractc9;

always @(posedge clk)
	if (ce) ex_gt_xc9 = ex_gt_xc8;
always @(posedge clk)
	if (ce) xc9 <= xc8;
vtdl u91 (.clk(clk), .ce(ce), .a(4'd7), .d(sa1 ^ sb1), .q(sa8));
vtdl u92 (.clk(clk), .ce(ce), .a(4'd7), .d(sc1), .q(sc8));
delay3 #(((FMSB+1)*2)+3) u93 (.clk(clk), .ce(ce), .i(mo6), .o(mo9));
delay3 #(FMSB+1) u93 (.clk(clk), .ce(ce), .i(fractc6), .o(fractc9));

always @(posedge clk)
	if (ce) ex9 = (exinf8&xcInf8) ? ex8 : resZero8 ? 0 : ex_gt_xc8 ? ex8 : xc8;

// Compute output sign
always @*
	case ({resZero8,sa8,op8,sc8})	// synopsys full_case parallel_case
	4'b0000: so9 <= 0;			// + + + = +
	4'b0001: so9 <= !a_gt_b8;	// + + - = sign of larger
	4'b0010: so9 <= !a_gt_b8;	// + - + = sign of larger
	4'b0011: so9 <= 0;			// + - - = +
	4'b0100: so9 <= a_gt_b8;		// - + + = sign of larger
	4'b0101: so9 <= 1;			// - + - = -
	4'b0110: so9 <= 1;			// - - + = -
	4'b0111: so9 <= a_gt_b8;		// - - - = sign of larger
	4'b1000: so9 <= 0;			//  A +  B, sign = +
	4'b1001: so9 <= rm8==3;		//  A + -B, sign = + unless rounding down
	4'b1010: so9 <= rm8==3;		//  A -  B, sign = + unless rounding down
	4'b1011: so9 <= 0;			// +A - -B, sign = +
	4'b1100: so9 <= rm8==3;		// -A +  B, sign = + unless rounding down
	4'b1101: so9 <= 1;			// -A + -B, sign = -
	4'b1110: so9 <= 1;			// -A - +B, sign = -
	4'b1111: so9 <= rm8==3;		// -A - -B, sign = + unless rounding down
	endcase

// -----------------------------------------------------------
// Clock #10
// Compute the difference in exponents, provides shift amount
// -----------------------------------------------------------
reg [EMSB:0] xdiff10;
reg [(FMSB+1)*2+2:0] mfs;

always @(posedge clk)
	if (ce) xdiff10 = ex_gt_xc9 ? ex9 - xc9 : xc9 - ex9;

// determine which fraction to denormalize
always @(posedge clk)
	if (ce) mfs = ex_gt_xc9 ? {fractc9,{FMSB+1{1'b0}}} : mo9;

// -----------------------------------------------------------
// Clock #11
// -----------------------------------------------------------
reg [6:0] xdif11;

always @(posedge clk)
	if (ce) xdif11 = xdiff10 > FMSB+3 ? FMSB+3 : xdiff10;

// -----------------------------------------------------------
// Clock #12
// Determine the sticky bit
// -----------------------------------------------------------

wire sticky, sticky12;
wire [(FMSB+1)*2+2:0] mfs12;

generate
begin
if (WID==128)
    redor128 u1 (.a(xdif11), .b({mfs,2'b0}), .o(sticky) );
else if (WID==96)
    redor96 u1 (.a(xdif11), .b({mfs,2'b0}), .o(sticky) );
else if (WID==80)
    redor80 u1 (.a(xdif11), .b({mfs,2'b0}), .o(sticky) );
else if (WID==64)
    redor64 u1 (.a(xdif11), .b({mfs,2'b0}), .o(sticky) );
else if (WID==32)
    redor32 u1 (.a(xdif11), .b({mfs,2'b0}), .o(sticky) );
end
endgenerate

// register inputs to shifter and shift
delay1 #(1)      d16(.clk(clk), .ce(ce), .i(sticky), .o(sticky12) );
delay1 #(7)      d15(.clk(clk), .ce(ce), .i(xdif11),   .o(xdif12) );
delay1 #((FMSB+1)*2+3) d14(.clk(clk), .ce(ce), .i(mfs), .o(mfs12) );

// -----------------------------------------------------------
// Clock #13
// - denormalize operand
// -----------------------------------------------------------
wire [(FMSB+1)*2+2:0] mfs13;
wire [(FMSB+1)*2+2:0] mo13;
wire ex_gt_xc13;

delay4 #(((FMSB+1)*2)+3) u131 (.clk(clk), .ce(ce), .i(mo9), .o(mo13));
delay4 u132 (.clk(clk), .ce(ce), .i(ex_gt_xc9), .o(ex_gt_xc13));

always @(posedge clk)
	if (ce) mfs13 = ({mfs12,2'b0} >> xdif12)|sticky12;

// -----------------------------------------------------------
// Clock #14
// Sort operands
// -----------------------------------------------------------
wire [(FMSB+1)*2+2:0] oa, ob;
wire a_gt_b14;

vtdl u141 (.clk(clk), .ce(ce), .a(4'd5), .d(a_gt_b8), .q(a_gt_b14));

always @(posedge clk)
	if (ce) oa = ex_gt_xc13 ? {mo13,2'b00} : mfs13;
always @(posedge clk)
	if (ce) ob = ex_gt_xc13 ? mfs13 : {mo13,2'b00};

// -----------------------------------------------------------
// Clock #15
// - Sort operands
// -----------------------------------------------------------
reg [(FMSB+1)*2+2:0] oaa, obb, mab;
wire realOp15;

vtdl u151 (.clk(clk), .ce(ce), .a(4'd7), .d(realOp7), .q(realOp15));

always @(posedge clk)
	if (ce) oaa = a_gt_b14 ? oa : ob;
always @(posedge clk)
	if (ce) obb = a_gt_b14 ? ob : oa;

// -----------------------------------------------------------
// Clock #16
// - perform add/subtract
// - addition can generate an extra bit, subtract can't go negative
// -----------------------------------------------------------
reg [(FMSB+1)*2+2:0] mab, mo16;
wire [FMSB+1:0] fractc16;
wire Nan16;
wire cNan16;
wire aInf16, cInf16;
wire op16;

vtdl u161 (.clk(clk), .ce(ce), .a(4'd10), .d(qNanOut5|aNan5|bNan5), .q(Nan16));
vtdl u162 (.clk(clk), .ce(ce), .a(4'd14), .d(cNan1), .q(cNan16));
vtdl u163 (.clk(clk), .ce(ce), .a(4'd9), .d(exinf6), .q(aInf16));
vtdl u164 (.clk(clk), .ce(ce), .a(4'd14), .d(cInf1), .q(cInf16));
vtdl u165 (.clk(clk), .ce(ce), .a(4'd14), .d(op1), .q(op16));
delay3 #(((FMSB+1)*2)+3) u166 (.clk(clk), .ce(ce), .i(mo13), .o(mo16));
vtdl #(FMSB+2) u167 (.clk(clk), .ce(ce), .a(4'd6), .d(fractc9), .q(fractc16));

always @(posedge clk)
	if (ce) mab = realOp15 ? oaa - obb : oaa + obb;

// -----------------------------------------------------------
// Clock #17
// - adjust for Nans
// -----------------------------------------------------------
reg [(FMSB+1)*2+2:0] mo17;
wire so17;

vtdl           u171 (.clk(clk), .ce(ce), .a(4'd5), .d(so9), .q(so17));
vtdl #(EMSB+1) u172 (.clk(clk), .ce(ce), .a(4'd5), .d(ex9), .q(ex17));

always @*
	casez({aInf16&cInf16,Nan16,cNan16})
	3'b1??:		mo17 = {1'b0,op16,{FMSB-1{1'b0}},op16,{FMSB{1'b0}}};	// inf +/- inf - generate QNaN on subtract, inf on add
	3'b01?:		mo17 = {1'b0,mo16};
	3'b001: 	mo17 = {1'b0,fractc16[FMSB+1:0],{FMSB{1'b0}}};
	default:	mo17 = mab;		// mab has an extra lead bit and two trailing bits
	endcase

assign o = {so17,ex17,mo17};

vtdl u173 (.clk(clk), .ce(ce), .a(4'd11), .d(over5),  .q(overflow) );
vtdl u174 (.clk(clk), .ce(ce), .a(4'd11), .d(over5),  .q(inf) );
vtdl u175 (.clk(clk), .ce(ce), .a(4'd11), .d(under5), .q(underflow) );

endmodule


// Multiplier with normalization and rounding.

module fpFMAnr(clk, ce, a, b, c, o, rm, sign_exe, inf, overflow, underflow);
parameter WID=32;
localparam MSB = WID-1;
localparam EMSB = WID==128 ? 14 :
                  WID==96 ? 14 :
                  WID==80 ? 14 :
                  WID==64 ? 10 :
				  WID==52 ? 10 :
				  WID==48 ? 11 :
				  WID==44 ? 10 :
				  WID==42 ? 10 :
				  WID==40 ?  9 :
				  WID==32 ?  7 :
				  WID==24 ?  6 : 4;
localparam FMSB = WID==128 ? 111 :
                  WID==96 ? 79 :
                  WID==80 ? 63 :
                  WID==64 ? 51 :
				  WID==52 ? 39 :
				  WID==48 ? 34 :
				  WID==44 ? 31 :
				  WID==42 ? 29 :
				  WID==40 ? 28 :
				  WID==32 ? 22 :
				  WID==24 ? 15 : 9;

localparam FX = (FMSB+2)*2-1;	// the MSB of the expanded fraction
localparam EX = FX + 1 + EMSB + 1 + 1 - 1;
input clk;
input ce;
input  [MSB:0] a, b;
output [MSB:0] o;
input [2:0] rm;
output sign_exe;
output inf;
output overflow;
output underflow;

wire [EX:0] o1;
wire sign_exe1, inf1, overflow1, underflow1;
wire [MSB+3:0] fpn0;

fpFMA       #(WID) u1 (clk, ce, op, rm, a, b, c, o1, inf1, overflow1, underflow1);
fpNormalize #(WID) u2(.clk(clk), .ce(ce), .under(underflow1), .i(o1), .o(fpn0) );
fpRoundReg  #(WID) u3(.clk(clk), .ce(ce), .rm(rm), .i(fpn0), .o(o) );
delay2      #(1)   u4(.clk(clk), .ce(ce), .i(sign_exe1), .o(sign_exe));
delay2      #(1)   u5(.clk(clk), .ce(ce), .i(inf1), .o(inf));
delay2      #(1)   u6(.clk(clk), .ce(ce), .i(overflow1), .o(overflow));
delay2      #(1)   u7(.clk(clk), .ce(ce), .i(underflow1), .o(underflow));
endmodule

