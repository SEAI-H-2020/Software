module.exports = function(app, pool){
//Celsius -> Fahrenheit 
function CelsiustoFahrenheit(cTemperature) {
    var fTemperature = (cTemperature * 1.8) + 32;
    fTemperature = Math.round(fTemperature * 1e2) / 1e2;
    console.log(cTemperature + " Celsius -> " + fTemperature + " Fahrenheit");
    return fTemperature;
}

//Meters per second -> Miles per hour        
function MPStoMPH(mpsWind) {
    var mphWind = mpsWind * 2.237;
    mphWind = Math.round(mphWind * 1e2) / 1e2;
    console.log(mpsWind + " Meters per second -> " + mphWind + " Miles per hour");
    return mphWind;
}

// converts latest measurement from metric to imperial
app.get("/measurements/imperial", async(req, res) => {
    try {
        const newMeasurement = await pool.query(
            "SELECT * from measurements ORDER BY measurements.tstamp DESC LIMIT 1 "
        );

        var cTemperature = newMeasurement.rows[0].temperature;
        var mpsWind = newMeasurement.rows[0].wind;

        //Celsius -> Fahrenheit 
        var fTemperature = CelsiustoFahrenheit(cTemperature);

        //Meters per second -> Miles per hour
        var mphWind = MPStoMPH(mpsWind);

        newMeasurement.rows[0].temperature = fTemperature;
        newMeasurement.rows[0].wind = mphWind;
        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

//Measurements between 'start' and 'finish'
app.get("/measurements/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const newMeasurement = await pool.query(
            "SELECT * from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "';"
        );

        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

//Measurements between 'start' and 'finish' in imperial
app.get("/measurements/imperial/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const newMeasurement = await pool.query(
            "SELECT * from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "';"
        );
        var i = 0;
        var rows = newMeasurement.rowCount;
        var cTemperature = 0;
        var mpsWind = 0;
        var fTemperature = 0;
        var mphWind = 0;

        console.log(rows);
        for (i; i < rows; i++) {
            cTemperature = newMeasurement.rows[i].temperature;
            mpsWind = newMeasurement.rows[i].wind;

            //Celsius -> Fahrenheit 
            fTemperature = CelsiustoFahrenheit(cTemperature);

            //Meters per second -> Miles per hour
            mphWind = MPStoMPH(mpsWind);

            newMeasurement.rows[i].temperature = fTemperature;
            newMeasurement.rows[i].wind = mphWind;
        }

        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

//Sensor X measurements between 'start' and 'finish'
app.get("/measurements/:sensor/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getSensor = [req.params.sensor];
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const newMeasurement = await pool.query(
            "SELECT " + getSensor + ",tstamp from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "';"
        );

        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

//Sensor X measurements between 'start' and 'finish' in Imperial
app.get("/measurements/imperial/:sensor/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getSensor = [req.params.sensor];
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const newMeasurement = await pool.query(
            "SELECT " + getSensor + ",tstamp from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "';"
        );

        var i = 0;
        var rows = newMeasurement.rowCount;
        var cTemperature = 0;
        var mpsWind = 0;
        var fTemperature = 0;
        var mphWind = 0;

        console.log(rows);
        for (i; i < rows; i++) {

            if (getSensor == "temperature") {
                //Celsius -> Fahrenheit 
                cTemperature = newMeasurement.rows[i].temperature;
                fTemperature = CelsiustoFahrenheit(cTemperature);
                newMeasurement.rows[i].temperature = fTemperature;
            } else if (getSensor == "wind") {
                //Meters per second -> Miles per hour
                mpsWind = newMeasurement.rows[i].wind;
                mphWind = MPStoMPH(mpsWind);
                newMeasurement.rows[i].wind = mphWind;
            }
        }
        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

// Min value of sensor X in interval Y
app.get("/measurements/:sensor/min/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getSensor = [req.params.sensor];
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const query = "SELECT " + getSensor + ",tstamp from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish +
            "'AND " + getSensor + " = (select min(" + getSensor + ") from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "');";
        //console.log(query);

        const newMeasurement = await pool.query(query);
        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

// Min value of sensor X in interval Y IMPERIAL
app.get("/measurements/imperial/:sensor/min/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getSensor = [req.params.sensor];
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const query = "SELECT " + getSensor + ",tstamp from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish +
            "'AND " + getSensor + " = (select min(" + getSensor + ") from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "');";
        //console.log(query);

        const newMeasurement = await pool.query(query);

        var i = 0;
        var rows = newMeasurement.rowCount;
        var cTemperature = 0;
        var mpsWind = 0;
        var fTemperature = 0;
        var mphWind = 0;
        console.log(rows);
        for (i; i < rows; i++) {

            if (getSensor == "temperature") {
                //Celsius -> Fahrenheit 
                cTemperature = newMeasurement.rows[i].temperature;
                fTemperature = CelsiustoFahrenheit(cTemperature);
                newMeasurement.rows[i].temperature = fTemperature;
            } else if (getSensor == "wind") {
                //Meters per second -> Miles per hour
                mpsWind = newMeasurement.rows[i].wind;
                mphWind = MPStoMPH(mpsWind);
                newMeasurement.rows[i].wind = mphWind;
            }
        }

        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

// Max value of sensor X in interval Y
app.get("/measurements/:sensor/max/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getSensor = [req.params.sensor];
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const query = "SELECT " + getSensor + ",tstamp from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish +
            "'AND " + getSensor + " = (select MAX(" + getSensor + ") from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "');";
        //console.log(query);

        const newMeasurement = await pool.query(query);

        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});

// Max value of sensor X in interval Y IMPERIAL
app.get("/measurements/imperial/:sensor/max/:start/:finish", async(req, res) => {
    //YYYY-MM-DD HH:MM:SS
    var getSensor = [req.params.sensor];
    var getStart = [req.params.start];
    var getFinish = [req.params.finish];

    try {
        const query = "SELECT " + getSensor + ",tstamp from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish +
            "'AND " + getSensor + " = (select MAX(" + getSensor + ") from measurements " +
            "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getFinish + "');";
        //console.log(query);

        const newMeasurement = await pool.query(query);

        var i = 0;
        var rows = newMeasurement.rowCount;
        var cTemperature = 0;
        var mpsWind = 0;
        var fTemperature = 0;
        var mphWind = 0;
        console.log(rows);
        for (i; i < rows; i++) {

            if (getSensor == "temperature") {
                //Celsius -> Fahrenheit 
                cTemperature = newMeasurement.rows[i].temperature;
                fTemperature = CelsiustoFahrenheit(cTemperature);
                newMeasurement.rows[i].temperature = fTemperature;
            } else if (getSensor == "wind") {
                //Meters per second -> Miles per hour
                mpsWind = newMeasurement.rows[i].wind;
                mphWind = MPStoMPH(mpsWind);
                newMeasurement.rows[i].wind = mphWind;
            }
        }

        res.json(newMeasurement.rows);

    } catch (err) {
        console.log(err.message);
    }
});
}