
class UserStore {
    constructor() {
        this.users = {};
        this.usernames = {};
    }

    insert(username, password) {
        const uid = Math.random().toString();
        this.users[uid] = {
            username,
            uid,
            password,
            vault: "put something here!",
            restricted: true,
        };
        this.usernames[username] = uid;
        return uid;
    }

    get(uid) {
        return this.users[uid] ?? {};
    }

    lookup(username) {
        return this.usernames[username];
    }

    remove(uid) {
        const user = this.get(uid);
        delete this.usernames[user.username];
        delete this.users[uid];
    }
}


const users = new UserStore();
const uid = users.insert("Ciccio", "Formaggio");
console.log("Primo insert:\nuid\nuser\nusernames");
console.log(uid);
console.log(users.users);
console.log(users.usernames);

var undf = undefined;
var res_lookup = users.lookup(undf);
var res_users = users.get(res_lookup);
console.log("Login\nundefined\nlookup\nget\npassword");
console.log(undf);
console.log(res_lookup);
console.log(res_users);
console.log(res_users.password);