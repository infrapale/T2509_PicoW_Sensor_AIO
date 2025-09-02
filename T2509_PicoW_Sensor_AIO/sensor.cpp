#include "Arduino.h"
#include "sensor.h"

#include "main.h"
#include "atask.h"
#include <Adafruit_BMP280.h>
#include "mqtt_task.h"

#define BMP280_ADDR   0x76

// function prototypes
void scan_i2c();
void print_bmp_data(void);
void bmp280_set_configuration(void);
void sensor_task(void);


// atask_st:            = {"Label          ", ival, next, state, prev, cntr, run, task_ptr };
atask_st sensor_handle  = {"Sensor Task    ", 1000,    0,     0,  255,    0,   1, sensor_task};

Adafruit_BMP280 bmp280;

sensors_st sensor =
{
    .bmp280 = {
        .meta = {.active=false, .updated=false, .next_meas=0},
        .pressure = 0, 
        .temperature = 0.0
    }
};

void sensor_initialize(void)
{
  uint8_t status = bmp280.begin(BMP280_ADDR);

  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor!!"));
    // while (1) delay(10);
  }
  else {    
    print_bmp_data();
    sensor.bmp280.meta.active = true;
  }
 
    atask_add_new(&sensor_handle);  
}

void sensor_task(void)
{
    switch(sensor_handle.state)
    {
        case 0:
            sensor_handle.state = 10;
            break;
        case 10:
            if(sensor.bmp280.meta.active && (millis() > sensor.bmp280.meta.next_meas))
            {
                sensor.bmp280.pressure = bmp280.readPressure();
                sensor.bmp280.temperature = bmp280.readTemperature();
                //Print the results
                Serial.printf("Temperature: %.2f °C \t", sensor.bmp280.temperature);
                Serial.printf("Pressure: %d Pa\n", sensor.bmp280.pressure);
                sensor.bmp280.meta.next_meas = millis() + 5000;
                sensor.bmp280.meta.updated = true;   //cleared by MQTT code
            }                     
            sensor_handle.state = 20;
            break;
        case 20:
            mqtt_update_sensor_values(&sensor);
            sensor_handle.state = 10;
            break;
        case 30:
            sensor_handle.state = 10;
            break;
    }
}

void print_bmp_data(void)
{
    Serial.print(F("SensorID was: 0x")); Serial.println(bmp280.sensorID(),16);
    Serial.print(F("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n"));
    Serial.print(F("   ID of 0x56-0x58 represents a BMP 280,\n"));
    Serial.print(F("        ID of 0x60 represents a BME 280.\n"));
    Serial.print(F("        ID of 0x61 represents a BME 680.\n"));
}

void bmp280_set_configuration(void)
{
  /* Default settings from the datasheet. */
  bmp280.setSampling( Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                      Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                      Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                      Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                      Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}
