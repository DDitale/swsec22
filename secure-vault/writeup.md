# Secure Vault - WEB

> Clam saw all those cool celebrities posting everything they do on twitter, so he decided to give it a go himself. Turns out, that's a horrible idea. After recovering from his emotional trauma, he wrote a secure vault to store his deepest secrets. Legend has it that there's even a flag in there. Can you get it?

---

Accedendo alla homepage presente in descrizione, possiamo vedere una pagina con una form di login/registrazione

![home](./pics/01_home.png)

Dopo il signup, possiamo vedere un campo di testo con una stringa e un cookie settato con un JWT ([Json Web Token](https://jwt.io/)).

![saved_cookie](./pics/03_saved_cookie.png)

Possiamo ricavare il contenuto del token usando `https://jwt.io/#debugger-io`. Vediamo così che contiene l'ID dell'utente che ha fatto l'accesso

![decoded_cookie](./pics/04_decoded_cookie.png)

Ora, guardando al sorgente della challenge, possiamo notare che l'obiettivo è visitare l'endpoint `/vault` mentre siamo utenti `unrestricted`

```js
app.get("/vault", (req, res) => {
    if (!res.locals.user) {
        res.status(401).send("Log in first");
        return;
    }
    const user = users.get(res.locals.user.uid);
    res.type("text/plain").send(user.restricted ? user.vault : flag);
});
```

Tornando sul sito mentre siamo loggati, possiamo vedere che il contenuto di `user.vault` non è altro che il contenuto della textarea nella home:

![set_vault](./pics/05_set_vault.png)

![vault](./pics/06_vault.png)

Come visto quindi finora, dopo un login o una registrazione, il cookie viene settato con l'uid dell'utente e firmato con una chiave random.

Conoscendo già il formato della chiave (`0.[0-9]+`) si potrebbe pensare di usare un tool come [jwtcrack](https://github.com/brendan-rius/c-jwt-cracker) per creare un token ad hoc valido e saltare il check.
Questo tool usa un attacco a forza bruta per cercare di ricavare il "secret" usato per firmare il token


Questo può essere lanciato con:
`docker run -it --rm  jwtcrack [token] [charset] [maxlen] [algorithm]`

Quindi, nel nostro caso:
`docker run -it --rm  jwtcrack eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1aWQiOiIwLjM0Nzc5Mjc3MTY2MDg4MjIiLCJpYXQiOjE2NTE1NzcwNDh9.asR5mHIczZ-s1tZHCoCNoLKtPsPFu8S46adyRwOYa-U 0.123456789 20 sha256`

Dopo un giorno senza risposte, torniamo a porre attenzione al codice.

In `express.js`, ogni applicazione ha una funzione `app.use` usata come middleware e richiamata ad ogni richiesta:

```js
app.use((req, res, next) => {
    try {
        res.locals.user = jwt.verify(req.cookies.token, jwtKey, {
            algorithms: ["HS256"],
        });
    } catch (err) {
        if (req.cookies.token) {
            res.clearCookie("token");
        }
    }
    next();
});
```
Possiamo qui notare che l'unica condizione tale per la quale viene settata la variabile è quella di usare un Token valido come cookie.

Inoltre, la funzione di login (a differenza di quella di registrazione) non verifica mai che i campi della richiesta siano correttamente popolati:
```js
app.post("/login", (req, res) => {
    const user = users.get(users.lookup(req.body.username));
    if (user && user.password === req.body.password) {
        res.cookie(
            "token",
            jwt.sign({ uid: user.uid }, jwtKey, { algorithm: "HS256" })
        );
        res.redirect("/");
    } else {
        res.redirect("/?e=" + encodeURIComponent("Invalid username/password"));
    }
});
```

Possiamo usare queste informazioni a nostro vantaggio: facendo una richiesta di login con un body vuoto, dovrebbe essere generato un token valido.

Per testare questa ipotesi usiamo un tool chiamato [Burp](https://portswigger.net/burp/communitydownload): con questo proxy possiamo intercettare la richiesta di login e modificarne il contenuto.

![login_aaa](./pics/07_alogin.png)

![richiesta_login](./pics/07_login%20(copy).png)

Qui possiamo spostare la richiesta al tab "Repeater" dove è possibile modificare e reinviarla senza problemi

![repeater](./pics/09_repeater.png)

![login_no_creds](./pics/10_login_without_creds_crop.png)

Ora abbiamo un token valido ma che non è associato a nessun utente!

![decoded_jwt](./pics/11_decoded_jwt.png)

Possiamo vedere il perché abbia funzionato rileggendo le funzioni chiamate nel metodo di login:

```js
const user = users.get(users.lookup(req.body.username));

//~~~~

get(uid) {
    return this.users[uid] ?? {};
}
lookup(username) {
    return this.usernames[username];
}
```

In javascript, alcuni oggetti quando sono presenti in una espressione booleana, sono sempre valutati a `True` e altri a `False`. Questi oggetti vengnono chiamati rispettivamente [Truthy](https://developer.mozilla.org/en-US/docs/Glossary/Truthy) e [Falsy](https://developer.mozilla.org/en-US/docs/Glossary/Falsy).

Qui possiamo vedere due esempi di questo tipo di oggetti:

Nella funzione `lookup`, se passiamo un valore `undefined` verrà restituito sempre `undefined`.

La funzione `get(uid)` proverà a valutare `this.users[undefined]`, ma essendo `undefined` un Falsy, restituirà un oggetto vuoto.

Abbiamo poi il seguente controllo:
```js
if (user && user.password === req.body.password)
```
dove `user` è un oggetto vuoto, quindi un Truthy.

Lo stesso si applica per la verifica della password: se non inviamo una richiesta e dato che l'oggetto vuoto non avrà un attributo password, entrambi i valori saranno `undefined` quindi si avrà `undefined===undefined` che è vera come condizione.

Abbiamo così il nostro cookie corretto.

Ora anche per l'accesso all'endpoint `vault`, l'attributo `user.restricted` sarà `undefined` e valutato come False: invece del contenuto del vault verrà mostrata la flag.

Tutto ciò che bisogna fare è quindi settare il cookie con il valore ottenuto dalla richiesta di login "vuota" e possiamo leggere la flag correttamente.

![set_cookie](./pics/12_set_cookie.png)
![flag](./pics/13_flag.png)