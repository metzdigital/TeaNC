# TeaNC-sw
A Teenie Terminal Node Controller

## What is TeaNC?
TeaNC is a very compact terminal node controller with VHF radio, GPS, WiFi / Bluetooth all built in. The device is intended to run from a single 3.7V 800mAHr battery or USB. It has a LCD screen, two push buttons for settings, and slide switch for power. For further specs see the hardware page: [TeaNC-hw](https://github.com/metzdigital/TeaNC-hw).  

## How did TeaNC come about? 
During the intense fire season this year in California a firefighter went missing and was eventually found dead. This terrible tragedy made me wonder if something like the APRS system could be utilized to help track teams working in steep terrain. I wanted to make something small that wouldn't get in the way, that could be clipped to a backpack, and is relatively inexpensive. 

## Desired Modes of Operation / Focus on software
 * Team Tracking - Travels with an individual, broadcasts position data every 10 minutes, goal is to maximize battery life
   * Could also be useful for hikers doing multiple day trips in isolated regions
 * Strategic Digipeater - Travels with an individual or setup in strategic locations, goal is to get the signal out, broadcasts its location every 10 minutes, rebroadcasts received messages, can be run from battery or USB solar panel. 
 * igate - Folks playing at home that want to published received APRS messages to the internet, given the WiFi functionality it should be possible to setup a unit outside and connect it to an access point. 
 * Push data via Bluetooth SPP to APRSdroid via the KISS protocol 
