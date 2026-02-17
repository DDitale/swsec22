# Fiesta - Fault Injection

Challenge Description

>Elias Öberson @DrAndroid1337 - Nov 1
>
>Walking around the financial district, >I stumbled upon this strange
>device. Does anybody know what it is?
>
>Elias Öberson @DrAndroid1337 - Nov 1
>
>It does not have any recognisable logos >or marks, only the letters
>"V1" written on one side and a USB >connector on the other. Here is a
>photo.
>
>Elias Öberson @DrAndroid1337 - Nov 1
>
>It probably is some kind of memory, but >it has an unusual design. I
>will check it when I arrive home.
>
>Elias Öberson @DrAndroid1337 - Nov 1
>
>I finally arrived home. I connected the >USB device to my computer, but
>it looks that it is permanently locked.
>
>Elias Öberson @DrAndroid1337 - Nov 1
>
>Now I am curious about the device and >its content. Would I be able to
>unlock it using FI?

This challenge demonstrated the technical limitations of our tools. We attempted to implement an attack, but despite also using a Zybo Z7-10, we were unable to retrieve the flag, even though the challenge explicitly required performing a Fault Injection.

Below are the various procedures that were carried out.

We loaded the challenge binary and observed that it printed the challenge banner on the serial interface and then continuously displayed the string **"LOCK"** in a loop. <br>

![Lock](./pics/lock_lock.png)

Our first thought was that a block of code was being executed where a condition was always true, causing **LOCK** to be printed repeatedly. Therefore, we needed a way to make the program skip the `while` check. Doing so might have caused the flag to be printed on the serial interface.

We then considered interrupting the Arduino’s normal execution by briefly cutting its power supply during runtime. <br>

Question: How could we achieve this, and more importantly, which tool should we use?

Initially, we considered using another Arduino connected to the power supply line of the Arduino running the binary. We wrote a small script that momentarily removed power for an extremely short time interval. Below is a snippet that allowed us to perform this action within a chosen time window.

```c
  if (uiInterval != 0) {
    digitalWrite(pinAttack, LOW);
    delayMicroseconds(uiInterval);
    digitalWrite(pinAttack, HIGH);
    delay(1);
    for(int i = 0; i < 100; i++){
		digitalWrite(pinAttack, LOW);
		delayMicroseconds(1);
		digitalWrite(pinAttack, HIGH);e
		delayMicroseconds(1);
    }
    uiInterval = 0;
    tPrevious = millis();
  }
```

![setup_arduino](./pics/2_arduino.jpg)

Problem… it does not work. <br>
We tried multiple times to execute the attack, but unfortunately the best result we achieved was simply forcing the device to reboot.

After several attempts, we analyzed the signal using a logic analyzer. We observed that the high → low and low → high transitions occurred on the order of microseconds, which is technically too long for the Arduino to maintain sufficient voltage to remain powered on.

We therefore looked for a different approach.

Since one of us works with embedded systems, we decided to use the Zybo Z7-10 — an FPGA development board provided for the exam and equipped with a powerful FPGA.

Unfortunately, another issue arose. The Zybo’s output pins operate at **3.3V**, while the Arduino power supply requires **5V**. The only viable solution was therefore to shift the output voltage from 3.3V to 5V.

We built the following circuit to perform this voltage conversion:

![Level shifter](./pics/level_shift.gif)

Through several tests, we verified that the circuit worked. We were able to shift the voltage from 3.3V to 5V by leveraging the Arduino as a voltage reference.

![Level shifter realizzato](./pics/logic_level_reale.jpg)

Great — now we needed to replicate the behavior previously implemented with the Arduino script, but this time using VHDL. To do so, we reused an old frequency divider designed for the ASDI exam.


```vhdl
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
		    else if (count > 19999995 and count < 20000000) then
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

```

By using a `Zybo-z7-10-Master.xdc` file, we were able to map the `second` signal to output pin 1 of the **JC** pin header on the board.

After completing the implementation, we connected everything together:

![setup](./pics/setup.jpg)

Result? It did not work. Unfortunately, many factors must be taken into account. We believe we were unable to reach a sufficiently small time interval to successfully bypass the binary.

The limitation is likely not caused by the Zybo itself, but rather by the simplicity of the components used to build the circuit. These components may not have reacted quickly enough to the FPGA-generated pulses.

Another possible explanation is the significant presence of capacitors on the board, which likely stabilize the voltage too effectively, preventing this type of attack from succeeding with our setup.

Due to these constraints, we ultimately stopped the experimentation and were unable to retrieve the flag.

There are, however, specialized tools designed for these types of attacks, such as the **ChipWhisperer**, which we were unable to test due to lack of access to the hardware.
