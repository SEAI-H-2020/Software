//
// Based on https://medium.com/swlh/automatic-api-documentation-in-node-js-using-swagger-dd1ab3c78284
//
const swaggerAutogen = require('swagger-autogen')()
const outputFile = './swagger_output.json'
const endpointsFiles = ['./index.js', 'Authentication_API.js', 'DB_API.js']
const doc = {
    info: {
        version: "0.1.0",
        title: "Smart Sensor Box API",
        description: `Documentation automatically generated by the <b>swagger.autogen</b> module.
                    <p> SEAI H - The Software Team </p>
                    <p><b>WARNING:</b> In development, some of these APIs do not have failchecks yet<p>`

    },
    host: "smartsensorbox.ddns.net:5000",
    basePath: "/",
    schemes: ['http'],
    consumes: ['application/json'],
    produces: ['application/json'],
    tags: [{
        "name": "Measurements",
        "description": "Endpoints to upload or read measurement data"
    },
    {
        "name": "User Settings",
        "description": "Modify and view user settings such as sync and sample time"
    },
    {
        "name": "Authentication",
        "description": "Endpoints to authenticate users and view user data"
    },
    {
        "name": "Export",
        "description": "Export sensor measurements into different formats"
    }
    ],
    definitions: {
        NewMeasurement: {
            $temperature: 15,
            $humidity: 80,
            $wind: 2400,
            $noise_level: 65,
            $voltage: 3.3
        },
        NewMeasurement_tstmap: {
            $tstamp: "'2020-12-22 22:49:39'",
            $temperature: 15,
            $humidity: 80,
            $wind: 2400,
            $noise_level: 65,
            $voltage: 3.3
        },
        NewUserSetting: {
            $sample_time: 5,
            $username: "banana",
            $email: "banana@fe.up.pt"
        },

        MultipleMeasures: [{
            $temperature: 11,
            $humidity: 80,
            $wind: 2.5,
            $noise_level: 65,
            $voltage: 3.3
        }, {
            $temperature: 15,
            $humidity: 80,
            $wind: 2.3,
            $noise_level: 85,
            $voltage: 3.2
        }],
        MultipleMeasures_tstamp: [{
            $tstamp: "'2020-12-22 22:49:39'",
            $temperature: 11,
            $humidity: 80,
            $wind: 2.5,
            $noise_level: 65,
            $voltage: 3.3
        }, {
            $tstamp: "'2020-12-22 22:50:39'",
            $temperature: 15,
            $humidity: 80,
            $wind: 2.3,
            $noise_level: 85,
            $voltage: 3.2
        }]
    }
}


swaggerAutogen(outputFile, endpointsFiles, doc).then(() => {
    require('./index.js')
});