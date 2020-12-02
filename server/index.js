// Based on https://www.youtube.com/watch?v=ldYcgPKEZC8

const express = require("express");
const app = express();
const cors = require("cors");
const json2csv = require("json2csv")

const pool = require("./db");

app.use(cors());
app.use(express.json());

//File Name for csv export of measurements 
const fileName = "export.csv";

//Add authentication APIs
require('./Authentication_API')(app, pool);

// Upload sensor measurements
app.post("/measurements", async (req, res) => {
    try {
        console.log("Hey");
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
    try{
        console.log("GET request user settings of box: " + req.params.box_id);
        const user_settings = await pool.query(
            "SELECT* FROM configurations WHERE box = $1",
            [req.params.box_id]
        );
        res.json(user_settings.rows);
    } catch (err) {
            console.log(err.message);
    }
});

//Update User Settings 
app.put("/usersettings/:box_id", async(req,res) =>{
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
})

const port = 5000;
app.listen(port, () => {
    console.log("Servidor iniciado no port " + port)
});