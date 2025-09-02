#ifndef __SENSOR_H__
#define __SENSOR_H__

typedef struct
{
    bool        active;
    bool        updated;
    uint32_t    next_meas;
} sensor_meta_st;

typedef struct 
{
    sensor_meta_st meta;
    uint32_t  pressure;
    float     temperature;
} sensor_bmp280_st;

typedef struct
{
    sensor_bmp280_st    bmp280;
} sensors_st;

void sensor_initialize(void);

#endif