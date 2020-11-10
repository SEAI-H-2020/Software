import firebase_admin
import datetime
import time
import random
import argparse

from pyowm import OWM
from firebase_admin import credentials
from firebase_admin import db

# CHANGE DB CONFIGS HERE
URL='https://testdb-1fb74.firebaseio.com/'
keyPath='serviceAccountKey.json'

def sign_in_rtdb(keyfile_path=keyPath, url=URL):
    # Fetch the service account key JSON file contents
    cred = credentials.Certificate(keyfile_path)

    # Initialize the app with a service account, granting admin privileges
    firebase_admin.initialize_app(cred, {
        'databaseURL': url
    })
    return db

def sign_in_openweatermap(apikey='33ad830a6029caa22792ea214464f035'):
    owm = OWM(apikey)
    mgr = owm.weather_manager()
    return mgr

def generate_fake_data():
    measurements = {}
    measurements['temp'] = random.gauss(20, 1)
    measurements['hum'] = random.gauss(60, 10)
    measurements['windspeed'] = random.gauss(5, 1)
    measurements['noise'] = random.gauss(50, 20)
    measurements['voltage'] = random.gauss(3.3, 0.1)
    return measurements


def retrieve_real_weather_data(mgr, location = 'Porto,Pt'):
    observation = mgr.weather_at_place(location)
    w = observation.weather
    measurements = {}
    measurements['temp'] = w.temperature('celsius')['temp']
    measurements['hum'] = w.humidity
    measurements['windspeed'] = w.wind()['speed']
    measurements['noise'] = random.gauss(50, 20)
    measurements['voltage'] = random.gauss(3.3, 0.1)
    return measurements



def publish_neasurements(db, measurements):
    sensor_values = {
        u'humidity': measurements['hum'],
        u'temperature': measurements['temp'],
        u'windspeed': measurements['windspeed'],
        u'noise_level': measurements['noise']
    }

    data = {
        u'sensor_values': sensor_values,
        u'battery': measurements['voltage'],
        #u'date': datetime.datetime.now(),
    }

    db.push(data)

    print(f'temp:{measurements["temp"]:.2f}ºC\t'
          f' hum:{measurements["hum"]:.2f}%\t '
          f'windspeed:{measurements["windspeed"]:.2f}km/h\t'
          f' noise:{measurements["noise"]:.2f}dB\t'
          f' batt:{measurements["voltage"]:.2f}V')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')
    parser.add_argument("--real", action='store_true', help='Uses real data for OpenWeatherMap')
    parser.add_argument("--location", type=str, default='Porto, Pt', help='Specify location from openweathermap for temp, hum and windspeed ')
    parser.add_argument("--clear", action='store_true', help="Deletes DB before starting")
    parser.add_argument("-n", type=int, default=100, help='Number of measurements')
    parser.add_argument("-t", type=int, default=60, help='Period of measurements in seconds')
    args = parser.parse_args()

    db = sign_in_rtdb()
    db = db.reference(u'SimBox/Measurements')
    if args.clear:
        print('Deleting...')
        docs = db.delete()
        print('Deleted')
    if args.real or args.location:
        mgr = sign_in_openweatermap()

    for measure in range(args.n):
        if args.real:
            measurements = retrieve_real_weather_data(mgr,args.location)
        else:
            measurements = generate_fake_data()
        publish_neasurements(db, measurements)
        time.sleep(args.t)

    print('All Done.')