

# spi2par2019

## What is it?
This is a recreation of a legacy Original Xbox adaptor that allowed you to use extremely common and cheap HD44780 compliant character LCD displays with the Xenium modchip SPI interface. The legacy adaptor was called 'spi2par' and has long since been out of production and extremely hard to come by.

I have never had one either, so this design has been made from the ground up.

There are two PCB layouts in this repository:

1. More faithful to the legacy spi2par design.
2. An LCD backpack design that takes advantage of the standard pinout of most HD44780 LCD displays to minimise the amount of wiring needed.

These two layouts are shown in the below photo. The LCD backpack has two pin arrangements so can be used on the two common LCD pin header layouts as shown installed. The intent is to make this as open and easily accessible to anyone with minimal experience in soldering and programming.
![enter image description here](https://raw.githubusercontent.com/Ryzee119/spi2par2019/master/images/boards.jpg)

## Features
1. Converts the Xenium SPI interface to HD44780 compliant parallel LCDs. These are extremely cheap and readily available LCD modules.
2. Software controllable brightness
3. Software controllable contrast
4. This feature never existed in the legacy. You can connect two additional wires to the motherboard LPC header, and the board will read fan speed and MB/CPU temperatures directly from the Xbox System Management bus. It will display and update this mid-game. Traditionally the LCD will just pause until you renter the dashboard.  Example below:
![enter image description here](https://raw.githubusercontent.com/Ryzee119/spi2par2019/master/images/ingame_temp.jpg)


## Assembly and Materials Required
There is two ways you can do this. Option 1 is the intended way and has a PCB daughter board to make installation easier.  <br> The second option uses less components but requires more wiring.

#### Option 1 - Arduino module and PCB adaptor
The standard design
|Part|Qty | Possible Source|
|--|--|--|
| Ryzee119's spi2par2019 PCB |1| OSHPark (Bit expensive, 3 boards): <br>[spi2par2019backpack](https://oshpark.com/shared_projects/HGCRYTFI) OR<br> [spi2par2019faithful](https://oshpark.com/shared_projects/7YvM7Fwu) <br> <br> [Elecrow (This is a referral link) ~5USD+shipping for 10 boards:](http://www.elecrow.com/referral-program/MTEzNjlqMnQ=/) <br> Download the required zip file from the [Gerbers Folder](https://github.com/Ryzee119/spi2par2019/tree/master/hardware/gerbers) and upload the zip file to their online ordering service. Use the following PCB properties: <br> `2 layers, 45x30mm, 1.6mm thick, HASL, 1oz copper, no castellated holes, any colour you want!`| 
| Arduino Pro Micro Leonardo 5V/16Mhz |1| [Any clones will work](https://www.aliexpress.com/item/New-Pro-Micro-for-arduino-ATmega32U4-5V-16MHz-Module-with-2-row-pin-header-For-Leonardo/32768308647.html). Make sure they're the 5V/16Mhz variant. | 
| N-Channel MOSFET 2N7002 SOT23 at position Q1| 1 |Find anywhere convenient <br> [Digikey](https://www.digikey.com.au/short/p4zbn8)<br> [Aliexpress](https://www.aliexpress.com/item/Free-Shipping-200PCS-2N7002-MOSFET-N-CH-60V-300MA-SOT-23/897983645.html) |
| JST SH1.0 10 pin 1mm connector (To connect to Xenium) | 1 |Digikey: [Connector](https://www.digikey.com.au/product-detail/en/jst-sales-america-inc/SHR-10V-S-B/455-1385-ND/759874) + 3x[Jumpers](https://www.digikey.com.au/product-detail/en/jst-sales-america-inc/ASSHSSH28K152/455-3076-ND/6009452) OR <br> Aliexpress: [Connector with Jumpers (Get the 10P one)](https://www.aliexpress.com/item/5PCS-100MM-SH-1-0-Wire-Cable-Connector-DIY-SH1-0-JST-2-3-4-5/32952366214.html)|
| General Hook-up Wire 26AWG or so| A metre or two |-|

Cost of getting ten made works out to roughly $4-5usd each.
1. Solder the MOSFET to position Q1.
2. Solder the Arduino module inplace, note the silkscreen markings for orientation.
3. If using the LCD backpack design, position the PCB the line up with the LCD header and solder in place. 
4. If using the 'Faithful' design, solder cables to each corresponding point between the LCD and the spi2par2019 PCB.
5. Program the Arduino as per the programming instructions below.
6. Wire as per the installation image below. Note the soldered jumper wire between the CS pad on the spi2par2019 PCB and the resistor on the Arduino. This is a stupid design limitation of the Arduino when being used as an SPI Slave so this jumper needs to be added.

#### Option 2 - Arduino module only
No extra PCB required, no backlight control, slightly harder soldering
|Part|Qty | Possible Source|
|--|--|--|
| Arduino Pro Micro Leonardo 5V/16Mhz |1| [Any clones will work](https://www.aliexpress.com/item/New-Pro-Micro-for-arduino-ATmega32U4-5V-16MHz-Module-with-2-row-pin-header-For-Leonardo/32768308647.html). Make sure they're the 5V/16Mhz variant. | 
| JST SH1.0 10 pin 1mm connector (To connect to Xenium) | 1 |Digikey: [Connector](https://www.digikey.com.au/product-detail/en/jst-sales-america-inc/SHR-10V-S-B/455-1385-ND/759874) + 3x[Jumpers](https://www.digikey.com.au/product-detail/en/jst-sales-america-inc/ASSHSSH28K152/455-3076-ND/6009452) OR <br> Aliexpress: [Connector with Jumpers (Get the 10P one)](https://www.aliexpress.com/item/5PCS-100MM-SH-1-0-Wire-Cable-Connector-DIY-SH1-0-JST-2-3-4-5/32952366214.html)|
| General Hook-up Wire 26AWG or so| A metre or two |-|

Cost of getting ten made works out to roughly $3-4usd each.

1.  Program the Arduino as per the programming instructions below.
2. Wire as per the alternative installation image below. Note the soldered jumper wire between the Pin A2 and the resistor on the Arduino. This is a stupid design limitation of the Arduino when being used as an SPI Slave so this jumper needs to be added.


## Programming

1. Clone this repository to your PC
2. Download then install the [Arduino IDE](https://www.arduino.cc/en/Main/Software).
3. Open `spi2par2019.ino` <br> ![enter image description here](https://i.imgur.com/4Hws0dc.jpg)
4. Compile by clicking the tick in the top left. <br> ![enter image description here](https://i.imgur.com/opNy2Fo.jpg)
5. Confirm it has compiled successfully from the console output. <br> ![enter image description here](https://i.imgur.com/iDf2zib.jpg)
6. Connect a MicroUSB cable between the Arduino Module and the PC. <br> ![enter image description here](https://i.imgur.com/orj2ahq.jpg)
7. Set the Board Type the `Arduino Leonardo` and the comport correctly. Mine happens to be `COM14`. <br> ![enter image description here](https://i.imgur.com/GXofSoA.jpg)
8. Click the upload button and confirm successful. The LCD backlight should come on but no text will be displayed until connected to a Xenium or the Xbox SMBus. <br> ![enter image description here](https://i.imgur.com/dCDJMdK.jpg)

## Installation

![spi2par2019 connection diagram](https://raw.githubusercontent.com/Ryzee119/spi2par2019/master/images/spi2par_connection.jpg)

#### Alternative Installation
![spi2par2019 connection diagram alt](https://raw.githubusercontent.com/Ryzee119/spi2par2019/master/images/spi2par_connection2.jpg)

## Further Setup

Will add soon.
