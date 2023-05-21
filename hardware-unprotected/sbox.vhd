
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use ieee.numeric_std.all;         
use work.all;

entity sbox is
  
  port (
       InpxDI  : in  std_logic_vector(3 downto 0);
       OupxD0  : out std_logic_vector(3 downto 0)
    );

end sbox;

architecture lookuptable of sbox is

signal x0, x1, x2, x3 : std_logic; 

signal y0,y1,y2,y3 : std_logic; 

begin

-- OupxD0 <= InpxDI_S0;

x0 <= InpxDI(0);
x1 <= InpxDI(1);
x2 <= InpxDI(2);
x3 <= InpxDI(3);

y0 <= ( x0 AND x2 ) XOR ( x0 ) XOR ( x1 ) XOR ( x3 ) XOR ( '1' ) ;
y1 <= ( x0 ) XOR ( x1 AND x2 ) XOR ( x3 ) XOR ( '1' ) ;
y2 <= ( x0 AND x2 ) XOR ( x1 AND x2 ) XOR ( x1 ) XOR ( x3 ) ;
y3 <= ( x0 AND x1 ) XOR ( x0 AND x2 ) XOR ( x2 ) XOR ( x3 ) ;

OupxD0 <= y3 & y2 & y1 & y0;


end lookuptable;






