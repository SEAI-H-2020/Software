# MeasureSim

# MeasureSim

Tools to simulate sensor measurements and send them to our PostgreSQL database through REST APIs using a simple node.js backend server.

## Test Conditions

All scripts were tested under Windows 10 and using Python 3.7.9

## Pre-requisites (Windows)

Follow this [guide](https://phoenixnap.com/kb/how-to-install-python-3-windows) if you need to install python3. Don't forget to add Python to your Path!

Note!: If you installed node.js with additional features you probably already have python3 installed.

## Instalation (Ubuntu and Windows)

Start by cloning the repository and changing to the MeasureSim/postgres directory

To install all the dependencies using pip run:

```
python -m pip install -r requirements.txt
```


## Running the script

To upload to the raspberry pi server:
```
python .\send_api.py --url http://smartsensorbox.ddns.net:5000/measurements
```
To upload to a local database running on your pc:
```
python .\send_api.py --url http://localhost:5000/measurements
```
To upload to a generic backend server (as long as it has the APIs to receive the data):
```
python .\send_api.py --url http://<SERVER_URL>:<SERVER_PORT>/<API_ENDPOINT>
```

## Additional arguments

There are additional arguments to control the frequency, clear the db, change location etc.
These can be checked using the `-h` argument.

```
usage: send_api.py [-h] --url URL [--real] [--location LOCATION] [--clear]
                   [-n N] [-t T]

Send fake measurements trough REST API.

optional arguments:
  -h, --help           show this help message and exit
  --url URL            URL for upload measurements rest API endpoint
  --real               Uses real data for OpenWeatherMap
  --location LOCATION  Specify location from openweathermap for temp, hum and
                       windspeed
  --clear              Deletes DB before starting
  -n N                 Number of measurements
  -t T                 Period of measurements in seconds
```

## Real data example

Real weather data can be used from [OpenWeatherMap](https://openweathermap.org/city/2735943) using the `--real` argument. Location can also be changed using `--location` by default it uses "Porto,Pt".
To send a measurements every 5 seconds for a total of 10 measurents, using data from Porto and clear the db at startup run:

```
python .\send_api.py --url http://smartsensorbox.ddns.net:5000/measurements -t 5 -n 10 --clear --real --location "Porto,Pt" 
```


**Note 1:** Noise and voltage measurements are stil simulated

**Note 2:** If you ommit the `--real` parameter the script simulates all values using gaussian distributions


