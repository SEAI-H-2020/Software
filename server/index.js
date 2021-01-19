// Based on https://www.youtube.com/watch?v=ldYcgPKEZC8

const express = require("express");
const app = express();
const cors = require("cors");

const pool = require("./db");

const swaggerUI = require('swagger-ui-express');
const swaggerFile = require('./swagger_output.json');

app.use(cors());
app.use(express.json());

//Create Swagger GUI
app.use('/doc', swaggerUI.serve, swaggerUI.setup(swaggerFile))

//Add authentication APIs
require('./Authentication_API')(app, pool);

//Add authentication APIs
require('./DB_API')(app, pool);

//Add update APIs
require('./ota_update')(app, pool);

//Get User Settings
app.get("/usersettings/:box_id", async(req, res) => {
    /*
        Swagger Documentation:
        #swagger.tags = ['User Settings']
        #swagger.method = 'get'
        #swagger.description = 'Gets user settings by box id'
        #swagger.parameters['box_id'] = {description: "Unique box serial number", type: "integer"}
    */
    try {
        box_id = req.params.box_id
        console.log("GET request user settings of box: " + box_id);
        const user_settings = await pool.query(
            "SELECT* FROM configurations WHERE box = $1", [box_id]
        );
        res.json(user_settings.rows);
    } catch (err) {
        console.log(err.message);
    }
});

//Update User Settings 
app.put("/usersettings/:box_id", async(req, res) => {
    /*
        Swagger Documentation:
        #swagger.tags = ['User Settings']
        #swagger.method = 'put'
        #swagger.description = 'Updates user settings by box id'
        #swagger.parameters['box_id'] = {description: "Unique box serial number", type: "integer"}
        #swagger.parameters['userSettings'] = {
            in: 'body',
            description: 'Measurement from sensor box',
            required: true,
            type: 'object',
            schema: {$ref: "#/definitions/NewUserSetting"}
    }
    */
    try {
        const params = req.params;
        console.log("UPDATE request user settings of box: " + req.params.box_id);

        //Calculate sync period from sample period
        let sync_time;
        switch(req.body.sample_time){
            case 1:
                sync_time = 5;
                break;
            case 5:
                sync_time = 25;
                break;
            case 10:
            case 15:
            case 30:
            case 60:
                sync_time = 30;
                break;
            default:
                throw new Error(`Invalid sample_time: ${req.body.sample_time}`);
        }
        
        const settingsres = await pool.query(
            `UPDATE configurations SET sync_period = $1, sample_time = $2 WHERE box = $3`, 
             [
                 sync_time, req.body.sample_time, req.params.box_id
            ]
        );
        const usersres = await pool.query(
            `UPDATE users SET username = $1, email = $2 
            WHERE username = (SELECT username from configurations where box = $3)`, [req.body.username, req.body.email, req.params.box_id]
        );
        
        console.log(settingsres)
        console.log(usersres)
        res.json({
            settings: settingsres,
            users: usersres
        });
    } catch (err) {
        console.log(err.message);
        res.json(err.message);
    }
});

//Update User Settings 
app.post("/usersettings/:box_id", async(req, res) => {
    /*
        Swagger Documentation:
        #swagger.tags = ['User Settings']
        #swagger.method = 'post'
        #swagger.description = 'Insert new user settings by box id \n To update existing user use the put request'
        #swagger.parameters['box_id'] = {description: "Unique box serial number", type: "integer"}
        #swagger.parameters['userSettings'] = {
            in: 'body',
            description: 'Measurement from sensor box',
            required: true,
            type: 'object',
            schema: {$ref: "#/definitions/NewUserSetting"}
    }
    */
    try {
        const params = req.params;
        console.log("Insert request user settings of box: " + req.params.box_id);

        //Calculate sync period from sample period
        let sync_time;
        switch(req.body.sample_time){
            case 1:
                sync_time = 5;
                break;
            case 5:
                sync_time = 25;
                break;
            case 10:
            case 15:
            case 30:
            case 60:
                sync_time = 30;
                break;
            default:
                throw new Error(`Invalid sample_time: ${req.body.sample_time}`);
        }

        const queryres = await pool.query(
            `INSERT INTO configurations (box, sync_period, sample_time, username, email)
             VALUES ($1, $2, $3, $4, $5)`,
            [
                req.params.box_id, sync_time, req.body.sample_time, req.body.username, req.body.email
            ]
        );
        console.log(queryres)
        res.json(queryres);
    } catch (err) {
        console.log(err.message);
        res.json(err.message);
    }
});

const port = 5000;
app.listen(port, () => {
    console.log("Servidor iniciado no port " + port)
});