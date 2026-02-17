# Magic Number - Rev

Let’s download the file attached to the challenge and make it executable with `chmod +x magic-number`.

When we try to run it, it asks us to enter a number: entering a random number simply returns `Wrong`.

![First screen](./primo_input.png)

However, if we enter clearly invalid values (negative numbers, decimals, or strings), we observe different behaviors:

![Second screen](./wrong_input.png)

Apparently, the program expects a positive number.

We continue the analysis to understand how it works using a disassembler; in our case, we will use Binary Ninja Cloud.

The disassembled code of the `main` function is shown below:  
![Disassembly](./ninja_listing.png)

To simplify reading the jumps, we switch to “Tree” mode:  
![Disassembly diagram](./ninja_tree.png)

We can see that the program reads a string from the keyboard; if the read is successful, it calls the function `sub_401156`, passing the read string and `0x539` as arguments (we are on x64, so the first two function arguments are stored in the `rdi` and `esi` registers respectively).

Finally, the function result is compared with `0x16f2456f447`, and if they are equal, the flag is printed with `printf("CCIT{%lu}", user_input)`.

From this, we understand that the input string is actually the flag.

Now let’s analyze the function `sub_401156`.

![Recursive function](./sub_ricorsiva.png)

From the call at address `0x401198`, we can see that this is a recursive function.

The function body compares the first argument with `0`. If they are different, it calls itself again with the first argument decreased by 1 and adds to the second argument the result of the recursive call with itself; otherwise, it returns `0`.

In practice, this function performs multiplication through repeated addition: it adds the value `0x539` as many times as the value entered by the user.

We can make `main` more readable by renaming symbols in Binary Ninja:

![How to rename symbols](./rinomina_funzione.png)

![Cleaned-up sum function](./somma_rinominata.png)

![Changing constant display](./main_display_as.png)

![Cleaned-up main function](./main_pulito.png)

Once this is done, we can easily understand that the program takes an integer as input, multiplies it by `0x539 = 1337`, and compares it with `0x16f2456f447 = 1576862676039`.

All we need to do is input the value `1576862676039 / 1337`, which is `1179403647`.

However, this leads to a crash:

![Stack overflow](./harakiri.png)

This happens because the function is called recursively 1,179,403,647 times. At each call, the old base pointer, new local variables, and return address are saved on the stack.

Since this is repeated such a large number of times, the stack grows beyond its allocated memory region, resulting in a `Segmentation fault`.

We can still submit the flag as it would be formatted by the program at the end and see that it is correctly recognized:

`CCIT{1179403647}`
