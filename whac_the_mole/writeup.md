# Whac the Mole - Hardware

Challenge description:

> Who doesn't like a classic game of whac-the-mole?
>
> This time the moles infiltrated deep into the backyard of a poor farmer's family.  
> The moles are ruining the crops, which the farmer desperately needs to provide for his wife and two children.
>
> Any traveler able to help him by exterminating those darn things will be greatly rewarded.
>
> Are you up for the task?

Hardware challenge, implemented both in simulation and physically.

---

For this challenge, we use `PICSimLab`, a simulator based on `avrsim`, for the initial analysis phase.

When the program starts, a message appears on the serial interface:

![Startup message](./step_1.png)

After pressing Enter, nothing appears on the serial monitor: the only reaction from the board is the blinking of the onboard LED connected to pin 13.

After what seems like a timeout, the following message is displayed:

![Fail message](./step_2.png)

We can use the Oscilloscope tool provided by the simulator to observe what actually happens on pin 13:

![Oscilloscope capture on pin 13](./oscilloscopio_13.png)

We can see that the LED blinks three times with a 100 ms period and a 50% duty cycle.

Running the program multiple times shows that the number of pulses changes \[1–6\], but the time before the error message always remains around 5–6 seconds.

Given the program’s context, we can hypothesize that this represents the number of the mole currently moving and to be hit.

Using the Pushbutton objects in PICSimLab, we can connect them to the board and attempt to interact with it.

![Pushbuttons](./matrice_tasti.png)

Pressing random buttons reveals that for some of them, the error message appears before the 5-second timeout; moreover, with certain `pulse count – button` combinations, a message is displayed indicating that we successfully hit the mole.

![OK message](./messaggio_ok.png)

The goal, therefore, is to determine all six `mole – pin` pairs and complete the required 50 hits to win the game and obtain the flag.

Before doing so, we replicated the measurements using a real Arduino in order to accurately measure the timings needed to count the pulses. This was done using `Logic 2` and an initial example sketch to verify correct pin interaction: `WTM_Player.ino`.

![Pulse timing on pin 13](./logic_13.png)

![Serial message timing](./timing_tasto_inesistente.png)

This led us to notice a couple of things:

- At each restart, the mole pins are reassigned  
- If the code runs for too long, it always times out around the 5th–7th mole

After solving the pin assignment issue and correctly defining the timing intervals, we proceeded to write the code necessary for another Arduino to solve the challenge: `Sketch_WTM.ino`.

![Physical Arduino setup](./setup_reale.jpg)
![Initial guessing phase](./fase_guessing.png)
![Flag](./flag.png)
