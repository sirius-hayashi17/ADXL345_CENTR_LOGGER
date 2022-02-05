#include <DS1307RTC.h>
#include <TimeLib.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <avr/wdt.h>

File myFile;
const int chipSelect = 10;

String time;
tmElements_t tm;

int ADXL345 = 0x53; // The ADXL345 sensor I2C address SD0 -> GND
float X_out, Y_out, Z_out;  // Outputs ADXL345_1

void getAccelration(int accelSensor) {
  // === Read acceleromter data === //
  Wire.beginTransmission(accelSensor);
  Wire.write(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(accelSensor, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  X_out = ( Wire.read() | Wire.read() << 8); // X-axis value
  X_out = X_out / 256; //For a range of +-2g, we need to divide the raw values by 256, according to the datasheet
  Y_out = ( Wire.read() | Wire.read() << 8); // Y-axis value
  Y_out = Y_out / 256;
  Z_out = ( Wire.read() | Wire.read() << 8); // Z-axis value
  Z_out = Z_out / 256;
}

void setup() {
  Serial.begin(9600);
  wdt_enable(WDTO_2S);
  while (!Serial) ; // wait for serial
  delay(200);
  Serial.println("DataLogger Shield Test");
  pinMode(SS, OUTPUT);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD Card initialization failed!");
    return;
  }
  Serial.println("SD Card OK.");
  //ReadText();

  Wire.begin(); // Initiate the Wire library
  // Set ADXL345 in measuring mode
  Wire.beginTransmission(ADXL345); // Start communicating with the device
  Wire.write(0x2D); // Access/ talk to POWER_CTL Register - 0x2D
  // Enable measurement
  Wire.write(8); // (8dec -> 0000 1000 binary) Bit D3 High for measuring enable
  Wire.endTransmission();
  delay(10);

  // Off-set Calibration
  //X-axis
  Wire.beginTransmission(ADXL345);
  Wire.write(0x1E);  // X-axis offset register
  Wire.write(6);
  Wire.endTransmission();
  delay(10);

  //Y-axis
  Wire.beginTransmission(ADXL345); //Y:~ -265 Error: ~265 - 256 = 9 Zoffset:
  Wire.write(0x1F); // Y-axis offset register
  Wire.write(-3);
  Wire.endTransmission();
  delay(10);

  //Z-axis
  Wire.beginTransmission(ADXL345); //Z: ~292 Error: ~292 - 256 = 36 Zoffset: ~9 LSB
  Wire.write(0x20); // Z-axis offset register
  Wire.write(8);
  Wire.endTransmission();
  delay(10);

}

void loop() {
  getAccelration(ADXL345);
  time = Now() + String(X_out) + "," + String(Y_out) + "," + String(Z_out);
  wdt_reset();
  Serial.println(time);
  WriteText(time);
  delay(200);
}


void ReadText() {
  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void WriteText(String txt) {
  myFile = SD.open("test.txt", FILE_WRITE);
  if (myFile) {
    myFile.println(txt);
    myFile.close();
  }
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}


String Now() {
  String time = "";
  if (RTC.read(tm)) {
    //    time = String(tm.Hour+":"+tm.Minute+":"+tm.Secnd+" DAY : "+tm.Day+"/"+tm.Month+"/"+tmYearToCalendar(tm.Year));
    time += tm.Hour;
    time += ":";

    time += tm.Minute;
    time += ":";

    time += tm.Second;
    time += ",";

    //time+=tm.Day;
    //time+="/";

    //time+=tm.Month;
    //time+="/";

    //time+=tmYearToCalendar(tm.Year);
  }
  else {
    time = "NO";
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    }
    else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
  }
  return time;
}
