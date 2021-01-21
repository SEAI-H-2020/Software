module.exports = function(app, pool){
// list of users and their respective username, password and email
app.get("/users", async(req, res) => {
    /*
        Swagger Documentation:
        #swagger.tags = ['Authentication']
        #swagger.description = 'Lists all users and their data.'
    */   
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
    /*
        Swagger Documentation:
        #swagger.tags = ['Authentication']
        #swagger.description = 'Returns whether the pair username+password is correct/incorrect.'
        #swagger.parameters['username'] = {type: "string"}
        #swagger.parameters['password'] = {type: "string"}
    */   
    try {

        var string_result = "";

        var getUsername = [req.params.username];
        var getPassword = [req.params.password];


        const md5 = require('md5');
        var encryptedPassword = md5(String(getPassword));

        console.log('Normal password : ', String(getPassword));
        console.log('Hashed password : ', encryptedPassword);

        const checkUsername = "SELECT COUNT(*) from users WHERE username = '" + getUsername + "'";
        console.log(checkUsername);
        const usernameExists = await pool.query(checkUsername);
        var userCounter = usernameExists.rows[0].count;

        if (userCounter == 1) {
            const user_pw = "SELECT (password = '" + encryptedPassword + "') AS pwd_match FROM users WHERE username = '" + getUsername + "'";
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
        var aux = {
            "result": string_result
            };
        //res.json(string_result);
        res.send(aux);
        console.log(string_result);
    } catch (err) {
        console.log(err.message);
    }
});

//inserts the username+password+email and returns if the process was sucesseful or not
app.get("/insertuser/:username/:password/:email", async(req, res) => {
    /*
        Swagger Documentation:
        #swagger.tags = ['Authentication']
        #swagger.description = 'Inserts a new user and returns if the process was sucesseful or not'
        #swagger.parameters['username'] = {type: "string"}
        #swagger.parameters['password'] = {type: "string"}
        #swagger.parameters['email'] = {type: "string"}
    */   
    try {

        var string_result = "";

        var getUsername = [req.params.username];
        var getPassword = [req.params.password];
        var getEmail = [req.params.email];
        var validatedUser = 1;

        const md5 = require('md5');
        var encryptedPassword = md5(String(getPassword));

        console.log('Normal password : ', String(getPassword));
        console.log('Hashed password : ', encryptedPassword);


        const checkUsername = "SELECT COUNT(*) FROM users WHERE username = '" + getUsername + "'";
        const usernameExists = await pool.query(checkUsername);

        var userCounter = usernameExists.rows[0].count;

        if (userCounter > 0){
            string_result = "Username já existe na DB";
            validatedUser = 0;
        }

        const checkEmail = "SELECT COUNT(*) FROM users WHERE email = '" + getEmail + "'";
        const emailExists = await pool.query(checkEmail);

        var mailCounter = emailExists.rows[0].count;

        if (mailCounter > 0){
            validatedUser = 0;

            if (string_result != "")
                string_result = "Username e email já existem na BD";
            else
                string_result = "Email já existe na DB";
        }

        if (validatedUser == 1){
            const insertUser = "INSERT INTO users(username, password, email) VALUES ('" + getUsername + "', '" + encryptedPassword + "', '" + getEmail + "')";

            console.log(insertUser);
            await pool.query(insertUser);
    
            string_result = "Utilizador inserido na DB com sucesso";
        } 
        }catch (err) {
            if (err.code == 23505)
                string_result = "Utilizador já existe na DB";
        }

    var aux = {
        "result": string_result
        };
    //res.json(string_result);
    res.send(aux);    console.log(string_result);
});
}