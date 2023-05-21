library IEEE;
use IEEE.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use IEEE.numeric_std.all;
use work.all;

entity baksheesh_unprotected is port (
	reset_n : in std_logic;
    clk : in  std_logic;
    plaintext : in  std_logic_vector(127 downto 0);
    key : in  std_logic_vector(127 downto 0);
	start : in std_logic;
    ciphertext : out std_logic_vector(127 downto 0);
	ready : out std_logic
);
end baksheesh_unprotected;

architecture behav of baksheesh_unprotected is
 
signal round_counter : std_logic_vector(5 downto 0);

signal round_constant, round_constant_p, round_constant_n, round_constant_1: std_logic_vector(4 downto 0); -- round constants

signal KeyxD : std_logic_vector(127 downto 0);
  
-- counters for input and output

-- control signals
  signal LoadxS, LoadxS_del, UxD, round_toggle : std_logic;
  
-- Input to encryption core
  signal S1xD_S1      : std_logic_vector(127 downto 0);
  
  signal AoutxD_S1,PoutxD_S1,CoutxD_S1: std_logic_vector(127 downto 0);

  signal RegxDP_S1       : std_logic_vector(127 downto 0);
  
  signal KeyxDP       : std_logic_vector(127 downto 0);

  signal InxDI_S1  : std_logic_vector(127 downto 0); 

subtype Int6Type is integer range 0 to 127;
  type Int6Array is array (0 to 127) of Int6Type;
  constant Perm : Int6Array := (

  0, 33, 66, 99, 96,  1, 34, 67, 64, 97,  2, 35, 32, 65, 98,  3,
  4, 37, 70,103,100,  5, 38, 71, 68,101,  6, 39, 36, 69,102,  7,
  8, 41, 74,107,104,  9, 42, 75, 72,105, 10, 43, 40, 73,106, 11,
 12, 45, 78,111,108, 13, 46, 79, 76,109, 14, 47, 44, 77,110, 15,
 16, 49, 82,115,112, 17, 50, 83, 80,113, 18, 51, 48, 81,114, 19,
 20, 53, 86,119,116, 21, 54, 87, 84,117, 22, 55, 52, 85,118, 23,
 24, 57, 90,123,120, 25, 58, 91, 88,121, 26, 59, 56, 89,122, 27,
 28, 61, 94,127,124, 29, 62, 95, 92,125, 30, 63, 60, 93,126, 31

);

type state_type is (A, B, C);
signal state : state_type;

begin

InxDI_S1 <= plaintext;

LoadxS <= '1' when round_constant_p = "00000" else '0'; -- changed ##############

round_constant_1 <= "10000" when LoadxS = '1' else round_constant_n;

-------------------------------------------------------------------------------
-- Encryption core
-------------------------------------------------------------------------------

KeyxD <= KeyxDP(0) &  KeyxDP(127 downto 1);

i_slayer : entity slayer_1(s1) port map (RegxDP_S1,  S1xD_S1);

loop1: for i in 0 to 127 generate
  
PoutxD_S1(Perm(i)) <= S1xD_S1(i);
  
end generate loop1;

AoutxD_S1 <= PoutxD_S1 xor KeyxD;

round_constant(4) <= round_constant_p(4);
round_constant(3) <= round_constant_p(3);
round_constant(2) <= round_constant_p(2);
round_constant(1) <= round_constant_p(1);
round_constant(0) <= round_constant_p(0);

CoutxD_S1 (106) <= AoutxD_S1 (106) xor round_constant_p(0);
CoutxD_S1 (67)  <= AoutxD_S1 (67)  xor round_constant_p(1);
CoutxD_S1 (35)  <= AoutxD_S1 (35)  xor round_constant_p(2);
CoutxD_S1 (19)  <= AoutxD_S1 (19)  xor round_constant_p(3);
CoutxD_S1 (13)  <= AoutxD_S1 (13)  xor ( round_constant_p(4));
CoutxD_S1 (8)   <= AoutxD_S1 (8)   xor round_toggle;

CoutxD_S1 (127 downto 107) <= AoutxD_S1 (127 downto 107);
CoutxD_S1 (105 downto 68)  <= AoutxD_S1 (105 downto 68);
CoutxD_S1 (66 downto 36)   <= AoutxD_S1 (66 downto 36);
CoutxD_S1 (34 downto 20)   <= AoutxD_S1 (34 downto 20);
CoutxD_S1 (18 downto 14)   <= AoutxD_S1 (18 downto 14);
CoutxD_S1 (12  downto 9)   <= AoutxD_S1 (12  downto 9);
CoutxD_S1 (7  downto 0)    <= AoutxD_S1 (7  downto 0);

ciphertext <= CoutxD_S1;

UxD <= round_constant_p(4) xor round_constant_p(2);

round_constant_n <= round_constant_p(3 downto 0) & UxD;

sr01 : entity Sreg (sr) port map (CoutxD_S1, InxDI_S1, clk, LoadxS, RegxDP_S1); 
kr01 : entity Kreg (kr) port map (KeyxD, key, clk, LoadxS, KeyxDP); 

FSM : process(reset_n, clk)

begin
	if reset_n ='0' then
		state <= A;
		ready <= '0';
		round_toggle <= '0';
	elsif(rising_edge(clk)) then	
	   LoadxS_del <= LoadxS;
	   round_toggle <= not round_toggle;
		case state is
			when A =>
				round_counter <= (others=>'0');
				round_constant_p <= "00000";  
			
				ready <= '0';
				if start = '1' then
					state <= B;
				end if;
			when B =>
					round_constant_p <= round_constant_1;
					
					if round_counter = "100010" then -- "11111"
						state <= A;
						ready <= '1';
					else
						round_counter <= round_counter + 1;
					end if;
								
			when others => 
				state <= A;
		end case;		 
	end if;
end process FSM;

end architecture behav;

