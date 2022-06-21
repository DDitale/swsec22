# Key Server - Crypto

Descrizione della challenge

>We have received a portable asymmetric key storage for evaluation purposes.
> This portable device was manufactured by Ebian Corp to
facilitate secure communications with customers.
>
> It generates and stores adminstrators' public keys.
> Customers can use this repository to find the public key of the admin they want to contact, and administrators can use this repository to update their key information.
>
> If this fancy keychain passes the test we are going to
give them away like candy, secure candy.
>
---

All'avvio del programma, sul monitor seriale si presenta il seguente banner:

```
Ebian Corp has a repository to store its adminstrators' public keys.
1)If you are a customer you can list all the public keys.
2)If you are an admin you can update your keys.
Just sign the plaintext "admin" and more options will be provided.
The parameters to be used for the signature are SHA1 and PKCS1_v1_5
```

Da qui possiamo capire che abbiamo a che fare con chiavi RSA algoritmi di firma digitale.

Usando il comando `1`, possiamo vedere l'elenco delle chiavi salvate sulla piattaforma

```
> 1

Public Key List: 
Alice: 
00c90e4c65c0a96080df5aaf73caaf1146afb364cb37894f6bb745182191bf577a2777d8b5dbf57b2ad9f902e2
d3abadfe6ebeb7366d7b11f0cbfd4371dbadf5f548e60644b3365a654b8efe2de32bbb1cc0288d367c0e8cf9ac
8a2544cb067f677c87e82362e3b3ce950d072c5b1baffd650a31db4ff7d7209f0aec0178d7ba8b
Bob: 
00db87e4a4774c4c4606faadeb58460d6c62282aced115ae9d256d6ca2d32b49615c9257869aa0b1757b8faaae
401f94474ddbf5f54b75dfaa7bef370cc9842a920ff9484cabacece44e7c2c80c2c97775a39d035c59475db933
74cbac0d0e4f0830bcc51fe4680ef1d8afce89d61ef7a1fe8f03dd26a7049303f1cbfa94b10323
...
Gary: 
00bdb08ad1d97628b0d4e9bdcdb0303007e66b9d82b3ca3e7df476911f1d0ffd81f67487b4fafc4e252b30c501
055335ab74f1e92e411615b5263d5117daf715740f826a6f8faba2620ddda2852a3595aa9f051d3e0b46766440
360f986cc2db7b7f2d9431e9324280109ac1ed43900a57531ee2878e895c6f5b4ba4311051413d
```

Per ogni utente è salvato solo un valore, però sappiamo che le chiavi RSA pubbliche sono definite da una coppia (n,e).

Essendo `e` un valore solitamente di piccole dimensioni, possiamo dare per scontato che quello presente in lista sia `n` e che `e` assuma un valore standard (solitamente 2^16 + 1, tale da rendere più efficienti le operazioni di elevazione a potenza - algoritmo square & multiply)

Per quanto riguarda le chiavi, possiamo presupporre che tra queste vi siano alcune con almeno un fattore comune. Questo perché la generazione di numeri primi pseudorandomici abbastanza grandi è relativamente complessa per dispositivi embedded, inoltre potrebbe essere stato usato per più generazioni lo stesso seed.

Questo può esserci di aiuto, in quanto entrambe le chiavi, pubblica e privata, sono calcolate sulla base di 2 numeri primi abbastanza grandi generati casualmente: il prodotto di questa coppia coincide proprio con `n` e, una volta fissato `e`, possiamo ricavare anche `d` ovvero l'esponente della chiave privata, dalla sua definizione: `d*e = 1 mod(λ(n))` dove `λ(n)` è solitamente calcolato come `(p-1)*(q-1)`

Una volta noti quindi i due fattori che hanno generato le chiavi, possiamo ricavare una chiave privata valida e firmare con essa il plaintext `admin` così come scritto nel banner iniziale.

Procediamo quindi con il calcolare il massimo comune divisore tra tutte le chiavi

```python
...
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

q = None

for pub1, pub2 in combinations(keys.values(), 2):
    massimo_comune_divisore = gcd(pub1,pub2)
    if(massimo_comune_divisore != 1):
        utenti = [k for k,v in keys.items() if v == pub1 or v == pub2]
        print(f"GCD per {pub1} e {pub2} = {massimo_comune_divisore} appartenente a {utenti}")
        q = massimo_comune_divisore

...
```
Troviamo così che le chiavi di Bob e Gary hanno un fattore in comune!

![Chiavi fattore comune](./chaivi_comune.png)

Noto questo, possiamo proseguire con il calcolare l'altro fattore dividendo `n` per il primo e ricavare così tutte le informazioni necessarie per generare una chiave privata valida:

```python

n1 = pub1
p1 = n1//q
phi1 = (p1-1)*(q-1)
d1 = pow(e, -1, phi1)

key_1 = RSA.construct((n1,e,d1,p1,q), consistency_check=True)

message_to_sign = b'admin'
sign_1 = PKCS1_v1_5.new(key_1).sign(SHA1.new(message_to_sign))

s.write(b'2\r')
s.write(b''+binascii.hexlify(sign_1)+b'\r')

```

![Flag](./flag.png)