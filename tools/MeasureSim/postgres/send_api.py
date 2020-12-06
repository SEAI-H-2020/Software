import random
import argparse
import requests
import time
from pyowm import OWM

# Default number of uploads
DEFAULT_MAX_UPLOADS = 10000
DEFAULT_SAMPLE_PERIOD_SECS = 60

# OpenWeatherMap
OWM_API_URL = '33ad830a6029caa22792ea214464f035'

def sign_in_openweatermap(apikey=OWM_API_URL):
    owm = OWM(apikey)
    mgr = owm.weather_manager()
    return mgr

def generate_fake_data():
    measurements = {}
    measurements['temp'] = random.gauss(20, 1)
    measurements['hum'] = random.gauss(60, 10)
    measurements['wind'] = random.gauss(5, 1)
    measurements['noise'] = random.gauss(50, 20)
    measurements['voltage'] = random.gauss(3.3, 0.1)
    return measurements


def retrieve_real_weather_data(mgr, location = 'Porto,Pt'):
    observation = mgr.weather_at_place(location)
    w = observation.weather
    measurements = {}
    measurements['temp'] = w.temperature('celsius')['temp']
    measurements['hum'] = w.humidity
    measurements['wind'] = w.wind()['speed']
    measurements['noise'] = random.gauss(50, 20)
    measurements['voltage'] = random.gauss(3.3, 0.1)
    return measurements

def clear_db(url, debug =False):
    res = requests.delete(url)
    if debug:
        print(res.content)
    res_json = res.json()
    print(f'Deleted {res_json["rowCount"]} rows')

def publish_neasurements(url, measurements, debug = False):
    data = {
        u'humidity': measurements['hum'],
        u'temperature': measurements['temp'],
        u'wind': measurements['wind'],
        u'noise_level': measurements['noise'],
        u'voltage': measurements['voltage']
    }

    res = requests.post(url, json = data)

    if debug:
        print(res.content)

    print(f'temp:{measurements["temp"]:.2f}ºC\t'
          f' hum:{measurements["hum"]:.2f}%\t '
          f'windspeed:{measurements["wind"]:.2f}km/h\t'
          f' noise:{measurements["noise"]:.2f}dB\t'
          f' batt:{measurements["voltage"]:.2f}V')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Send fake measurements trough REST API.')
    parser.add_argument("--url", type=str, required=True, help='URL for upload measurements rest API endpoint')
    parser.add_argument("--real", action='store_true', help='Uses real data for OpenWeatherMap')
    parser.add_argument("--location", type=str, default='Porto, Pt', help='Specify location from openweathermap for temp, hum and windspeed ')
    parser.add_argument("--clear", action='store_true', help="Deletes DB before starting")
    parser.add_argument("-n", type=int, default=DEFAULT_MAX_UPLOADS, help='Number of measurements')
    parser.add_argument("-t", type=int, default=DEFAULT_SAMPLE_PERIOD_SECS, help='Period of measurements in seconds')
    args = parser.parse_args()

    if args.clear:
        clear_db(args.url)

    if args.real:
        mgr = sign_in_openweatermap()

    for measure in range(args.n):
        if args.real:
            measurements = retrieve_real_weather_data(mgr,args.location)
        else:
            measurements = generate_fake_data()
        publish_neasurements(args.url, measurements, debug=False)
        time.sleep(args.t)
    
    print('All Done.')
