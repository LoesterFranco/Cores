-------------------------------------------------------------------------------
--        __
--   \\__/ o\    (C) 2020  Robert Finch, Waterloo
--    \  __ /    All rights reserved.
--     \/_//     robfinch<remove>@finitron.ca
--       ||
--
-- File: xbusTransmitter.vhd
-- Modified from:
--
-- File: rgb2dvi.vhd
-- Author: Elod Gyorgy
-- Original Project: HDMI input on 7-series Xilinx FPGA
-- Date: 30 October 2014
--
-------------------------------------------------------------------------------
-- (c) 2014 Copyright Digilent Incorporated
-- All Rights Reserved
-- 
-- This program is free software; distributed under the terms of BSD 3-clause 
-- license ("Revised BSD License", "New BSD License", or "Modified BSD License")
--
-- Redistribution and use in source and binary forms, with or without modification,
-- are permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice, this
--    list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright notice,
--    this list of conditions and the following disclaimer in the documentation
--    and/or other materials provided with the distribution.
-- 3. Neither the name(s) of the above-listed copyright holder(s) nor the names
--    of its contributors may be used to endorse or promote products derived
--    from this software without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
-- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
-- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
-- FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
-- DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
-- SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
-- CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
-- OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
-- OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--
-------------------------------------------------------------------------------
--
-- Purpose:
-- This module connects to a top level DVI 1.0 source interface comprised of three
-- TMDS data channels and one TMDS clock channel. It includes the necessary
-- clock infrastructure (optional), encoding and serialization logic.
-- On the input side it has 24-bit RGB video data bus, pixel clock and synchronization
-- signals. 
--  
-------------------------------------------------------------------------------

library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity xbusTransmitter is
   Generic (
      kParallelWidth : natural := 14;
      kGenerateSerialClk : boolean := true;
      kClkPrimitive : string := "MMCM"; -- "MMCM" or "PLL" to instantiate, if kGenerateSerialClk true
      kClkRange : natural := 2;  -- MULT_F = kClkRange*5 (choose >=120MHz=1, >=60MHz=2, >=40MHz=3)      
      kRstActiveHigh : boolean := true); --true, if active-high; false, if active-low
   Port (
      -- DVI 1.0 TMDS video interface
      TMDS_Clk_p : out std_logic;
      TMDS_Clk_n : out std_logic;
      TMDS_Data_p : out std_logic_vector(2 downto 0);
      TMDS_Data_n : out std_logic_vector(2 downto 0);
      
      -- Auxiliary signals 
      aRst : in std_logic; --asynchronous reset; must be reset when RefClk is not within spec
      aRst_n : in std_logic; --asynchronous reset; must be reset when RefClk is not within spec
      
      -- Video in
      dat_i : in std_logic_vector(((kParallelWidth-2)*3)-1 downto 0);
      sync_i : in std_logic;
      de_i : in std_logic;
      PixelClk : in std_logic; --pixel-clock recovered from the DVI interface
      
      SerialClk : in std_logic); -- 5x PixelClk
   
end xbusTransmitter;

architecture Behavioral of xbusTransmitter is
type dataOut_t is array (2 downto 0) of std_logic_vector(kParallelWidth-3 downto 0);
type dataOutRaw_t is array (2 downto 0) of std_logic_vector(kParallelWidth-1 downto 0);
signal pDataOut : dataOut_t;
signal pDataOutRaw : dataOutRaw_t;
signal pVde, pC0, pC1 : std_logic_vector(2 downto 0);
signal aRst_int, aPixelClkLckd : std_logic;
signal PixelClkIO, SerialClkIO, aRstLck, pRstLck : std_logic;
begin

ResetActiveLow: if not kRstActiveHigh generate
   aRst_int <= not aRst_n;
end generate ResetActiveLow;

ResetActiveHigh: if kRstActiveHigh generate
   aRst_int <= aRst;
end generate ResetActiveHigh;

-- Generate SerialClk internally?
ClockGenInternal: if kGenerateSerialClk generate
   ClockGenX: entity work.xbusClockGen
      Generic map (
         kClkRange => kClkRange,  -- MULT_F = kClkRange*5 (choose >=120MHz=1, >=60MHz=2, >=40MHz=3, >=30MHz=4, >=25MHz=5
         kClkPrimitive => kClkPrimitive) -- "MMCM" or "PLL" to instantiate, if kGenerateSerialClk true
      Port map (
         PixelClkIn => PixelClk,
         PixelClkOut => PixelClkIO,
         SerialClk => SerialClkIO,
         aRst => aRst_int,
         aLocked => aPixelClkLckd);
   --TODO revise this
   aRstLck <= not aPixelClkLckd;         
end generate ClockGenInternal;

ClockGenExternal: if not kGenerateSerialClk generate
   PixelClkIO <= PixelClk;
   SerialClkIO <= SerialClk;
   aRstLck <= aRst_int;
end generate ClockGenExternal;

-- We need a reset bridge to use the asynchronous aLocked signal to reset our circuitry
-- and decrease the chance of metastability. The signal pLockLostRst can be used as
-- asynchronous reset for any flip-flop in the PixelClk domain, since it will be de-asserted
-- synchronously.
LockLostReset: entity work.ResetBridge
   generic map (
      kPolarity => '1')
   port map (
      aRst => aRstLck,
      OutClk => PixelClk,
      oRst => pRstLck);

-- Clock needs no encoding, send a pulse
ClockSerializer: entity work.xbusOutputSERDES
   generic map (
      kParallelWidth => kParallelWidth) -- TMDS uses 1:10 serialization
   port map(
      PixelClk => PixelClkIO,
      SerialClk => SerialClkIO,
      sDataOut_p => TMDS_Clk_p,
      sDataOut_n => TMDS_Clk_n,
      --Encoded parallel data (raw)
      pDataOut => "11111110000000",
      --pDataOut => "1111100000",
      aRst => pRstLck);

DataEncoders: for i in 0 to 2 generate
   DataEncoder: entity work.xbusEncoder
      port map (
         PixelClk => PixelClk,
         SerialClk => SerialClk,
         pDataOutRaw => pDataOutRaw(i),
         aRst => pRstLck,
         pDataOut => pDataOut(i),
         pC0 => pC0(i),
         pC1 => pC1(i),
         pVde => pVde(i)
      );
   DataSerializer: entity work.xbusOutputSERDES
      generic map (
         kParallelWidth => kParallelWidth) -- TMDS uses 1:10 serialization
      port map(
         PixelClk => PixelClkIO,
         SerialClk => SerialClkIO,
         sDataOut_p => TMDS_Data_p(i),
         sDataOut_n => TMDS_Data_n(i),
         --Encoded parallel data (raw)
         pDataOut => pDataOutRaw(i),
         aRst => pRstLck);      
end generate DataEncoders;

-- DVI Output conform DVI 1.0
-- except that it sends blank pixel during blanking
-- for some reason vid_data is packed in RBG order
pDataOut(0) <= dat_i((kParallelWidth-2)*1-1 downto 0); -- green is channel 1
pDataOut(1) <= dat_i((kParallelWidth-2)*2-1 downto kParallelWidth-2); -- blue is channel 0
pDataOut(2) <= dat_i((kParallelWidth-2)*3-1 downto (kParallelWidth-2)*2); -- red is channel 2
pC0(2 downto 1) <= (others => '0'); -- default is low for control signals
pC1(2 downto 1) <= (others => '0'); -- default is low for control signals
pC0(0) <= sync_i; -- channel 0 carries control signals too
pC1(0) <= '0'; -- channel 0 carries control signals too
pVde <= de_i & de_i & de_i; -- all of them are either active or blanking at once

end Behavioral;
