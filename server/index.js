// Based on https://www.youtube.com/watch?v=ldYcgPKEZC8

const express = require("express");
const app = express();
const cors = require("cors");
const json2csv = require("json2csv");

const pool = require("./db");

const swaggerUI = require('swagger-ui-express');
const swaggerFile = require('./swagger_output.json');

app.use(cors());
app.use(express.json());

//File Name for csv export of measurements 
const fileName = "export.csv";

//Create Swagger GUI
app.use('/doc', swaggerUI.serve, swaggerUI.setup(swaggerFile))

//Add authentication APIs
require('./Authentication_API')(app, pool);

// Upload sensor measurements
app.post("/measurements", async (req, res) => {
    /*
        Swagger Documentation:
        #swagger.method = 'post'
        #swagger.tags = ['Measurements']
        #swagger.description = 'Endpoint to upload sensor data.'
        #swagger.parameters['sensorData'] = {
            in: 'body',
            description: 'Measurement from sensor box',
            required: true,
            type: 'object',
            schema: {$ref: "#/definitions/NewMeasurement"}
        }
    */   
    try {
        const sensorData = req.body;
        const newMeasurement = await pool.query(
            "INSERT INTO measurements (temperature, humidity, wind, noise_level, voltage) \
            VALUES ($1, $2, $3, $4, $5)",
            [req.body.temperature, req.body.humidity, req.body.wind, 
            req.body.noise_level, req.body.voltage]
        );
        res.json(newMeasurement);
    } catch (err) {
        console.log(err.message);
    }
    console.log(req.body);
});

// Get most recent measurement
app.get("/measurements", async (req, res) => {
    /*
        Swagger Documentation:
        #swagger.method = 'get'
        #swagger.tags = ['Measurements']
        #swagger.description = 'Gets most recent measurement'
    */   

    try {
        const newMeasurement = await pool.query(
            "SELECT * from measurements ORDER BY measurements.tstamp DESC LIMIT 1"
        );
        res.json(newMeasurement.rows);
    } catch (err) {
        console.log(err.message);
    }
});

// Donwload csv 
app.get("/csv", async (req, res) => {
    /*
        Swagger Documentation:  
        #swagger.tags = ['Export']
        #swagger.description = 'Exports all sensor measurements in csv format'
    */   
    try {
        const queryres = await pool.query(
            "SELECT * from measurements"
        );
        // ToDo: Add checks for csv integrity
        const csv = json2csv.parse(queryres.rows);
        res.header('Content-Type', 'text/csv');
        res.attachment(fileName);
        res.send(csv);
    } catch (err) {
        console.log(err.message);
    }
});

//  Clear database 
app.delete("/measurements", async (req, res) => {
    /*
        Swagger Documentation:
        #swagger.method = 'delete'
        #swagger.tags = ['Measurements']
        #swagger.description = 'Deletes all measurements!! \n WARNING: Use with caution'
    */   

    try {
        const queryres = await pool.query(
            "DELETE from measurements"
        );
        res.json(queryres)
    } catch (err) {
        console.log(err.message);
    }
});

//Get User Settings
app.get("/usersettings/:box_id", async(req,res) =>{
        /*
            Swagger Documentation:
            #swagger.tags = ['User Settings']
            #swagger.method = 'get'
            #swagger.description = 'Gets user settings by box id'
            #swagger.parameters['box_id'] = {description: "Unique box serial number", type: "integer"}
        */   
       try{
        box_id = req.params.box_id
        console.log("GET request user settings of box: " + box_id);
        const user_settings = await pool.query(
            "SELECT* FROM configurations WHERE box = $1",
            [box_id]
        );
        res.json(user_settings.rows);
    } catch (err) {
            console.log(err.message);
    }
});

//Update User Settings 
app.put("/usersettings/:box_id", async(req,res) =>{
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
    try{
        const params = req.params;
        console.log("UPDATE request user settings of box: " + req.params.box_id);
        const queryres = await pool.query(
            `UPDATE configurations SET sync_period = $1, sample_time = $2,
             shutdown_on_wakeup = $3, username = $4 WHERE box = $5`,
            [req.body.sync_period, req.body.sample_time, 
                req.body.shutdown_on_wakeup, req.body.username, req.params.box_id]
        );
        console.log(queryres)
        res.json("Updated!");
    } catch (err) {
            console.log(err.message);
    }
});

//Update User Settings 
app.post("/usersettings/:box_id", async(req,res) =>{
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
try{
    const params = req.params;
    console.log("Insert request user settings of box: " + req.params.box_id);
    const queryres = await pool.query(
        `INSERT INTO configurations VALUES ($1, $2, $3, $4, $5)`,
        [req.params.box_id, req.body.sync_period, req.body.sample_time, 
            req.body.shutdown_on_wakeup, req.body.username]
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