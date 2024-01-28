/* 
 * Project Logic Introduction
 * Author: Evan Rust
 */

#include "Particle.h"
#include <Wire.h>
#include <SparkFunLSM9DS1.h>
#include <Adafruit_ADT7410.h>

#define SEND_EVENT_NAME "SEND-DATA"
#define PUBLISH_EVENT_NAME "PROCESS-DATA"

/*
 * Device types
 ----------------
 * Temperature: 0
 * IMU: 1
 */
#define DEVICE_TEMP 0
#define DEVICE_IMU 1
#define DEVICE_TYPE DEVICE_IMU

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);

// Run the application and system concurrently in separate threads
SYSTEM_THREAD(ENABLED);

static char buf[particle::protocol::MAX_EVENT_DATA_LENGTH + 1];
static volatile bool sendDataFlag = false;

// Show system, cloud connectivity, and application logs over USB
// View logs with CLI using 'particle serial monitor --follow'
SerialLogHandler logHandler(115200, LOG_LEVEL_INFO);

#if DEVICE_TYPE == DEVICE_TEMP
Adafruit_ADT7410 temp;
void getTemperature(void);
#endif

#if DEVICE_TYPE == DEVICE_IMU
LSM9DS1 imu;
void getAcceleration(void);
#define LSM9DS1_M	0x1E // Would be 0x1C if SDO_M is LOW
#define LSM9DS1_AG	0x6B // Would be 0x6A if SDO_AG is LOW
// Earth's magnetic field varies by location. Add or subtract 
// a declination to get a more accurate heading. Calculate 
// your's here:
// http://www.ngdc.noaa.gov/geomag-web/#declination
#define DECLINATION -8.58 // Declination (degrees) in Boulder, CO.
#endif

void handleSendData(const char *event, const char *data) {
    sendDataFlag = true;
}

void setup() {
    Log.info("Starting up...");

#if DEVICE_TYPE == DEVICE_TEMP
    if (!temp.begin()) {
        Log.error("ADT7410 failed to start");

        while (1)
            delay(500);
    }
#endif

#if DEVICE_TYPE == DEVICE_IMU
    imu.settings.device.commInterface = IMU_MODE_I2C;
    imu.settings.device.mAddress = LSM9DS1_M;
    imu.settings.device.agAddress = LSM9DS1_AG;

    if (!imu.begin()) {
        Log.error("LSM9DS1 failed to start");

        while (1)
            delay(500);
    }
#endif

    Particle.subscribe(SEND_EVENT_NAME, handleSendData);
}

void loop() {
    if (sendDataFlag) {
        sendDataFlag = false;

#if DEVICE_TYPE == DEVICE_TEMP
        getTemperature();
#endif
#if DEVICE_TYPE == DEVICE_IMU
        getAcceleration();
#endif

        Particle.publish(PUBLISH_EVENT_NAME, buf);
    }
}

#if DEVICE_TYPE == DEVICE_TEMP
void getTemperature() {
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);

    float tempC = temp.readTempC();
    float tempF = tempC * 9.0 / 5 + 32;

    writer.beginObject();
        writer.name("type").value("temp");
        writer.name("ts").value(Time.now());
        writer.name("tempF").value(tempF);
    writer.endObject();
}
#endif

#if DEVICE_TYPE == DEVICE_IMU
void getAcceleration() {
    memset(buf, 0, sizeof(buf));
    JSONBufferWriter writer(buf, sizeof(buf) - 1);

    imu.readAccel();

    writer.beginObject();
        writer.name("type").value("imu");
        writer.name("ts").value(Time.now());
        writer.name("x").value(imu.calcAccel(imu.ax));
        writer.name("y").value(imu.calcAccel(imu.ay));
        writer.name("z").value(imu.calcAccel(imu.az));
    writer.endObject();
}
#endif