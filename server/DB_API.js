module.exports = function(app, pool) {
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

    app.get("/measurements/average/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Measurements average between the "start" and "end" interval'
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const newMeasurement = await pool.query(
                "SELECT ROUND(AVG(temperature)::numeric,2) as avg_temperature, ROUND(AVG(humidity)::numeric,2) as avg_humidity, ROUND(AVG(wind)::numeric,2) as avg_wind, ROUND(AVG(noise_level )::numeric,2) as avg_noise_level " +
                "from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
            );
            newMeasurement.rows[0].avg_temperature = parseFloat(newMeasurement.rows[0].avg_temperature);
            newMeasurement.rows[0].avg_humidity = parseFloat(newMeasurement.rows[0].avg_humidity);
            newMeasurement.rows[0].avg_wind = parseFloat(newMeasurement.rows[0].avg_wind);
            newMeasurement.rows[0].avg_noise_level = parseFloat(newMeasurement.rows[0].avg_noise_level);
            //res.json(newMeasurement.rows);
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            console.log(jsonResult);
            res.json(jsonResult);

        } catch (err) {
            console.log(err.message);
        }
    });

    app.get("/measurements/average/:sensor/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Sensor average measurement between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const query = "SELECT ROUND(AVG(" + getSensor + ")::numeric,2)  as avg_" + getSensor +
                " from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';";
            //console.log(query);

            const newMeasurement = await pool.query(query);
            if (getSensor == "temperature") {
                newMeasurement.rows[0].avg_temperature = parseFloat(newMeasurement.rows[0].avg_temperature);
            } else if (getSensor == "humidity") {
                newMeasurement.rows[0].avg_humidity = parseFloat(newMeasurement.rows[0].avg_humidity);
            } else if (getSensor == "wind") {
                newMeasurement.rows[0].avg_wind = parseFloat(newMeasurement.rows[0].avg_wind);
            } else if (getSensor == "noise_level") {
                newMeasurement.rows[0].avg_noise_level = parseFloat(newMeasurement.rows[0].avg_noise_level);
            }

            res.json(newMeasurement.rows);

        } catch (err) {
            console.log(err.message);
        }
    });

    //Average imperial Measurements between 'start' and 'end'
    app.get("/measurements/imperial/average/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Measurements average (in imperial units) between the "start" and "end" interval'
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const newMeasurement = await pool.query(
                "SELECT ROUND(AVG(temperature)::numeric,2) as avg_temperature, ROUND(AVG(humidity)::numeric,2) as avg_humidity, ROUND(AVG(wind)::numeric,2) as avg_wind, ROUND(AVG(noise_level )::numeric,2) as avg_noise_level " +
                "from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
            );

            var cTemperature = newMeasurement.rows[0].avg_temperature;
            var mpsWind = newMeasurement.rows[0].avg_wind;

            //Celsius -> Fahrenheit 
            var fTemperature = CelsiustoFahrenheit(cTemperature);

            //Meters per second -> Miles per hour
            var mphWind = MPStoMPH(mpsWind);

            newMeasurement.rows[0].avg_temperature = fTemperature;
            newMeasurement.rows[0].avg_humidity = parseFloat(newMeasurement.rows[0].avg_humidity);
            newMeasurement.rows[0].avg_wind = mphWind;
            newMeasurement.rows[0].avg_noise_level = parseFloat(newMeasurement.rows[0].avg_noise_level);
            res.json(newMeasurement.rows);

        } catch (err) {
            console.log(err.message);
        }
    });

    //Average sensor easurements between 'start' and 'end'
    app.get("/measurements/imperial/average/:sensor/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Sensor average measurement (in imperial units) between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const query = "SELECT ROUND(AVG(" + getSensor + ")::numeric,2) as avg_" + getSensor +
                " from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';";
            //console.log(query);
            const newMeasurement = await pool.query(query);

            var cTemperature = 0;
            var mpsWind = 0;
            var fTemperature = 0;
            var mphWind = 0;

            if (getSensor == "temperature") {
                //Celsius -> Fahrenheit 
                cTemperature = newMeasurement.rows[0].avg_temperature;
                fTemperature = CelsiustoFahrenheit(cTemperature);
                newMeasurement.rows[0].avg_temperature = fTemperature;
            } else if (getSensor == "wind") {
                //Meters per second -> Miles per hour
                mpsWind = newMeasurement.rows[0].avg_wind;
                mphWind = MPStoMPH(mpsWind);
                newMeasurement.rows[0].avg_wind = mphWind;
            } else if (getSensor == "humidity") {
                newMeasurement.rows[0].avg_humidity = parseFloat(newMeasurement.rows[0].avg_humidity);
            } else if (getSensor == "noise_level") {
                newMeasurement.rows[0].avg_noise_level = parseFloat(newMeasurement.rows[0].avg_noise_level);
            }

            res.json(newMeasurement.rows);

        } catch (err) {
            console.log(err.message);
        }
    });


    // converts latest measurement from metric to imperial
    app.get("/measurements/imperial", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Gets most recent measurement (in imperial units)'
        */
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

    //Measurements between 'start' and 'end'
    app.get("/measurements/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Measurements between the "start" and "end" interval'
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const newMeasurement = await pool.query(
                "SELECT * from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
            );

            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            console.log(jsonResult);
            res.json(jsonResult);

        } catch (err) {
            console.log(err.message);
        }
    });

    //Measurements between 'start' and 'end' in imperial
    app.get("/measurements/imperial/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Measurement (in imperial units) between the "start" and "end" interval'
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const newMeasurement = await pool.query(
                "SELECT * from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
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

    //Sensor X measurements between 'start' and 'end'
    app.get("/measurements/:sensor/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Sensor measurement between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const newMeasurement = await pool.query(
                "SELECT " + getSensor + ",tstamp from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
            );

            res.json(newMeasurement.rows);

        } catch (err) {
            console.log(err.message);
        }
    });

    //Sensor X measurements between 'start' and 'end' in Imperial
    app.get("/measurements/imperial/:sensor/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Sensor measurement (in imperial units) between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const newMeasurement = await pool.query(
                "SELECT " + getSensor + ",tstamp from measurements WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
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
    app.get("/measurements/:sensor/min/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Minimum value of sensor measurement between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const query = "SELECT " + getSensor + ",tstamp from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd +
                "'AND " + getSensor + " = (select min(" + getSensor + ") from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "');";
            //console.log(query);

            const newMeasurement = await pool.query(query);
            res.json(newMeasurement.rows);

        } catch (err) {
            console.log(err.message);
        }
    });

    // Min value of sensor X in interval Y IMPERIAL
    app.get("/measurements/imperial/:sensor/min/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Minimum value of sensor measurement (in imperial units) between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const query = "SELECT " + getSensor + ",tstamp from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd +
                "'AND " + getSensor + " = (select min(" + getSensor + ") from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "');";
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
    app.get("/measurements/:sensor/max/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Maximum value of sensor measurement between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const query = "SELECT " + getSensor + ",tstamp from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd +
                "'AND " + getSensor + " = (select MAX(" + getSensor + ") from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "');";
            //console.log(query);

            const newMeasurement = await pool.query(query);

            res.json(newMeasurement.rows);

        } catch (err) {
            console.log(err.message);
        }
    });

    // Max value of sensor X in interval Y IMPERIAL
    app.get("/measurements/imperial/:sensor/max/:start/:end", async(req, res) => {
        /*
        Swagger Documentation:
        #swagger.tags = ['Measurements']
        #swagger.method = 'get'
        #swagger.description = 'Maximum value of sensor measurement (in imperial units) between the "start" and "end" interval'
        #swagger.parameters['sensor'] = {description: 'temperature, humidity, wind, noise_level', type: 'string'}
        #swagger.parameters['start'] = {description: 'YYYY-MM-DD HH:MM:SS', type: 'string'}
        #swagger.parameters['end'] = {description: "YYYY-MM-DD HH:MM:SS", type: "string"}
        */
        var getSensor = [req.params.sensor];
        var getStart = [req.params.start];
        var getEnd = [req.params.end];

        try {
            const query = "SELECT " + getSensor + ",tstamp from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd +
                "'AND " + getSensor + " = (select MAX(" + getSensor + ") from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "');";
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