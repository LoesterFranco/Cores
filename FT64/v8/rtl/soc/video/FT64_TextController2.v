// ============================================================================
//        __
//   \\__/ o\    (C) 2006-2018  Robert Finch, Waterloo
//    \  __ /    All rights reserved.
//     \/_//     robfinch<remove>@finitron.ca
//       ||
//
//	FT64_TextController2.v
//		text controller
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
//	Text Controller
//
//	FEATURES
//
//	This core requires an external timing generator to provide horizontal
//	and vertical sync signals, but otherwise can be used as a display
//  controller on it's own. However, this core may also be embedded within
//  another core such as a VGA controller.
//
//	Window positions are referenced to the rising edge of the vertical and
//	horizontal sync pulses.
//
//	The core includes an embedded dual port RAM to hold the screen
//	characters.
//
//
//--------------------------------------------------------------------
// Registers
//
// 00h
//	7 - 0		         cccccccc  number of columns (horizontal displayed number of characters)
//	15- 8		         rrrrrrrr	 number of rows (vertical displayed number of characters)
//  19-16                dddd  character output delay
//	43-32       nnnn nnnnnnnn  window left       (horizontal sync position - reference for left edge of displayed)
//	59-48       nnnn nnnnnnnn  window top        (vertical sync position - reference for the top edge of displayed)
// 01h
//	 4- 0               nnnnn  maximum scan line (char ROM max value is 7)
//  11- 8							   wwww	 pixel size - width 
//  15-12							   hhhh	 pixel size - height 
//  24                      r  reset state bit
// 02h
//	15- 0   cccccccc cccccccc  color code for transparent background
// 03h
//   4- 0               eeeee	 cursor end
//   7- 5                 bbb  blink control
//                             BP: 00=no blink
//                             BP: 01=no display
//                             BP: 10=1/16 field rate blink
//                             BP: 11=1/32 field rate blink
//  12- 8               sssss  cursor start
//  15-14									 tt	 cursor image type (box, underline, sidebar, asterisk
//  47-32   aaaaaaaa aaaaaaaa	 cursor position
// 04h
//  15- 0   aaaaaaaa aaaaaaaa  start address (index into display memory)
// 07h
//	150 - - aaaaaaaa aaaaaaaa  light pen position
//--------------------------------------------------------------------
//
// ============================================================================

//`define USE_CLOCK_GATE

module FT64_TextController2(
	rst_i, clk_i, cs_i,
	cyc_i, stb_i, ack_o, wr_i, sel_i, adr_i, dat_i, dat_o,
	lp_i,
	dot_clk_i, hsync_i, vsync_i, blank_i, border_i, zrgb_i, zrgb_o, xonoff_i
);
parameter num = 4'd1;
parameter COLS = 8'd48;
parameter ROWS = 8'd35;

// Syscon
input  rst_i;			// reset
input  clk_i;			// clock

// Slave signals
input  cs_i;            // circuit select
input  cyc_i;			// valid bus cycle
input  stb_i;           // data strobe
output ack_o;			// data acknowledge
input  wr_i;			// write
input  [ 7:0] sel_i;	// byte lane select
input  [15:0] adr_i;	// address
input  [63:0] dat_i;	// data input
output [63:0] dat_o;	// data output
reg    [63:0] dat_o;

input lp_i;				// light pen

// Video signals
input dot_clk_i;		// video dot clock
input hsync_i;			// end of scan line
input vsync_i;			// end of frame
input blank_i;			// blanking signal
input border_i;			// border area
input [31:0] zrgb_i;		// input pixel stream
output reg [31:0] zrgb_o;	// output pixel stream
input xonoff_i;

reg controller_enable;
reg [31:0] bkColor32, bkColor32d;	// background color
reg [31:0] fgColor32, fgColor32d;	// foreground color
wire [23:0] tcColor24;	// transparent color

wire pix;				// pixel value from character generator 1=on,0=off

reg por;
wire vclk;
reg [15:0] rego;
reg [11:0] windowTop;
reg [11:0] windowLeft;
reg [ 7:0] numCols;
reg [ 7:0] numRows;
reg [11:0] charOutDelay;
reg [ 1:0] mode;
reg [ 4:0] maxScanline;
reg [ 4:0] maxScanpix;
reg [ 4:0] cursorStart, cursorEnd;
reg [15:0] cursorPos;
reg [1:0] cursorType;
reg [15:0] startAddress;
reg [ 2:0] rBlink;
reg [ 3:0] bdrColorReg;
reg [ 3:0] pixelWidth;	// horizontal pixel width in clock cycles
reg [ 3:0] pixelHeight;	// vertical pixel height in scan lines

wire [11:0] hctr;		// horizontal reference counter (counts clocks since hSync)
wire [11:0] scanline;	// scan line
wire [ 7:0] row;		// vertical reference counter (counts rows since vSync)
wire [ 7:0] col;		// horizontal column
reg  [ 4:0] rowscan;	// scan line within row
wire nxt_row;			// when to increment the row counter
wire nxt_col;			// when to increment the column counter
wire [ 5:0] bcnt;		// blink timing counter
wire blink;
reg  iblank;

wire nhp;				// next horizontal pixel
wire ld_shft = nxt_col & nhp;


// display and timing signals
reg [15:0] txtAddr;		// index into memory
reg [15:0] penAddr;
wire [63:0] screen_ram_out;		// character code
wire [8:0] char_bmp;		// character ROM output
wire [15:0] txtBkColor;	// background color code
wire [15:0] txtFgColor;	// foreground color code
wire [7:0] txtZorder;
reg  [15:0] txtTcCode;	// transparent color code
reg  bgt, bgtd;

wire [63:0] tdat_o;
wire [8:0] chdat_o;

wire [2:0] scanindex = scanline[2:0];

//--------------------------------------------------------------------
// bus interfacing
// Address Decoding
// I/O range Dx
//--------------------------------------------------------------------
// Register the inputs
reg cs_rom, cs_reg, cs_text, cs_any;
reg [15:0] radr_i;
reg [63:0] rdat_i;
reg rwr_i;
reg [7:0] rsel_i;
always @(posedge clk_i)
	cs_rom <= cs_i && cyc_i && stb_i && (adr_i[15:8] >= 8'hE0);
always @(posedge clk_i)
	cs_reg <= cs_i && cyc_i && stb_i && (adr_i[15:8] == 8'hDF);
always @(posedge clk_i)
	cs_text <= cs_i && cyc_i && stb_i && (adr_i[15:8] < 8'hDF);
always @(posedge clk_i)
	cs_any <= cs_i && cyc_i && stb_i;
always @(posedge clk_i)
	rwr_i <= wr_i;
always @(posedge clk_i)
	rsel_i <= sel_i;
always @(posedge clk_i)
	radr_i <= adr_i;
always @(posedge clk_i)
	rdat_i <= dat_i;	

// Register outputs
always @(posedge clk_i)
	if (cs_text) dat_o <= tdat_o;
	else if (cs_rom) dat_o <= {55'd0,chdat_o};
	else if (cs_reg) dat_o <= {32'd0,rego};
	else dat_o <= 64'h0;

//always @(posedge clk_i)
//	if (cs_text) begin
//		$display("TC WRite: %h %h", adr_i, dat_i);
//		$stop;
//	end

// - there is a four cycle latency for reads, an ack is generated
//   after the synchronous RAM read
// - writes can be acknowledged right away.

ack_gen #(
	.READ_STAGES(4),
	.WRITE_STAGES(0),
	.REGISTER_OUTPUT(1)
)
uag1 (
	.clk_i(clk_i),
	.ce_i(1'b1),
	.i(cs_any),
	.we_i(rwr_i),
	.o(ack_o)
);

//--------------------------------------------------------------------
//--------------------------------------------------------------------
`ifdef USE_CLOCK_GATE
BUFHCE ucb1 (.I(dot_clk_i), .CE(controller_enable), .O(vclk));
`else
assign vclk = dot_clk_i;
`endif

//--------------------------------------------------------------------
// Video Memory
//--------------------------------------------------------------------
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Address Calculation:
//  - Simple: the row times the number of  cols plus the col plus the
//    base screen address
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

reg [15:0] rowcol;
always @(posedge vclk)
	rowcol = row * numCols;
always @(posedge vclk)
	txtAddr <= startAddress + rowcol + col;

// Register read-back memory
wire [3:0] rrm_adr = radr_i[6:3];
wire [63:0] rrm_o;

regReadbackMem #(.WID(16)) rrm0
(
  .wclk(clk_i),
  .adr(rrm_adr),
  .wce(cs_reg),
  .we(rwr_i & |rsel_i[1:0]),
  .i(rdat_i[15:0]),
  .o(rrm_o[15:0])
);

regReadbackMem #(.WID(16)) rrm1
(
  .wclk(clk_i),
  .adr(rrm_adr),
  .wce(cs_reg),
  .we(rwr_i & |rsel_i[3:2]),
  .i(rdat_i[31:16]),
  .o(rrm_o[31:16])
);

regReadbackMem #(.WID(16)) rrm2
(
  .wclk(clk_i),
  .adr(rrm_adr),
  .wce(cs_reg),
  .we(rwr_i & |rsel_i[5:4]),
  .i(rdat_i[47:32]),
  .o(rrm_o[47:32])
);

regReadbackMem #(.WID(16)) rrm3
(
  .wclk(clk_i),
  .adr(rrm_adr),
  .wce(cs_reg),
  .we(rwr_i & |rsel_i[7:6]),
  .i(rdat_i[63:48]),
  .o(rrm_o[63:48])
);

wire [23:0] lfsr1_o;
lfsr #(24) ulfsr1(rst_i, dot_clk_i, 1'b1, 1'b0, lfsr1_o);
wire [63:0] lfsr_o = {16'h0,
												lfsr1_o[23:21],2'b0,lfsr1_o[20:18],3'b0,lfsr1_o[17:16],3'b0,
												lfsr1_o[15:13],2'b0,lfsr1_o[12:10],3'b0,lfsr1_o[9:8],3'b0,
												8'h00,lfsr1_o[7:0]
										};

// text screen RAM
wire [12:0] bram_adr = radr_i[15:3];
syncRam8kx64 screen_ram1	// This ram is really only 56k
(
  .clka(clk_i),
  .ena(cs_text),
  .wea({8{rwr_i}}&rsel_i),
  .addra(bram_adr),  // input wire [11 : 0] addra
  .dina(rdat_i),
  .douta(tdat_o),
  .clkb(vclk),
  .enb(ld_shft|por),
  .web({8{por}}),
  .addrb(txtAddr[12:0]),
  .dinb(lfsr_o),
  .doutb(screen_ram_out)
);

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Character bitmap ROM
// - room for 512 characters
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
char_ram charRam0
(
	.clk_i(clk_i),
	.cs_i(cs_rom),
	.we_i(1'b0),
	.adr_i(bram_adr),
	.dat_i(rdat_i[8:0]),
	.dat_o(chdat_o),
	.dot_clk_i(vclk),
	.ce_i(ld_shft),
	.char_code_i(screen_ram_out[8:0]),
	.scanline_i(rowscan[2:0]),
	.bmp_o(char_bmp)
);
/*
syncRam4kx9 charRam0
(
  .clka(clk_i),    // input wire clka
  .ena(cs_rom),      // input wire ena
  .wea(1'b0),//rwr_i),      // input wire [0 : 0] wea
  .addra(bram_adr),  // input wire [11 : 0] addra
  .dina(rdat_i[8:0]),    // input wire [8 : 0] dina
  .douta(chdat_o),  // output wire [8 : 0] douta
  .clkb(vclk),    // input wire clkb
  .enb(ld_shft),      // input wire enb
  .web(1'b0),      // input wire [0 : 0] web
  .addrb({screen_ram_out[8:0],rowscan[2:0]}),  // input wire [11 : 0] addrb
  .dinb(9'h0),    // input wire [8 : 0] dinb
  .doutb(char_bmp)  // output wire [8 : 0] doutb
);
*/

// pipeline delay - sync color with character bitmap output
reg [15:0] txtBkCode1;
reg [15:0] txtFgCode1;
reg [7:0] txtZorder1;
always @(posedge vclk)
	if (ld_shft) txtBkCode1 <= screen_ram_out[31:16];
always @(posedge vclk)
	if (ld_shft) txtFgCode1 <= screen_ram_out[47:32];
always @(posedge vclk)
	if (ld_shft) txtZorder1 <= screen_ram_out[55:48];

//--------------------------------------------------------------------
// Light Pen
//--------------------------------------------------------------------
wire lpe;
edge_det u1 (.rst(rst_i), .clk(clk_i), .ce(1'b1), .i(lp_i), .pe(lpe), .ne(), .ee() );

always @(posedge clk_i)
	if (rst_i)
		penAddr <= 32'h0000_0000;
	else begin
		if (lpe)
			penAddr <= txtAddr;
	end


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Register read port
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
always @*
	if (cs_reg) begin
		case(rrm_adr)
		4'd7:		  rego <= penAddr;
		default:	rego <= rrm_o;
		endcase
	end
	else
		rego <= 32'h0000;


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Register write port
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

always @(posedge clk_i)
	if (rst_i) begin
	  por <= 1'b1;
	  controller_enable <= 1'b1;
// 104x63
/*
		windowTop    <= 12'd26;
		windowLeft   <= 12'd260;
		pixelWidth   <= 4'd0;
		pixelHeight  <= 4'd1;		// 525 pixels (408 with border)
*/
// 52x31
/*
		// 84x47
		windowTop    <= 12'd16;
		windowLeft   <= 12'd90;
		pixelWidth   <= 4'd1;		// 681 pixels
		pixelHeight  <= 4'd1;		// 384 pixels
*/
		// 56x31
		if (num==4'd1) begin
      windowTop    <= 12'd18;//12'd16;
      windowLeft   <= 12'h51;//12'd86;
      pixelWidth   <= 4'd1;		// 800 pixels
      pixelHeight  <= 4'd1;		// 600 pixels
      numCols      <= COLS;
      numRows      <= ROWS;
      maxScanline  <= 5'd7;
      maxScanpix   <= 5'd7;
      rBlink       <= 3'b111;		// 01 = non display
      startAddress <= 16'h0000;
      cursorStart  <= 5'd00;
      cursorEnd    <= 5'd31;
      cursorPos    <= 16'h0003;
      cursorType 	 <= 2'b00;
      txtTcCode    <= 16'h1ff;
      charOutDelay <= 12'd3;
		end
		else if (num==4'd2) begin
      windowTop    <= 12'd64;//12'd16;
      windowLeft   <= 12'h376;//12'd86;
      pixelWidth   <= 4'd0;        // 800 pixels
      pixelHeight  <= 4'd0;        // 600 pixels
      numCols      <= 40;
      numRows      <= 25;
      maxScanline  <= 5'd7;
      maxScanpix   <= 5'd7;
      rBlink       <= 3'b111;        // 01 = non display
      startAddress <= 16'h0000;
      cursorStart  <= 5'd00;
      cursorEnd    <= 5'd31;
      cursorPos    <= 16'h0003;
      cursorType   <= 2'b00;
      txtTcCode    <= 9'h1ff;
      charOutDelay <= 12'd6;
		end
	end
	else begin
		
		if (bcnt > 6'd10)
			por <= 1'b0;
		
		if (cs_reg & rwr_i) begin	// register write ?
			$display("TC Write: r%d=%h", rrm_adr, rdat_i);
			case(rrm_adr)
			4'd0:	begin
					if (rsel_i[0]) numCols    <= rdat_i[7:0];
					if (rsel_i[1]) numRows    <= rdat_i[15:8];
					if (rsel_i[2]) charOutDelay <= rdat_i[19:16];
					if (|rsel_i[5:4]) windowLeft <= rdat_i[43:32];
					if (|rsel_i[7:6]) windowTop  <= rdat_i[59:48];
					end
			4'd1:
				begin
					if (rsel_i[0]) maxScanline <= rdat_i[4:0];
					if (rsel_i[1]) begin
						pixelHeight <= rdat_i[15:12];
						pixelWidth  <= rdat_i[11:8];	// horizontal pixel width
					end
					if (rsel_i[3]) por <= rdat_i[24];
					if (rsel_i[4]) controller_enable <= rdat_i[32];
				end
			4'd2:	if (|rsel_i[1:0]) txtTcCode <= rdat_i[15:0];
			4'd3:	
				begin
					if (rsel_i[0]) begin
						cursorEnd <= rdat_i[4:0];	// scan line sursor starts on
						rBlink      <= rdat_i[7:5];
					end
					if (rsel_i[1]) begin
						cursorStart <= rdat_i[12:8];	// scan line cursor ends on
						cursorType  <= rdat_i[15:14];
					end
					if (|rsel_i[7:4]) cursorPos <= rdat_i[47:32];
				end
			4'd4:	if (|rsel_i[3:0]) startAddress <= rdat_i[15:0];
			default: ;
			endcase
		end
	end


//--------------------------------------------------------------------
//--------------------------------------------------------------------

// "Box" cursor bitmap
reg [7:0] curout;
always @*
	case({cursorType,scanindex})
	// Box cursor
	5'b00_000:	curout = 8'b11111111;
	5'b00_001:	curout = 8'b10000001;
	5'b00_010:	curout = 8'b10000001;
	5'b00_011:	curout = 8'b10000001;
	5'b00_100:	curout = 8'b10000001;
	5'b00_101:	curout = 8'b10000001;
	5'b00_110:	curout = 8'b10011001;
	5'b00_111:	curout = 8'b11111111;
	// vertical bar cursor
	5'b01_000:	curout = 8'b11000000;
	5'b01_001:	curout = 8'b10000000;
	5'b01_010:	curout = 8'b10000000;
	5'b01_011:	curout = 8'b10000000;
	5'b01_100:	curout = 8'b10000000;
	5'b01_101:	curout = 8'b10000000;
	5'b01_110:	curout = 8'b10000000;
	5'b01_111:	curout = 8'b11000000;
	// underline cursor
	5'b10_000:	curout = 8'b00000000;
	5'b10_001:	curout = 8'b00000000;
	5'b10_010:	curout = 8'b00000000;
	5'b10_011:	curout = 8'b00000000;
	5'b10_100:	curout = 8'b00000000;
	5'b10_101:	curout = 8'b00000000;
	5'b10_110:	curout = 8'b00000000;
	5'b10_111:	curout = 8'b11111111;
	// Asterisk
	5'b11_000:	curout = 8'b00000000;
	5'b11_001:	curout = 8'b00000000;
	5'b11_010:	curout = 8'b00100100;
	5'b11_011:	curout = 8'b00011000;
	5'b11_100:	curout = 8'b01111110;
	5'b11_101:	curout = 8'b00011000;
	5'b11_110:	curout = 8'b00100100;
	5'b11_111:	curout = 8'b00000000;
	endcase


//-------------------------------------------------------------
// Video Stuff
//-------------------------------------------------------------

wire pe_hsync;
wire pe_vsync;
edge_det edh1
(
	.rst(rst_i),
	.clk(vclk),
	.ce(1'b1),
	.i(hsync_i),
	.pe(pe_hsync),
	.ne(),
	.ee()
);

edge_det edv1
(
	.rst(rst_i),
	.clk(vclk),
	.ce(1'b1),
	.i(vsync_i),
	.pe(pe_vsync),
	.ne(),
	.ee()
);

// Horizontal counter:
//
HVCounter uhv1
(
	.rst(rst_i),
	.vclk(vclk),
	.pixcce(1'b1),
	.sync(hsync_i),
	.cnt_offs(windowLeft),
	.pixsz(pixelWidth),
	.maxpix(maxScanpix),
	.nxt_pix(nhp),
	.pos(col),
	.nxt_pos(nxt_col),
	.ctr(hctr)
);


// Vertical counter:
//
HVCounter uhv2
(
	.rst(rst_i),
	.vclk(vclk),
	.pixcce(pe_hsync),
	.sync(vsync_i),
	.cnt_offs(windowTop),
	.pixsz(pixelHeight),
	.maxpix(maxScanline),
	.nxt_pix(),
	.pos(row),
	.nxt_pos(nxt_row),
	.ctr(scanline)
);

// Takes 3 clock for rowscan to become stable, but should be stable before any
// chars are displayed.
reg [4:0] maxScanlinePlusOne;
reg [13:0] rxmslp1;
always @(posedge vclk)
	maxScanlinePlusOne <= maxScanline + 1;
always @(posedge vclk)
	rxmslp1 <= row * maxScanlinePlusOne;
always @(posedge vclk)
	rowscan <= scanline - rxmslp1;


// Blink counter
//
VT163 #(6) ub1
(
	.clk(vclk),
	.clr_n(!rst_i),
	.ent(pe_vsync),
	.enp(1'b1),
	.ld_n(1'b1),
	.d(6'd0),
	.q(bcnt),
	.rco()
);

reg blink_en;
always @(posedge vclk)
	blink_en <= (cursorPos+2==txtAddr) && (scanline[4:0] >= cursorStart) && (scanline[4:0] <= cursorEnd);

VT151 ub2
(
	.e_n(!blink_en),
	.s(rBlink),
	.i0(1'b1), .i1(1'b0), .i2(bcnt[4]), .i3(bcnt[5]),
	.i4(1'b1), .i5(1'b0), .i6(bcnt[4]), .i7(bcnt[5]),
	.z(blink),
	.z_n()
);

always @(posedge vclk)
	if (ld_shft)
		bkColor32 <= {txtZorder1,txtBkCode1[15:11],3'h0,txtBkCode1[10:5],2'h0,txtBkCode1[4:0],3'h0};
always @(posedge vclk)
	if (nhp)
		bkColor32d <= bkColor32;
always @(posedge vclk)
	if (ld_shft)
		fgColor32 <= {txtZorder1,txtFgCode1[15:11],3'h0,txtFgCode1[10:5],2'h0,txtFgCode1[4:0],3'h0};
always @(posedge vclk)
	if (nhp)
		fgColor32d <= fgColor32;

always @(posedge vclk)
	if (ld_shft)
		bgt <= txtBkCode1==txtTcCode;
always @(posedge vclk)
	if (nhp)
		bgtd <= bgt;

// Convert character bitmap to pixels
// For convenience, the character bitmap data in the ROM is in the
// opposite bit order to what's needed for the display. The following
// just alters the order without adding any hardware.
//
wire [7:0] charRev = {
	char_bmp[0],
	char_bmp[1],
	char_bmp[2],
	char_bmp[3],
	char_bmp[4],
	char_bmp[5],
	char_bmp[6],
	char_bmp[7]
};

reg [7:0] charout1;
always @(posedge vclk)
	charout1 <= blink ? (charRev ^ curout) : charRev;

// Convert parallel to serial
ParallelToSerial ups1
(
	.rst(rst_i),
	.clk(vclk),
	.ce(nhp),
	.ld(ld_shft),
	.qin(1'b0),
	.d(charout1),
	.qh(pix)
);

reg pix1;
always @(posedge vclk)
	if (nhp)	
    pix1 <= pix;

// Pipelining Effect:
// - character output is delayed by 2 or 3 character times relative to the video counters
//   depending on the resolution selected
// - this means we must adapt the blanking signal by shifting the blanking window
//   two or three character times.
wire bpix = hctr[2] ^ scanline[4];// ^ blink;
always @(posedge vclk)
	if (nhp)	
		iblank <= (row >= numRows) || (col >= numCols + charOutDelay) || (col < charOutDelay);
	

// Choose between input RGB and controller generated RGB
// Select between foreground and background colours.
always @(posedge dot_clk_i)
	casez({controller_enable&xonoff_i,blank_i,iblank,border_i,bpix,pix1})
	6'b?1????:	zrgb_o <= 32'h00000000;
	6'b10010?:	zrgb_o <= 32'hFFBF2020;
	6'b10011?:	zrgb_o <= 32'hFFDFDFDF;
	6'b1000?0:	zrgb_o <= bgtd ? zrgb_i : bkColor32d;
	6'b1000?1:	zrgb_o <= fgColor32d;
	default:	zrgb_o <= zrgb_i;
	endcase

endmodule

