import serial
from pprint import pprint
from itertools import combinations
from math import gcd
#Import per la firma crittografica
from Crypto.PublicKey import RSA
from Crypto.Hash import SHA1
from Crypto.Signature import pkcs1_15
from Crypto.Signature import PKCS1_v1_5

s = serial.Serial('/dev/ttyACM0',baudrate=19200, timeout=2)
s.readlines() # Lettura del banner
s.write(b'1\r') 

# Dizionario per il salvataggio delle chiavi lette
keys = {}

s.readline()

a = b''
public_keys = []
while(b'Ebian Corp ' not in a):
    a = s.readline()
    if(b'Ebian Corp ' not in a):
        public_keys.append(a)

lines = s.readlines()
pprint(lines)

cur_name = None
for line in public_keys:
    if b": " in line:
        name = line.decode('utf-8').split(":")[0]
        cur_name = name
        keys[name] = []
    else:
        keys[cur_name].append(line)


for user in keys:
    keys[user] = int(''.join([ k.decode('utf-8').rstrip() for k in keys[user]]),16)

#pprint(keys)

n1 = None
#n2 = None
p1 = None
#p2 = None
q = None

for pub1, pub2 in combinations(keys.values(), 2):
    massimo_comune_divisore = gcd(pub1,pub2)
    if(massimo_comune_divisore != 1):
        utenti = [k for k,v in keys.items() if v == pub1 or v == pub2]
        print(f"GCD per {pub1} e {pub2} = {massimo_comune_divisore} appartenente a {utenti}")
        q = massimo_comune_divisore
        n1 = pub1
        p1 = n1//q
        #n2 = pub2
        #p2 = n2//q


e = 65537 #Valore di default per RSA
# Chiave privata è data n e d
# n = p*q dove p è il gcd che abbiamo trovato
# q possiamo ricavarlo proprio prendendo una delle due chiavi pubbliche (n_pub1, n_pub2) e dividendole per p
# d è l'inverso moltiplicativo di e modulo (p-1)(q-1) y = pow(x, -1, p)

phi1 = (p1-1)*(q-1)
#phi2 = (p2-1)*(q-1)

d1 = pow(e, -1, phi1)
#d2 = pow(e, -1, phi2)

key_1 = RSA.construct((n1,e,d1,p1,q), consistency_check=True)
#key_2 = RSA.construct((n2,e,d2,p2,q), consistency_check=True)

message_to_sign = b'admin'

sign_1 = PKCS1_v1_5.new(key_1).sign(SHA1.new(message_to_sign))
#sign_2 = pkcs1_15.new(key_2).sign(SHA1.new(message_to_sign))

s.close()
import binascii

s = serial.Serial('/dev/ttyACM0',baudrate=19200, timeout=2)
banner = s.readlines() #(b'management\r\n')
print(banner)
s.write(b'2\r') # Memory management

print("> Invio 2")
lines = s.readlines()
print(lines)

print("> invio sign_1")
print(binascii.hexlify(sign_1))
s.write(b''+binascii.hexlify(sign_1)+b'\r')


lines = s.readlines()
print(lines)

s.close()

