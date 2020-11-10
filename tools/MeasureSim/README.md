# MeasureSim

Tools to simulate sensor measurements and send them to Google Firestore or Real Time Database.

## Test Conditions

All scripts were tested under Windows 10 and using Python 3.7.9

## Pre-requisites (Windows)

Follow this [guide](https://phoenixnap.com/kb/how-to-install-python-3-windows) if you need to install python. Don't forget to add Python to your Path!

## Instalation (Ubuntu and Windows)

Start by cloning the repository and changing to the MeasureSim directory

To install all the dependencies using pip run:

```
python -m pip install -r requirements.txt
```

## Changing the configurations 

To modify the database URL and service key path change the scripts directly:
```
# CHANGE DB CONFIGS HERE
URL='<DATABASE_URL>'
keyPath='<SERVICE_KEY_PATH>'
```
If you don't have a service key please refer to the documentation:
> To authenticate a service account and authorize it to access Firebase services, you must generate a private key file in JSON format. To generate a private key file for your service account: In the Firebase console, open Settings > Service Accounts. Click Generate New Private Key, then confirm by clicking Generate Key.

## Running the script

To upload to firestore:
```
python .\send_firestore.py 
```
To upload to real time data base:
```
python .\send_rtdb.py 
```

## Additional arguments

There are additional arguments to control the frequency, clear the db, change location etc.
These can be checked using the `-h` argument.

```

PS C:\Users\ruimsc98\Desktop\Software\tools\MeasureSim> python .\send_firestore.py -h  
usage: send_firestore.py [-h] [--real] [--location LOCATION] [--clear] [-n N]
                         [-t T]

Send fake measurement to google firestore.

optional arguments:
  -h, --help           show this help message and exit
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
python .\send_firestore.py -t 5 -n 10 --clear --real --location "Porto,Pt" 
```

or

```
python .\send_rtdb.py -t 5 -n 10 --clear --real --location "Porto,Pt" 
```

**Note 1:** Noise and voltage measurements are stil simulated

**Note 2:** If you ommit the `--real` parameter the script simulates using gaussian distributions all values

