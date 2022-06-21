----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    17:00:03 03/05/2021 
-- Design Name: 
-- Module Name:    divisore_di_frequenza - Behavioral 
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

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity divisore_di_frequenza is
		port(   clock: in STD_LOGIC;
				second : out STD_LOGIC
			 );
end divisore_di_frequenza;

architecture Behavioral of divisore_di_frequenza is
	signal clk_1 : STD_LOGIC;
begin

--divisiore di frequenza

div: process (clock)
	variable count : integer := 0;
	begin
		if (clock = '0' and clock ' event ) then
		    if (count <= 19999995) then
		    clk_1 <= '1';
			count := count + 1;
		    else if (count > 19999995 and count < 20000000) then --50000000
				clk_1 <= '0';
				count := count + 1;
			else 
				clk_1 <= '0';
				count := 0;
			end if;
		end if;
	end if;
end process ;

second <= clk_1;

end Behavioral;

