----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    23:22:25 07/11/2017 
-- Design Name: 
-- Module Name:    gbox_2 - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;               -- contains conversion functions
use work.all;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity sbox_2 is
  
  port (
    InpxDI_S3 : in  std_logic_vector(3 downto 0);
	 InpxDI_S1 : in  std_logic_vector(3 downto 0);
    OupxDO : out std_logic_vector(3 downto 0)
    );

end sbox_2;

architecture lookuptable of sbox_2 is

signal T0xD,T1xD,T2xD,T3xD,a1,b1,c1,d1,a3,b3,c3,d3 : std_logic;

begin

-- lookuptable

-- OupxDO <= std_logic_vector(to_unsigned(SBOX(to_integer(unsigned(InpxDI(3 downto 0)))), 4));

a3 <= InpxDI_S3(0);
b3 <= InpxDI_S3(1);
c3 <= InpxDI_S3(2);
d3 <= InpxDI_S3(3);

a1 <= InpxDI_S1(0);
b1 <= InpxDI_S1(1);
c1 <= InpxDI_S1(2);
d1 <= InpxDI_S1(3); --not needed

T3xD <= (a1 and b1) xor (a1 and b3) xor (a1 and c1) xor (a1 and c3) xor (a3 and b1) xor (a3 and c1) xor c1 xor d1;
T2xD <= (a1 and c1) xor (a1 and c3) xor (a3 and c1) xor (b1 and c1) xor (b1 and c3) xor b1 xor (b3 and c1) xor d1;
T1xD <= a1 xor (b1 and c1) xor (b1 and c3) xor (b3 and c1) xor d1;
T0xD <= (a1 and c1) xor (a1 and c3) xor a1 xor (a3 and c1) xor b1 xor d1;

OupxDO <= T3xD & T2xD & T1xD & T0xD;

end lookuptable;
