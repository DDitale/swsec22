# Secure Vault - WEB

> Clam saw all those cool celebrities posting everything they do on Twitter, so he decided to give it a try himself. It turned out to be a terrible idea. After recovering from his emotional trauma, he wrote a secure vault to store his deepest secrets. Legend has it that there’s even a flag in there. Can you get it?

---

By accessing the homepage mentioned in the description, we can see a page with a login/registration form.

After signing up, we can see a text field containing a string and a cookie set with a JWT (JSON Web Token).

We can retrieve the contents of the token using `https://jwt.io/#debugger-io`. In this way, we see that it contains the ID of the user who logged in.

Now, by looking at the source code of the challenge, we can notice that the goal is to visit the `/vault` endpoint while being an `unrestricted` user.


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
Returning to the site while logged in, we can see that the contents of `user.vault` are nothing more than the contents of the textarea on the homepage:

![set_vault](./pics/05_set_vault.png)

![vault](./pics/06_vault.png)

As observed so far, after a login or registration, the cookie is set with the user's UID and signed with a random key.

Knowing the key format (`0.[0-9]+`), one might think of using a tool such as [jwtcrack](https://github.com/brendan-rius/c-jwt-cracker) to create a valid ad hoc token and bypass the check.  
This tool performs a brute-force attack to try to recover the secret used to sign the token.

It can be executed with:

`docker run -it --rm jwtcrack [token] [charset] [maxlen] [algorithm]`

So, in our case:

`docker run -it --rm jwtcrack eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1aWQiOiIwLjM0Nzc5Mjc3MTY2MDg4MjIiLCJpYXQiOjE2NTE1NzcwNDh9.asR5mHIczZ-s1tZHCoCNoLKtPsPFu8S46adyRwOYa-U 0.123456789 20 sha256`

After a day without any results, we turn our attention back to the code.

In `express.js`, every application has an `app.use` function used as middleware and invoked on every request:


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
Here we can observe that the only condition under which the variable is set is the use of a valid token as a cookie.

Furthermore, the login function (unlike the registration function) never verifies that the request fields are correctly populated.

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

We can use this information to our advantage: by sending a login request with an empty body, a valid token should be generated.

To test this hypothesis, we use a tool called [Burp](https://portswigger.net/burp/communitydownload). With this proxy, we can intercept the login request and modify its contents.

![login_aaa](./pics/07_alogin.png)

![login_request](./pics/07_login%20(copy).png)

Here we can move the request to the **Repeater** tab, where it can be edited and resent easily.

![repeater](./pics/09_repeater.png)

![login_no_creds](./pics/10_login_without_creds_crop.png)

We now have a valid token that is not associated with any user!

![decoded_jwt](./pics/11_decoded_jwt.png)

We can understand why this worked by re-examining the functions called within the login method:


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

In JavaScript, some objects, when used in a boolean expression, are always evaluated as `true`, while others are evaluated as `false`. These are known respectively as [Truthy](https://developer.mozilla.org/en-US/docs/Glossary/Truthy) and [Falsy](https://developer.mozilla.org/en-US/docs/Glossary/Falsy).

Here we can see two examples of such objects:

In the `lookup` function, if we pass an `undefined` value, it will always return `undefined`.

The `get(uid)` function will attempt to evaluate `this.users[undefined]`, but since `undefined` is Falsy, it will return an empty object.

We then have the following check:
```js
if (user && user.password === req.body.password)
```
where `user` is an empty object, therefore a Truthy value.

The same applies to password verification: if we do not send any value in the request, and since the empty object does not have a `password` attribute, both values will be `undefined`. As a result, the condition `undefined === undefined` evaluates to `true`.

This way, we obtain a valid cookie.

Now, when accessing the `/vault` endpoint, the `user.restricted` attribute will also be `undefined`, which is evaluated as `false`. Consequently, instead of the vault contents, the flag is displayed.

All that remains is to set the cookie with the value obtained from the "empty" login request, and we can correctly read the flag.

![set_cookie](./pics/12_set_cookie.png)
![flag](./pics/13_flag.png)
