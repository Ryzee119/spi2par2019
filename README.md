
<![endif]-->

# spi2par2019

## What is it?
This is a recreation of a legacy Original Xbox adaptor that allowed you to use extremely common and cheap HD44780 compliant character LCD displays with the Xenium modchip SPI interface. The legacy adaptor was called 'spi2par' and has long since been out of production and extremely hard to come by.

I have never had one either, so this design has been made from the ground up.

There are two PCB layouts.

1. More faithful to the legacy spi2par design.
2. An LCD backpack design that takes advantage of the standard pinout of most HD44780 LCD displays to minimise the amount of wiring needed.

The intent is to make this as open and easily accessible to anyone with minimal experience in soldering and programming.

## Features
1. Converts the Xenium SPI interface to HD44780 compliant parallel LCDs. These are extremely cheap and readily available LCD modules.
2. Software controllable brightness
3. Software controllable contrast
4. This feature never existed in the legacy. You can connect two additional wires to the motherboard LPC header, and the board will read fan speed and GPU/CPU temperatures directly from the Xbox System Management bus. It will display and update this mid-game. Traditionally the LCD will just pause until you renter the dashboard. As the values are read directly from the ADM1032 temperature monitor chip, the update rate is fast, and the values match exactly those reported by most dashboards. Technically this can be altered to read ANYTHING available on the SMC bus.

## Assembly and Materials Required

Will add soon

## Programming

Will add soon - Source code as it stands is available in this repo.

## Installation

![spi2par2019 connection diagram](https://i.imgur.com/asCjG0X.jpg)

## Further Setup

Will come soon.