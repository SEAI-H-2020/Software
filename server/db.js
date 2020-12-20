// Based on https://www.youtube.com/watch?v=ldYcgPKEZC8

const Pool = require("pg").Pool;

const pool = new Pool({
    user: "postgres",
    password: "postgres",
    host: "smartsensorbox.ddns.net",
    port: 5432,
    database: "seai"
});

module.exports = pool;