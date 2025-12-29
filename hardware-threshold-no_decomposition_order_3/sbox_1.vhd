
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;
use work.all;

entity sbox_1 is
  
port (
    InpxDI_S3 : in  std_logic_vector(3 downto 0);
	InpxDI_S2 : in  std_logic_vector(3 downto 0);
    OupxDO : out std_logic_vector(3 downto 0)
);

end sbox_1;

architecture lookuptable of sbox_1 is

signal T0xD,T1xD,T2xD,T3xD,a2,b2,c2,d2,a3,b3,c3,d3 : std_logic;

begin

a3 <= InpxDI_S3(0);
b3 <= InpxDI_S3(1);
c3 <= InpxDI_S3(2);
d3 <= InpxDI_S3(3);

a2 <= InpxDI_S2(0);
b2 <= InpxDI_S2(1);
c2 <= InpxDI_S2(2);
d2 <= InpxDI_S2(3);


T3xD <= (a2 and b2) xor (a2 and b3) xor (a2 and c2) xor (a2 and c3) xor (a3 and b2) xor (a3 and b3) xor (a3 and c2) xor (a3 and c3) xor c2 xor c3 xor d3;
T2xD <= (a2 and c2) xor (a2 and c3) xor (a3 and c2) xor (a3 and c3) xor (b2 and c2) xor (b2 and c3) xor b2 xor (b3 and c2) xor (b3 and c3) xor b3 xor d3;
T1xD <= a2 xor a3 xor (b2 and c2) xor (b2 and c3) xor (b3 and c2) xor (b3 and c3) xor d3 xor '1';
T0xD <= (a2 and c2) xor (a2 and c3) xor a2 xor (a3 and c2) xor (a3 and c3) xor a3 xor b2 xor b3 xor d3 xor '1';

OupxDO <= T3xD & T2xD & T1xD & T0xD;

end lookuptable;
