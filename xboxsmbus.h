/*
 * XBMC LCD Settings
 * You can what the LCD prints in XBMC dashboard by editing /system/UserData/LCD.xml file
 * Check https://redmine.exotica.org.uk/projects/xbmc4xbox/repository/xbmc4xbox/entry/branches/3.5/xbmc/GUIInfoManager.cpp to see what options are available
 * 
 */


/*
 * XBOX System Management Controller
 * See https://web.archive.org/web/20100708015155/http://www.xbox-linux.org/wiki/PIC
 * Note 1:  The PIC version can be read from register 0x01 as a 3-character ASCII string. Each time you read that register, the next of the 3 characters is returned. 
 *          "DBG" = DEBUGKIT Green
 *          "B11" = "DEBUGKIT Green"
 *          "P01" = "v1.0"
 *          "P05" = "v1.1"
 *          "P11" or "1P1" or "11P" = v1.2/v1.3 or v1.4. Need to read Video encoder. If FOCUS_ADDRESS returns valid = V1.4, else v1.2/1,3
 *          "P2L" or "1P1" or "11P" = v1.6
 *          
 * Note 2:  0x00  SCART
            0x01  HDTV
            0x02  VGA
            0x03  RFU
            0x04  S-Video
            0x05  undefined
            0x06  standard
            0x07  missing/disconnected
 */
#define SMC_ADDRESS 0x10  
#define SMC_VER 0x01             //PIC version string. Note 1.
#define SMC_TRAY 0x03            //tray state (Tray open=16, Tray closed no disk=64, Tray closed with disk=96, 49=Opening, 97=Eject pressed?, 81=Closing)
#define SMC_AVSTATE 0x04         //A/V Pack state Note 2.
#define SMC_CPUTEMP 0x09         //CPU temperature (°C) //Read from ADM1032 for better update rate
#define SMC_BOARDTEMP 0x0A       //board temperature (°C) //Read from ADM1032 for better update rate
#define SMC_FANSPEED 0x10        //current fan speed (0~50) multiply by 2 to get 0-100%


/* XBOX ADM1032 System Temperature Monitor (Ref http://xboxdevwiki.net/SMBus)
 * https://www.onsemi.com/pub/Collateral/ADM1032-D.PDF
 */
#define ADM1032_ADDRESS 0x4C  
#define ADM1032_MB 0x00             //°C
#define ADM1032_CPU 0x01            //°C


/* XBOX Conexant CX25871 Video Encoder. This chip was used in Xbox versions 1.0 through 1.3
 * https://pdf1.alldatasheet.com/datasheet-pdf/view/153349/CONEXANT/CX25871.html See Section 2.4 Reading Registers
 * Anything interesting to read from Conexant?
 */
#define CONEX_ADDRESS 0x45
#define CONEX_PID     0x00 //Returns 1 byte product ID. Bit 7-5=ID, Bit 4-0 = Version of chip read with readSMBus(CONEX_ADDRESS, CONEX_PID, &rxBuffer, 1)
#define CONEX_A2      0xA2
#define CONEX_A2_NI_OUT     (1<<0) //Bit zero of address 0xA2. 0=interlaced, 1=progressive
#define CONEX_2E      0x2E
#define CONEX_2E_HDTV_EN     (1<<7) //Bit 7 of address 0x2E. 1 = HDTV output mode enabled.


/* XBOX Focus FS453 Video Encoder, This chip was used in Xbox Version 1.4
 * https://www.manualslib.com/manual/52885/Focus-Fs453.html
 * https://assemblergames.com/attachments/focus-datasheet-pdf-zip.17803/
 * Anything interesting to read from Focus chip? See second attachment "2.1 Register Reference Table
 * for address offsets
 */
#define FOCUS_ADDRESS 0x6A 
#define FOCUS_PID 0x32 //Returns a 2 byte product ID. Low byte is first. Read with readSMBus(FOCUS_ADDRESS, FOCUS_PID, &rxBuffer, 2);
    /* This is the outputs from VID_CNTL0 register for each mode using component cable
       480P  48 3e  = 0100100000111110
       480I  48 c5  = 0100100011000101
       720P  58 2e  = 0101100000101110
       1080I 50 ae  = 0101000010101110
    */
#define FOCUS_VIDCNTL 0x92 //Video Control 0 register
#define FOCUS_VIDCNTL_VSYNC5_6 (1<<12) //Selects HDTV composite vertical sync width parameter. 1=HDTV
#define FOCUS_VIDCNTL_INT_PROG (1<<7)  //Interlaced/Progressive.  When set =1, input image is interlaced

/*
 * XBOX XCALIBUR For Xbox 1.6. I dont know anything about it?
 */
#define XCALIBUR_ADDRESS 0x70 


/* XBOX EEPROM
 * Could read anything stored in EEPROM, but doesnt seem like anything useful for a LCD screen
 * See https://web.archive.org/web/20081216023342/http://www.xbox-linux.org/wiki/Xbox_Serial_EEPROM for addresses
 * for different info
 */
#define EEPROM_ADDRESS 0x54  
#define EEPROM_SERIAL 0x34 //Read 12 bytes starting at 0x34.


 
