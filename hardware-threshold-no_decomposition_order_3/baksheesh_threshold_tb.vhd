LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

ENTITY baksheesh_threshold_tb IS
END baksheesh_threshold_tb;
 
ARCHITECTURE behavior OF baksheesh_threshold_tb IS 
 
    COMPONENT baksheesh_threshold
    PORT(
         reset_n : IN  std_logic;
         clk : IN  std_logic;
         plaintext : IN  std_logic_vector(127 downto 0);
         key : IN  std_logic_vector(127 downto 0);
         mask1 : IN  std_logic_vector(127 downto 0);
         mask2 : IN  std_logic_vector(127 downto 0);
         start : IN  std_logic;
         ciphertext : OUT  std_logic_vector(127 downto 0);
         ready : OUT  std_logic
        );
    END COMPONENT;
    
   signal reset_n : std_logic := '0';
   signal clk : std_logic := '0';
   signal plaintext : std_logic_vector(127 downto 0) := (others => '0');
   signal key : std_logic_vector(127 downto 0) := (others => '0');
   signal mask1 : std_logic_vector(127 downto 0) := (others => '0');
   signal mask2 : std_logic_vector(127 downto 0) := (others => '0');
   signal start : std_logic := '0';

   signal ciphertext : std_logic_vector(127 downto 0);
   signal ready : std_logic;

   constant clk_period : time := 100 ns;
 
BEGIN
 
   uut: baksheesh_threshold PORT MAP (
          reset_n => reset_n,
          clk => clk,
          plaintext => plaintext,
          key => key,
          mask1 => mask1,
          mask2 => mask2,
          start => start,
          ciphertext => ciphertext,
          ready => ready
        );

   clk_process :process
   begin
		clk <= '0';
		wait for clk_period/2;
		clk <= '1';
		wait for clk_period/2;
   end process;
 
   stim_proc: process
   begin		
      wait for 2*clk_period;
		reset_n <= '1';
		plaintext <= x"44444444444444444444444444444444";
		key <= x"00000000000000000000000000000000";
		-- ciphertext: cd 0b d7 38 38 8a d3 f6 68 b1 5a 36 ce b6 ff 92
		start <= '1';
		wait for clk_period;
		start <= '0';
        wait for 40*clk_period;
		plaintext <= x"e39c141fa57dba43f08a85b6a91f86c1";
		key <= x"d0f5c59a7700d3e799028fa9f90ad837";
		-- ciphertext: 13 ed e6 7c bd cc 3d bf 40 0a 62 d6 97 72 65 ea
		start <= '1';
		wait for clk_period;
		start <= '0';
		wait for 40*clk_period;
   end process;
END;
