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
        const queryres = await pool.query(
            `UPDATE configurations SET sync_period = $1, sample_time = $2,
             shutdown_on_wakeup = $3, username = $4, latest_firmware = $5 , email = $6 WHERE box = $7`, [req.body.sync_period, req.body.sample_time,
                req.body.shutdown_on_wakeup, req.body.username, req.body.latest_firmware, req.body.email, req.params.box_id
            ]
        );
        console.log(queryres)
        res.json("Updated!");
    } catch (err) {
        console.log(err.message);
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
        const queryres = await pool.query(
            `INSERT INTO configurations VALUES ($1, $2, $3, $4, $5, $6, $7)`, [req.params.box_id, req.body.sync_period, req.body.sample_time,
                req.body.shutdown_on_wakeup, req.body.username , req.body.email, req.body.latest_firmware
            ]
        );
        console.log(queryres)
        res.json("Inserted (not checking yet please confirm with db!)");
    } catch (err) {
        console.log(err.message);
    }
});

const port = 5000;
app.listen(port, () => {
    console.log("Servidor iniciado no port " + port)
});