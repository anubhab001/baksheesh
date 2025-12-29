library ieee;
use ieee.std_logic_1164.all;
 
use ieee.numeric_std.all;
use std.textio.all;
use work.all;

entity slayer_1 is
port (   ClkxCI : in std_logic;
         InpxDI_S3 : in  std_logic_vector(127 downto 0);
         InpxDI_S2 : in  std_logic_vector(127 downto 0);
         InpxDI_S1 : in  std_logic_vector(127 downto 0);
         OupxDO_S3 : out std_logic_vector(127 downto 0);
         OupxDO_S2 : out std_logic_vector(127 downto 0);
         OupxDO_S1 : out std_logic_vector(127 downto 0));
		 
end entity slayer_1;

architecture s1 of slayer_1 is 

begin 

loop1: for i in 0 to 31 generate 

------------- Shares minimed using ANF ---------------

sbox_1_i: entity sbox_1(lookuptable) port map (InpxDI_S3 (4*i+3 downto 4*i), InpxDI_S2 (4*i+3 downto 4*i), OupxDO_S1 (4*i+3 downto 4*i)); 
sbox_2_i: entity sbox_2(lookuptable) port map (InpxDI_S3 (4*i+3 downto 4*i), InpxDI_S1 (4*i+3 downto 4*i), OupxDO_S2 (4*i+3 downto 4*i)); 
sbox_3_i: entity sbox_3(lookuptable) port map (InpxDI_S2 (4*i+3 downto 4*i), InpxDI_S1 (4*i+3 downto 4*i), OupxDO_S3 (4*i+3 downto 4*i)); 

end generate loop1;


end architecture s1;
