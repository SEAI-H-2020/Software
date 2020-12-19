#include "ssbsensors.h"
/*
  Can be improved if we send all the data in one request
*/
int post_measurements(measurement_t measurements[], uint8_t num_measurements){
    HTTPClient http;
    http.begin("http://smartsensorbox.ddns.net:5000/measurements");
    http.addHeader("Content-Type", "application/json");
    DynamicJsonDocument POSTdoc(1024); //Adjust size later
    // Add values in the document
    for (uint8_t i = 0; i < num_measurements; i++){
      JsonObject obj = POSTdoc.createNestedObject();
      obj["temperature"] = String(measurements[i].temp);
      obj["humidity"] = String(measurements[i].hum);
      obj["wind"] = String(measurements[i].windspeed);
      obj["noise_level"] = String(measurements[i].noise);
      obj["voltage"] = String(measurements[i].voltage);
    }
    String requestBody;
    // build Json string
    serializeJson(POSTdoc, requestBody);
    Serial.println(requestBody);
    // post Json string}
    int httpResponseCode = http.POST(requestBody);

    #if DEBUG
    Serial.print("HTTP payload: ");
    Serial.print(httpPayload);
    #endif
    // POST test
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
      
    
    // Close TCP connection 
    http.end();

    return 0;
   
}