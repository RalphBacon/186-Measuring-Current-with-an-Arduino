// Do not remove the include below
#include "Arduino_INA219_I2C_No_Library.h"
#include <Wire.h>

// INA219 Registers used in this sketch
#define INA219_REG_CALIBRATION (0x5)
#define INA219_REG_CONFIG (0x0)
#define INA219_REG_CURRENT (0x04)

// INA219 Config values used to measure current in mA
#define INA219_CONFIG_BVOLTAGERANGE_16V 		(0x0000)// 0-16V Range
#define INA219_CONFIG_BVOLTAGERANGE_32V 		(0x2000)// 0-32V Range
#define INA219_CONFIG_GAIN_8_320MV 				6144	// 8 x Gain
#define INA219_CONFIG_BADCRES_12BIT 			384 	// Bus ADC resolution bits
#define INA219_CONFIG_SADCRES_12BIT_1S_532US 	24 		// 1 x 12=bit sample
#define INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS 7		// Continuous conversion (not triggered)
#define INA219_CONFIG_SADCRES_12BIT_8S_4260US 	(0x0058)// 8 x 12-bit shunt samples averaged together

// I2C address of the INA219 device (can be changed by soldering board)
byte response, hexAddress = 0x40;

void setup()
{
	//Debug monitor
	Serial.begin(9600);

	// Initialise I2C (default address of 0x40)
	Wire.begin();

	// Test that we can communicate with the device
	Wire.beginTransmission(hexAddress);

	// Set the calibration to 32V @2A by writing two bytes (4096, an int) to that register
	Wire.write(INA219_REG_CALIBRATION);
	Wire.write((4096 >> 8) & 0xFF);
	Wire.write(4096 & 0xFF);

	Wire.endTransmission();

	// Config
	Wire.beginTransmission(hexAddress);
	Wire.write(INA219_REG_CONFIG);

	// Set Config register stating we want:
	uint16_t config = INA219_CONFIG_BVOLTAGERANGE_16V // 16 volt, 2A range
						| INA219_CONFIG_GAIN_8_320MV  // 8 x Gain
						| INA219_CONFIG_BADCRES_12BIT // 12-bit bus ADC resolution
						| INA219_CONFIG_SADCRES_12BIT_8S_4260US // number of averaged samples
						| INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS; // Continuouis conversion
	Wire.write((config >> 8) & 0xFF);
	Wire.write(config & 0xFF);

	// Depending on sample number (8S) we delay here the correct amount for conversion to complete
	delayMicroseconds(4260);

	// See if something acknowledged the transmission
	response = Wire.endTransmission();
	if (response == 0)
	{
		Serial.print("I2C device found at hexAddress 0x");
		if (hexAddress < 16)
			Serial.print("0");
		Serial.println(hexAddress, HEX);
	}
	else if (response == 4) // unknown error
	{
		Serial.print("Unknown response at hexAddress 0x");
		if (hexAddress < 16)
			Serial.print("0");
		Serial.println(hexAddress, HEX);
	}

	// All done here
	Serial.println("Setup completed.");
}

void loop()
{
	// Initiate transmission to the device we want
	Wire.beginTransmission(hexAddress);

	// Tell device the register we want to write to (Current)
	Wire.write(INA219_REG_CURRENT);

	// Finish this "conversation"
	Wire.endTransmission();

	// Initiate transmission to the device we want
	Wire.beginTransmission(hexAddress);

	// Request the value (current in mA) two bytes
	Wire.requestFrom((int) hexAddress, 2);

	// Finish this conversation
	Wire.endTransmission();

	// delay required to allow INA219 to do the conversion (see samples)
	delayMicroseconds(4260);

	// Shift values to create properly formed integer
	// Note that no "conversation" start/end is required for a read
	// as the device is expecting this from the prep done above
	float value = ((Wire.read() << 8) | Wire.read());

	// Display the current being consumed
	// Current LSB = 100uA per bit (1000/100 = 10)
	Serial.print("Current (mA):");
	Serial.println((int) (value / 10));

	// Wait a bit so we don't flood the debug window
	delay(2000);
}
