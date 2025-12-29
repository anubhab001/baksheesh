----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    23:22:25 07/11/2017 
-- Design Name: 
-- Module Name:    gbox_3 - lookuptable 
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

entity sbox_3 is
  
  port (
    InpxDI_S2 : in  std_logic_vector(3 downto 0);
	 InpxDI_S1 : in  std_logic_vector(3 downto 0);
    OupxDO : out std_logic_vector(3 downto 0)
    );

end sbox_3;

architecture lookuptable of sbox_3 is

signal T0xD,T1xD,T2xD,T3xD,a1,b1,c1,d1,a2,b2,c2,d2 : std_logic;


begin

-- lookuptable

  --OupxDO <= std_logic_vector(to_unsigned(SBOX(to_integer(unsigned(InpxDI(3 downto 0)))), 4));

a2 <= InpxDI_S2(0);
b2 <= InpxDI_S2(1);
c2 <= InpxDI_S2(2);
d2 <= InpxDI_S2(3); --not needed

a1 <= InpxDI_S1(0);
b1 <= InpxDI_S1(1);
c1 <= InpxDI_S1(2);
d1 <= InpxDI_S1(3);

T3xD <= (a1 and b2) xor (a1 and c2) xor (a2 and b1) xor (a2 and c1) xor d2;

T2xD <= (a1 and c2) xor (a2 and c1) xor (b1 and c2) xor (b2 and c1) xor d2;

T1xD <= (b1 and c2) xor (b2 and c1) xor d2;

T0xD <= (a1 and c2) xor (a2 and c1) xor d2;

--OupxDO <= T0xD & T1xD & T2xD & T3xD;
OupxDO <= T3xD & T2xD & T1xD & T0xD;


end lookuptable;
