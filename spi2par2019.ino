#include "xeniumspi.h"
#include "xboxsmbus.h"
#include "src/SMWire/SMWire.h"
#include <LiquidCrystal.h>
#include <TFT.h>


//Some screens like difference contrast values. So I couldn't really set a good default value
//Play around and adjust to suit your screen. If you have flickering issues, it may help to solder
//an extra electrolytic capacitor between the contrast pin (normally labelled 'V0') and GND.
#define DEFAULT_CONTRAST 100 //0-255. Lower is higher contrast.
#define DEFAULT_BACKLIGHT 255 //0-255. Higher is brighter.
//#define USE_FAHRENHEIT 1 //Uncomment this line to make the in-game temp readouts display in Fahrenheit.


//HD44780 LCD Setup
const uint8_t rs = 18, en = 8, d4 = 7, d5 = 6, d6 = 5, d7 = 4; //HD44780 compliant LCD display pin numbers
const uint8_t ss_in = 17, miso = 14, mosi = 16, sck = 15, ss_out = A2; //SPI pin numbers, ss_in is the CS for the slave. As Xenium doesn't seem to use CS, ss_out is connected to ss_in on the PCB to manually trigger CS.
const uint8_t i2c_sda = 2, i2c_scl = 3; //i2c pins for SMBus
const uint8_t backlightPin = 10, contrastPin = 9; //Pin nubmers for backlight and contrast. Must be PWM enabled
uint8_t cursorPosCol = 0, cursorPosRow = 0; //Track the position of the cursor
uint8_t wrapping = 0, scrolling = 0; //Xenium spi command toggles for the lcd screen.
LiquidCrystal hd44780(rs, en, d4, d5, d6, d7); //Constructor for the LCD.


//SPI Data
uint8_t RxCommandQueue[256]; //Input FIFO buffer for raw SPI data from Xenium
uint8_t QueueProcessPos; //Tracks the current position in the FIFO queue that is being processed
uint8_t QueueRxPos; //Tracks the current position in the FIFO queue of the unprocessed input data (raw realtime SPI data)
uint8_t SPIState; //SPI State machine flag to monitor the SPI bus state can = SPI_ACTIVE, SPI_IDLE, SPI_SYNC, SPI_WAIT
uint32_t SPIIdleTimer; //Tracks how long the SPI bus has been idle for



//I2C Bus
uint32_t SMBusTimer; //Timer used to trigger SMBus reads
uint8_t i2cCheckCount = 0;      //Tracks what check we're up to of the i2c bus busy state
uint8_t I2C_BUSY_CHECKS = 250;  //To ensure we don't interfere with the actual Xbox's SMBus activity, we check the bus for activity for sending.
//This is how many times we check! Higher=slower but better. 250 is probably overkill


//SPI Bus Receiver Interrupt Routine
ISR (SPI_STC_vect) {
  RxCommandQueue[QueueRxPos] = SPDR;
  QueueRxPos++; //This is an unsigned 8 bit variable, so will reset back to 0 after 255 automatically
  SPIState = SPI_ACTIVE;
}

void setup() {
  pinMode(ss_out, OUTPUT);
  digitalWrite(ss_out, HIGH); //Force SPI CS signal high while we setup
  hd44780.begin(20, 4);
  hd44780.noCursor();


  //I put my logic analyser on the Xenium SPI bus to confirm the bus properties.
  //The master clocks at ~16kHz. SPI Clock is high when inactive, data is valid on the trailing edge (CPOL/CPHA=1. Also known as SPI mode 3)
  SPCR |= _BV(SPE);   //Turn on SPI. We don't set the MSTR bit so it's slave.
  SPCR |= _BV(SPIE);  //Enable to SPI Interrupt Vector
  SPCR |= _BV(CPOL);  //SPI Clock is high when inactive
  SPCR |= _BV(CPHA);  //Data is Valid on Clock Trailing Edge
  digitalWrite(ss_out, LOW); //SPI Slave is ready to receive data

  Wire.begin(0xDD); //Random address that is different from existing bus devices.
  TWBR = ((F_CPU / 72000) - 16) / 2; //Change I2C frequency closer to OG Xbox SMBus speed. ~72kHz Not compulsory really, but a safe bet

  analogWrite(backlightPin, DEFAULT_BACKLIGHT); //0-255 Higher number is brighter.
  analogWrite(contrastPin, DEFAULT_CONTRAST); //0-255 Lower number is higher contrast
  
  //Speed up PWM frequency. Gets rid of flickering
  TCCR1B &= 0b11111000;
  TCCR1B |= (1<<CS00);//Change Timer Prescaler for PWM

  hd44780.setCursor(0, 0);

}

void loop() {
  delayMicroseconds(50);

  //SPI to Parallel Conversion State Machine
  //One completion of processing command, set the buffer data value to 0
  //to indicate processing has been completed.
  if (QueueRxPos != QueueProcessPos) {
    switch (RxCommandQueue[QueueProcessPos]) {
      case 0:
        //No action required.
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumHideDisplay:
        hd44780.noDisplay();
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumShowDisplay:
        hd44780.display();
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumHideCursor:
        hd44780.noCursor();
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumShowUnderLineCursor:
      case XeniumShowBlockCursor:
      case XeniumShowInvertedCursor:
        //hd44780.cursor();
        //hd44780.blink();
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumBackspace:
        if (cursorPosCol > 0) {
          cursorPosCol--;
          hd44780.setCursor(cursorPosCol, cursorPosRow);
          hd44780.print(" ");
          hd44780.setCursor(cursorPosCol, cursorPosRow);
        }
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumLineFeed: //Move Cursor down one row, but keep column
        if (cursorPosRow < 3) {
          cursorPosRow++;
          hd44780.setCursor(cursorPosCol, cursorPosRow);
        }
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumDeleteInPlace: //Delete the character at the current cursor position
        hd44780.print(" ");
        hd44780.setCursor(cursorPosCol, cursorPosRow);
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumFormFeed: //Formfeed just clears the screen and resets the cursor.
        hd44780.clear();
        cursorPosRow = 0;
        cursorPosCol = 0;
        hd44780.setCursor(cursorPosCol, cursorPosRow);
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumCarriageReturn: //Carriage returns moves the cursor to the start of the current line
        cursorPosCol = 0;
        hd44780.setCursor(cursorPosCol, cursorPosRow);
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumSetCursorPosition: //Sets the row and column of cursor. The following two bytes are the row and column.
        if (RxCommandQueue[(uint8_t)(QueueProcessPos + 3)] != 0) {
          uint8_t col = RxCommandQueue[(uint8_t)(QueueProcessPos + 1)]; //Column
          uint8_t row = RxCommandQueue[(uint8_t)(QueueProcessPos + 2)]; //Row
          if (col < 20 && row < 4) {
            hd44780.setCursor(col, row);
            cursorPosCol = col, cursorPosRow = row;
          }
          RxCommandQueue[(uint8_t)(QueueProcessPos)]     = 0;
          RxCommandQueue[(uint8_t)(QueueProcessPos + 1)] = 0;
          RxCommandQueue[(uint8_t)(QueueProcessPos + 2)] = 0;
          RxCommandQueue[QueueProcessPos] = 0;
        }
        break;

      case XeniumSetBacklight:
        //The following byte after the backlight command is the brightness value
        //Value is 0-100 for the backlight brightness. 0=OFF, 100=ON
        //AVR PWM Output is 0-255. We multiply by 2.55 to match AVR PWM range.
        if ((QueueRxPos - QueueProcessPos) > 1 || (QueueRxPos - 1) < QueueProcessPos) { //ensure the command is complete
          uint8_t brightness = RxCommandQueue[(uint8_t)(QueueProcessPos + 1)];
          if (brightness >= 0 && brightness <= 100) {
            analogWrite(backlightPin, (uint8_t)(brightness * 2.55f)); //0-255 for AVR PWM
          }
          RxCommandQueue[QueueProcessPos] = 0;
          RxCommandQueue[(uint8_t)(QueueProcessPos + 1)] = 0;
        }
        break;

      case XeniumSetContrast:
        //The following byte after the contrast command is the contrast value
        //Value is 0-100 0=Very Light, 100=Very Dark
        //AVR PWM Output is 0-255. We multiply by 2.55 to match AVR PWM range.
        if ((QueueRxPos - QueueProcessPos) > 1 || (QueueRxPos - 1) < QueueProcessPos) { //ensure the command is complete
          uint8_t contrastValue = 100 - RxCommandQueue[(uint8_t)(QueueProcessPos + 1)]; //needs to convert to 100-0 instead of 0-100.
          if (contrastValue >= 0 && contrastValue <= 100) {
            analogWrite(contrastPin, (uint8_t)(contrastValue * 2.55f));
          }
          RxCommandQueue[QueueProcessPos] = 0;
          RxCommandQueue[(uint8_t)(QueueProcessPos + 1)] = 0;
        }
        break;

      case XeniumReboot:
        cursorPosRow = 0;
        cursorPosCol = 0;
        hd44780.begin(20, 4);
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumCursorMove:
        //The following 2 bytes after the initial command is the direction to move the cursor
        //offset+1 is always 27, offset+2 is 65,66,67,68 for Up,Down,Right,Left
        if (RxCommandQueue[(uint8_t)(QueueProcessPos + 1)] == 27 &&
            RxCommandQueue[(uint8_t)(QueueProcessPos + 2)] != 0) {

          switch (RxCommandQueue[(uint8_t)(QueueProcessPos + 2)]) {
            case 65: //UP
              if (cursorPosRow > 0) cursorPosRow--;
              break;
            case 66: //DOWN
              if (cursorPosRow < 3) cursorPosRow++;
              break;
            case 67: //RIGHT
              if (cursorPosCol < 19) cursorPosCol++;
              break;
            case 68: //LEFT
              if (cursorPosCol > 0) cursorPosCol--;
              break;
            default:
              //Error: Invalid cursor direction
              break;
          }
          hd44780.setCursor(cursorPosCol, cursorPosRow);
          RxCommandQueue[QueueProcessPos] = 0;
          RxCommandQueue[(uint8_t)(QueueProcessPos + 1)] = 0;
          RxCommandQueue[(uint8_t)(QueueProcessPos + 2)] = 0;
        }
        break;

      //Scrolling and wrapping commands are handled here.
      //The flags are toggled, but are not implemented properly yet
      //My testing seems to indicates it's not really needed.
      case XeniumWrapOff:
        wrapping = 0;
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumWrapOn:
        wrapping = 1;
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumScrollOff:
        scrolling = 0;
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case XeniumScrollOn:
        scrolling = 1;
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      case  32 ... 255: //Just an ASCII character
        if (cursorPosCol < 20) {
          hd44780.setCursor(cursorPosCol, cursorPosRow);
          hd44780.write(RxCommandQueue[QueueProcessPos]);
          cursorPosCol++;
        }
        RxCommandQueue[QueueProcessPos] = 0;
        break;

      //Not implemented yet
      case XeniumDrawBarGraph:
      case XeniumCustomCharacter:
      case XeniumLargeNumber:
      default:
        //hd44780.setCursor(0, 0);
        //hd44780.print("Not implemented ");
        //hd44780.print(RxCommandQueue[QueueProcessPos], HEX);
        //hd44780.setCursor(cursorPosCol, cursorPosRow);
        RxCommandQueue[QueueProcessPos] = 0;
        digitalWrite(ss_out, HIGH); //Force a resync
        delay(50);
        digitalWrite(ss_out, LOW);
        break;
    }

    if (RxCommandQueue[QueueProcessPos] == 0) {
      QueueProcessPos++;
    }

  }

  /* State machine to monitor the SPI Bus idle state */
  //If SPI bus has been idle pulse the CS line to resync the bus.
  //Xenium SPI bus doesnt use a Chip select line to resync the bus so this is a bit hacky, but improved reliability
  if (SPIState == SPI_ACTIVE) {
    SPIState = SPI_IDLE;
    SPIIdleTimer = millis();

  } else if (SPIState == SPI_IDLE && (millis() - SPIIdleTimer) > 150) {
    SPIState = SPI_SYNC;
    digitalWrite(ss_out, HIGH);
    SPIIdleTimer = millis();

  } else if (SPIState == SPI_SYNC && (millis() - SPIIdleTimer) > 50) {
    digitalWrite(ss_out, LOW);
    SPIState = SPI_WAIT;
  }


  /* Read data from Xbox System Management Bus */
  //Check that i2cBus is free, it has been atleast 2 seconds since last call,
  //and the SPI Bus has been idle for >10 seconds (i.e in a game or app that doesnt support LCD)
  if (i2cBusy() == 0 &&
      (millis() - SMBusTimer)   > 2000 &&
      (millis() - SPIIdleTimer) > 10000) {

    char rxBuffer[20];    //Raw data received from SMBus
    char lineBuffer[20]; //Fomatted data for printing to LCD


    //Read the current fan speed directly from the SMC and print to LCD
    if (readSMBus(SMC_ADDRESS, SMC_FANSPEED, &rxBuffer[0], 1) == 0) {
      if (rxBuffer[0] >= 0 && rxBuffer[0] <= 50) { //Sanity check. Number should be 0-50
        snprintf(lineBuffer, sizeof lineBuffer, "FAN: %3u%% ", rxBuffer[0] * 2);
        hd44780.setCursor(0, 2); //Write to 3rd row of hd44780. 0=Row 1, 2=Row 3
        hd44780.print(lineBuffer);
      }
    }



    //Read Focus Chip to determine video resolution (for Version 1.4 console only)
    if (readSMBus(FOCUS_ADDRESS, FOCUS_PID, &rxBuffer[0], 2) == 0) {
      uint16_t PID = ((uint16_t)rxBuffer[1]) << 8 | rxBuffer[0];
      if (PID == 0xFE05) {
        readSMBus(FOCUS_ADDRESS, FOCUS_VIDCNTL, &rxBuffer[0], 2);
        uint16_t VID_CNTL0 = ((uint16_t)rxBuffer[1]) << 8 | rxBuffer[0];

        if (VID_CNTL0 & FOCUS_VIDCNTL_VSYNC5_6 && VID_CNTL0 & FOCUS_VIDCNTL_INT_PROG) {
          //Must be HDTV, interlaced (1080i)
          hd44780.print(" 1080i ");

        } else if (VID_CNTL0 & FOCUS_VIDCNTL_VSYNC5_6 && !(VID_CNTL0 & FOCUS_VIDCNTL_INT_PROG)) {
          //Must be HDTV, Progressive 720p
          hd44780.print(" 720p  ");

        } else if (!(VID_CNTL0 & FOCUS_VIDCNTL_VSYNC5_6) && VID_CNTL0 & FOCUS_VIDCNTL_INT_PROG) {
          //Must be SDTV, interlaced 480i
          hd44780.print(" 480i  ");

        } else if (!(VID_CNTL0 & FOCUS_VIDCNTL_VSYNC5_6) && !(VID_CNTL0 & FOCUS_VIDCNTL_INT_PROG)) {
          //Must be SDTV, Progressive 480p
          hd44780.print(" 480p  ");

        } else {
          hd44780.print(VID_CNTL0, HEX); //Not sure what it is. Print the code.
        }
      }

    //Read Conexant Chip to determine video resolution (for Version 1.0 to 1.3 console only)
    } else if (readSMBus(CONEX_ADDRESS, CONEX_2E, &rxBuffer[0], 1) == 0) {
      if ((uint8_t)(rxBuffer[0] & 3) == 3) {
        //Must be 1080i
       hd44780.print(" 1080i ");

      } else if ((uint8_t)(rxBuffer[0] & 3) == 2) {
        //Must be 720p
        hd44780.print(" 720p  ");

      } else if ((uint8_t)(rxBuffer[0] & 3) == 1 && rxBuffer[0]&CONEX_2E_HDTV_EN) {
        //Must be 480p
        hd44780.print(" 480p  ");
        
      } else {
        hd44780.print(" 480i  ");
      }

    }

    //Read the CPU and M/B temps directly from the ADM1032 System Temperature Monitor then print to LCD
    if (readSMBus(ADM1032_ADDRESS, ADM1032_CPU, &rxBuffer[0], 1) == 0 &&
        readSMBus(ADM1032_ADDRESS, ADM1032_MB, &rxBuffer[1], 1) == 0) {
      if (rxBuffer[0] < 200 && rxBuffer[1] < 200 && rxBuffer[0] > 0 && rxBuffer[1] > 0) {
    #ifdef USE_FAHRENHEIT
        snprintf(lineBuffer, sizeof lineBuffer, "CPU:%3u%cC M/B:%3u%cC ", (uint8_t)((float)rxBuffer[0] * 1.8 + 32.0), (char)223,
                 (uint8_t)((float)rxBuffer[1] * 1.8 + 32.0), (char)223);
      #else
        snprintf(lineBuffer, sizeof lineBuffer, "CPU:%3u%cC M/B:%3u%cC ", rxBuffer[0], (char)223, rxBuffer[1], (char)223);
      #endif

        hd44780.setCursor(0, 3); //Write temperatures to LCD row 3
        hd44780.print(lineBuffer);
      }

      //If reading the ADM1032 failed, revert to SMC. Xbox is probably a 1.6.
    } else if (readSMBus(SMC_ADDRESS, SMC_CPUTEMP, &rxBuffer[0], 1) == 0 &&
               readSMBus(SMC_ADDRESS, SMC_BOARDTEMP, &rxBuffer[1], 1) == 0) {
      if (rxBuffer[0] < 200 && rxBuffer[1] < 200 && rxBuffer[0] > 0 && rxBuffer[1] > 0) {
      #ifdef USE_FAHRENHEIT
        snprintf(lineBuffer, sizeof lineBuffer, "CPU:%3u%cF M/B:%3u%cF ", (uint8_t)((float)rxBuffer[0] * 1.8 + 32.0), (char)223,
                 (uint8_t)((float)rxBuffer[1] * 1.8 + 32.0), (char)223);
      #else
        snprintf(lineBuffer, sizeof lineBuffer, "CPU:%3u%cC M/B:%3u%cC ", rxBuffer[0], (char)223, rxBuffer[1], (char)223);
      #endif

        hd44780.setCursor(0, 3); //Write temperatures to LCD row 3
        hd44780.print(lineBuffer);
      }
    }
    SMBusTimer = millis();

  } else if (SPIState != SPI_WAIT) {
    SMBusTimer = millis();

  }
  

}

/*
   Function: Check if the SMBus/I2C bus is busy
   ----------------------------
     returns: 0 if busy is free, non zero if busy or still checking
*/
uint8_t i2cBusy() {
  if (digitalRead(i2c_sda) == 0 || digitalRead(i2c_scl) == 0) { //If either the data or clock line is low, the line must be busy
    i2cCheckCount = I2C_BUSY_CHECKS;
  } else {
    i2cCheckCount--; //Bus isn't busy, decrease check counter so we check multiple times to be sure.
  }
  return i2cCheckCount;
}

/*
   Function: Read the Xbox SMBus
   ----------------------------
     address: The address of the device on the SMBus
     command: The command to send to the device. Normally the register to read from
     rx: A pointer to a receive data buffer
     len: How many bytes to read
     returns: 0 on success, -1 on error.
*/
int8_t readSMBus(uint8_t address, uint8_t command, uint8_t* rx, uint8_t len) {
  Wire.beginTransmission(address);
  Wire.write(command);
  if (Wire.endTransmission(false) == 0) { //Needs to be false. Send I2c repeated start, dont release bus yet
    Wire.requestFrom(address, len);
    for (uint8_t i = 0; i < len; i++) {
      rx[i] = Wire.read();
    }
    return 0;
  } else {
    return -1;
  }

}
