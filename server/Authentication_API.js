// list of users and their respective username, password and email
app.get("/users", async(req, res) => {
    try {
        const userstable = await pool.query(
            "SELECT * from users"
        );
        res.json(userstable.rows);
    } catch (err) {
        console.log(err.message);
    }
});

//returns whether the pair username+password is correct/incorrect
app.get("/users/:username/:password", async(req, res) => {
    try {

        var string_result = "";

        var getUsername = [req.params.username];
        var getPassword = [req.params.password];

        const checkUsername = "SELECT COUNT(*) from users WHERE username = '" + getUsername + "'";
        console.log(checkUsername);
        const usernameExists = await pool.query(checkUsername);
        var userCounter = usernameExists.rows[0].count;

        if (userCounter == 1) {
            const user_pw = "SELECT (password = crypt('" + getPassword + "', password)) AS pwd_match FROM users WHERE username = '" + getUsername + "'";
            console.log(user_pw);
            const userpw = await pool.query(user_pw);
            //res.json(userpw.rows);
            var pwd_match = userpw.rows[0].pwd_match;
            Boolean(pwd_match);
            if (pwd_match) {
                string_result = "Correct username and password!";

            } else if (!pwd_match) {
                string_result = "Incorrect password!";

            }
        } else {
            string_result = "Check the username."
        }
        res.send(string_result);
        console.log(string_result);
    } catch (err) {
        console.log(err.message);
    }
});

//inserts the username+password+email and returns if the process was sucesseful or not
app.get("/insertuser/:username/:password/:email", async(req, res) => {
    try {

        var string_result = "";

        var getUsername = [req.params.username];
        var getPassword = [req.params.password];
        var getEmail = [req.params.email];

        const insertUser = "INSERT INTO users(username, password, email) VALUES ('" + getUsername + "', '" + getPassword + "', '" + getEmail + "')";

        console.log(insertUser);
        await pool.query(insertUser);

        string_result = "Utilizador inserido na DB com sucesso";
    } catch (err) {
        if (err.code == 23505)
            string_result = "Utilizador já existe na DB";
    }

    res.send(string_result);
    console.log(string_result);
});