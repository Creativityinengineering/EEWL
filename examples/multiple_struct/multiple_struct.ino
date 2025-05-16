/* .+

.context    : EEWL EEPROM wear level library
.title      : test power off safety of EEWL
.kind       : c++ source
.author     : Fabrizio Pollastri <mxgbot@gmail.com>
.site       : Revello - Italy
.creation   : 27-Nov-2022
.copyright  : (c) 2021 Fabrizio Pollastri
.license    : GNU Lesser General Public License

.description
  This application counts the number of power up/down cycles. At each
  power up this counter is read from EEPROM, incremented by one and
  saved again into EEPROM to be preserved accross power cycles.
  A circular buffer len of 10 is defined. This extends the EEPROM life
  10 times, about 1 million of power cycles.
  The application uses a version of EEWl where the set free of the old
  data block is removed. This simulates a power off falling between
  the new data write and the old data removal. In this way, it is tested
  the capability of the begin method to recover from this anomaly.

.- */

// EEPROM circular buffer configuration
#define BUFFER_START 0x4  // buffer start address
#define BUFFER_LEN 50     // number of data blocks
#define eewl_serial_debug // Kommentiere diese Zeile aus, um serielle Ausgaben zu deaktivieren

#include <EEPROM.h>
#include "eewl.h"

// define a structure of data to be saved into EEPROM (an example).
struct SystemParameters {
  uint8_t byte1 = 0;
  uint8_t byte2 = 0xF;
  uint8_t byte3 = 0xAA;
  uint8_t byte4 = 0xBB;
  uint8_t byte5 = 0xCC;
};

// create instances of the structure
SystemParameters systemParameters, systemParameters_temp;

// create EEWL object managing data
EEWL sysParams(systemParameters, BUFFER_LEN, BUFFER_START);

// Makro für serielle Ausgaben
#ifdef eewl_serial_debug
  #define DEBUG_PRINT(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x) // Nichts tun
#endif

void setup() {
  // initialize serial
  Serial.begin(9600);
  delay(100);
  
  // init EEWL object
  printEEPROMHex(0, 50);
  sysParams.begin();
  printEEPROMHex(0, 50);
  
  // Temporäre Werte für die neuen Parameter
  uint8_t temp1 = 0;
  uint8_t temp2 = 0xF0;
  uint8_t temp3 = 0xAA;

  // Setze die neuen Werte in systemParameters
  systemParameters.byte1 = temp1;
  systemParameters.byte2 = temp2;
  systemParameters.byte3 = temp3;

  // Aktualisiere die Systemparameter
  DEBUG_PRINT("first update...");
  updateSystemParameters();
  printEEPROMHex(0, 50);
  DEBUG_PRINT("second update...");
  updateSystemParameters();
  printEEPROMHex(0, 50);
  systemParameters.byte3 = 0xEE;
  DEBUG_PRINT("third update...");
  updateSystemParameters();
  printEEPROMHex(0, 50);
  
  DEBUG_PRINT("Simulating multiple writes to test wear leveling...");
  for (uint8_t i = 0; i < 50; ++i) {
    systemParameters.byte1 = i;
    
    // Aktualisiere die Systemparameter
    updateSystemParameters();
    
    delay(50);  // Simulate a delay between writes
    printEEPROMHex(0, 50);
  }

  sysParams.get(systemParameters_temp);
  DEBUG_PRINT("Final value at Index 0 after wear leveling test byte 1-3:");
  DEBUG_PRINT(systemParameters.byte1);
  DEBUG_PRINT(systemParameters.byte2);
  DEBUG_PRINT(systemParameters.byte3);
  
  printEEPROMHex(0, 500);
}

void loop() {
  delay(1000);
}

// Funktion zum Aktualisieren der Systemparameter
void updateSystemParameters() {
  // Zuerst die aktuellen Werte aus dem EEPROM lesen
  sysParams.get(systemParameters_temp);
  
  // Überprüfen und nur speichern, wenn sich die Werte geändert haben
  if (hasSystemParametersChanged(systemParameters_temp, systemParameters)) {
    sysParams.put(systemParameters);
  } else {
    DEBUG_PRINT("nothing to write...");
  }
}

// Funktion zum Vergleichen der Systemparameter
bool hasSystemParametersChanged(const SystemParameters& oldParams, const SystemParameters& newParams) {
  return (oldParams.byte1 != newParams.byte1) || 
         (oldParams.byte2 != newParams.byte2) || 
         (oldParams.byte3 != newParams.byte3) ||
         (oldParams.byte4 != newParams.byte4) ||
         (oldParams.byte5 != newParams.byte5);
}

uint16_t combineBytes(byte lowByte, byte highByte) {
  return (static_cast<uint16_t>(highByte) << 8) | static_cast<uint16_t>(lowByte);
}

void printEEPROMHex(int startAddress, int numBytes) {
  for (int i = 0; i < numBytes; i++) {
    byte value = EEPROM.read(startAddress + i);  // Byte aus dem EEPROM lesen
    if (i > 0) {
      Serial.print(" ");  // Leerzeichen zwischen den Werten
    }
    Serial.print(value, HEX);  // Wert als Hexadezimal ausgeben
  }
  Serial.println();  // Neue Zeile nach der Ausgabe
}
/**** END ****/
