// Based on https://www.youtube.com/watch?v=ldYcgPKEZC8

const express = require("express");
const app = express();
const cors = require("cors");
const json2csv = require("json2csv")

const pool = require("./db");
const { json } = require("express");

app.use(cors());
app.use(express.json());

//File Name for csv export of measurements 
const fileName = "export.csv"

// Upload sensor measurements
app.post("/measurements", async (req, res) => {
    try {
        console.log("Hey");
        const newMeasurement = await pool.query(
            "INSERT INTO measuredownload csv from rest api nodements (temperature, humidity, wind, noise_level, voltage) \
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
    console.log(req.body);
});

// Donwload csv 
app.get("/csv", async (req, res) => {
    try {
        const queryres = await pool.query(
            "SELECT * from measurements"
        );
        // Add checks for csv integrity
        const csv = json2csv.parse(queryres.rows);
        res.header('Content-Type', 'text/csv');
        res.attachment(fileName);
        res.send(csv);
    } catch (err) {
        console.log(err.message);
    }
    console.log(req.body);
});

const port = 5000;
app.listen(port, () => {
    console.log("Servidor iniciado no port " + port)
});