#include "ssbsensors.h"

int post_measurements(measurement_t measurements[], uint8_t num_measurements, String endpoint){
    HTTPClient http;
    http.begin(endpoint);
    http.addHeader("Content-Type", "application/json");
    DynamicJsonDocument POSTdoc(1024); //Adjust size later //StaticJsonDocument<200> POSTdoc; //Adjust size later
    // Add values in the document
    for (uint8_t i = 0; i < num_measurements; i++){
      JsonObject obj = POSTdoc.createNestedObject();
      obj["temperature"] = String(measurements[i].temp);
      obj["humidity"] = String(measurements[i].hum);
      obj["wind"] = String(measurements[i].windspeed);
      obj["noise_level"] = String(measurements[i].noise);
      obj["voltage"] = String(measurements[i].voltage);
      obj["tstamp"] = String(measurements[i].timestamp);
    }
    String requestBody;
    // build Json string
    serializeJson(POSTdoc, requestBody);
    Serial.println("in posting");
    Serial.println(requestBody);
    //Serial.println(endpoint);
    
    //Debug Stuff
    int httpResponseCode = http.POST(requestBody);
    Serial.println(httpResponseCode);
    Serial.print(http.getString());


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
