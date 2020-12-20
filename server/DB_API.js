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

    // Upload sensor measurements
    app.post("/measurements", async(req, res) => {
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
            VALUES ($1, $2, $3, $4, $5)", [req.body.temperature, req.body.humidity, req.body.wind,
                    req.body.noise_level, req.body.voltage
                ]
            );
            res.json(newMeasurement);
        } catch (err) {
            console.log(err.message);
        }
        console.log(req.body);
    });

    // Donwload csv 
    app.get("/csv", async(req, res) => {
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
    app.delete("/measurements", async(req, res) => {
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
            res.json(queryres);
        } catch (err) {
            console.log(err.message);
        }
    });

    app.post("/measurements/multiple", async(req, res) => {
        /*
            Swagger Documentation:
            #swagger.method = 'post'
            #swagger.tags = ['Measurements']
            #swagger.description = 'Add nested jsons to send multiple measurements.'
            #swagger.parameters['measurements'] = {
                in: 'body',
                description: 'multiple measurements from sensor box',
                required: true,
                type: 'object',
                schema: {$ref: "#/definitions/MultipleMeasures"}
            }
        */
        console.log("oyyyy");
        try {
            console.log(req.body);
            let measure_count = 0;
            const measurements = req.body;
            var newMeasurement;
            for (var key in req.body) {
                console.log(measurements[key]);
                //console.log(measurements[key]);
                newMeasurement = await pool.query(
                    "INSERT INTO measurements (temperature, humidity, wind, noise_level, voltage) \
                VALUES ($1, $2, $3, $4, $5)", [measurements[key].temperature, measurements[key].humidity,
                        measurements[key].wind, measurements[key].noise_level, measurements[key].voltage
                    ]
                );
                measure_count++;
            };
            result_string = `Inserted ${measure_count} measurements!`;
            console.log(result_string);
            res.send(result_string);
        } catch (err) {
            console.log(err.message);
        }
    });

    // Get most recent measurement
    app.get("/measurements", async(req, res) => {
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
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);
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

            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            ////console.log(jsonResult);
            res.json(jsonResult);

        } catch (err) {
            console.log(err.message);
        }
    });

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
                "SELECT ROUND(AVG(temperature)::numeric,2) as temperature, ROUND(AVG(humidity)::numeric,2) as humidity, ROUND(AVG(wind)::numeric,2) as wind, ROUND(AVG(noise_level )::numeric,2) as noise_level " +
                "from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
            );
            newMeasurement.rows[0].temperature = parseFloat(newMeasurement.rows[0].temperature);
            newMeasurement.rows[0].humidity = parseFloat(newMeasurement.rows[0].humidity);
            newMeasurement.rows[0].wind = parseFloat(newMeasurement.rows[0].wind);
            newMeasurement.rows[0].noise_level = parseFloat(newMeasurement.rows[0].noise_level);
            //res.json(newMeasurement.rows);
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            // //console.log(jsonResult);
            res.json(jsonResult);

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
                "SELECT ROUND(AVG(temperature)::numeric,2) as temperature, ROUND(AVG(humidity)::numeric,2) as humidity, ROUND(AVG(wind)::numeric,2) as wind, ROUND(AVG(noise_level )::numeric,2) as noise_level " +
                "from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
            );

            var cTemperature = newMeasurement.rows[0].temperature;
            var mpsWind = newMeasurement.rows[0].wind;

            //Celsius -> Fahrenheit 
            var fTemperature = CelsiustoFahrenheit(cTemperature);

            //Meters per second -> Miles per hour
            var mphWind = MPStoMPH(mpsWind);

            newMeasurement.rows[0].temperature = fTemperature;
            newMeasurement.rows[0].humidity = parseFloat(newMeasurement.rows[0].humidity);
            newMeasurement.rows[0].wind = mphWind;
            newMeasurement.rows[0].noise_level = parseFloat(newMeasurement.rows[0].noise_level);

            newMeasurement.rows[0].id = 1;
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            ////console.log(jsonResult);
            res.json(jsonResult);

        } catch (err) {
            console.log(err.message);
        }
    });

    app.get("/measurements/:sensor/average/:start/:end", async(req, res) => {
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
            const query = "SELECT ROUND(AVG(" + getSensor + ")::numeric,2)  as " + getSensor +
                " from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';";
            //console.log(query);

            const newMeasurement = await pool.query(query);

            if (getSensor == "temperature") {
                newMeasurement.rows[0].temperature = parseFloat(newMeasurement.rows[0].temperature);
            } else if (getSensor == "humidity") {
                newMeasurement.rows[0].humidity = parseFloat(newMeasurement.rows[0].humidity);
            } else if (getSensor == "wind") {
                newMeasurement.rows[0].wind = parseFloat(newMeasurement.rows[0].wind);
            } else if (getSensor == "noise_level") {
                newMeasurement.rows[0].noise_level = parseFloat(newMeasurement.rows[0].noise_level);
            }
            newMeasurement.rows[0].id = 1;
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            ////console.log(jsonResult);
            res.json(jsonResult);

        } catch (err) {
            console.log(err.message);
        }
    });

    //Average sensor easurements between 'start' and 'end'
    app.get("/measurements/imperial/:sensor/average/:start/:end", async(req, res) => {
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
            const query = "SELECT ROUND(AVG(" + getSensor + ")::numeric,2) as " + getSensor +
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
                cTemperature = newMeasurement.rows[0].temperature;
                fTemperature = CelsiustoFahrenheit(cTemperature);
                newMeasurement.rows[0].temperature = fTemperature;
            } else if (getSensor == "wind") {
                //Meters per second -> Miles per hour
                mpsWind = newMeasurement.rows[0].wind;
                mphWind = MPStoMPH(mpsWind);
                newMeasurement.rows[0].wind = mphWind;
            } else if (getSensor == "humidity") {
                newMeasurement.rows[0].humidity = parseFloat(newMeasurement.rows[0].humidity);
            } else if (getSensor == "noise_level") {
                newMeasurement.rows[0].noise_level = parseFloat(newMeasurement.rows[0].noise_level);
            }

            newMeasurement.rows[0].id = 1;
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            ////console.log(jsonResult);
            res.json(jsonResult);

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

            //console.log(jsonResult);
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

            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);

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
                "SELECT " + getSensor + ",tstamp, id " +
                "from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
            );

            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);

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
                "SELECT " + getSensor + ",tstamp, id " +
                "from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "';"
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
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);

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
            const query = "SELECT " + getSensor + ",tstamp, id from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd +
                "'AND " + getSensor + " = (select min(" + getSensor + ") from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "');";
            //console.log(query);

            const newMeasurement = await pool.query(query);
            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);

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
            const query = "SELECT " + getSensor + ",tstamp, id from measurements " +
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

            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);

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
            const query = "SELECT " + getSensor + ",tstamp, id from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd +
                "'AND " + getSensor + " = (select MAX(" + getSensor + ") from measurements " +
                "WHERE tstamp BETWEEN '" + getStart + "' AND '" + getEnd + "');";
            //console.log(query);

            const newMeasurement = await pool.query(query);

            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);

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
            const query = "SELECT " + getSensor + ",tstamp, id from measurements " +
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

            var jsonResult = JSON.stringify(newMeasurement.rows);

            var preamble = "{ \"measurement\" : ";
            jsonResult = preamble + jsonResult + "}";
            jsonResult = JSON.parse(jsonResult);

            //console.log(jsonResult);
            res.json(jsonResult);

        } catch (err) {
            console.log(err.message);
        }
    });
}