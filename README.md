<p align="center">

  <a href="https://mqtt.org/">
    <img alt="MQTT.org" src="https://mqtt.org/assets/img/mqtt-logo-ver.jpg" height="150" />
  </a>
  <a href="https://www.raspberrypi.org/">
    <img alt="Powered by Raspberry Pi" src="https://www.raspberrypi.org/app/uploads/2017/06/Powered-by-Raspberry-Pi-Logo_Outline-Colour-Screen-500x153.png" height="150" />
  </a>
</p>
<h1 align="center">
  Automatic Discovery of MQTT Broker
</h1>
Contains code for an automatic MQTT discovery service for Raspberry Pi (or another linux system that supports systemd). The service responds to broadcast requests. Broadcast was chosen because I could not manage to find a way to do multicast from an ESP32 from within the Arduino environment (if you know how to do this, please let me know!). So, broadcast it is.

When a broadcast packet is received that starts with the "magic" sequence, a unicast packet is sent back with the address and port of the MQTT broker.

A few assumptions are made for the moment. Eventually a configuration file will be added. For now, the code assumes that:

* The MQTT broker is running on the same raspberry
* The raspberry is on wired internet (eth0)

If those assumptions are not true for you but you'd still like to use the code, no worries! These things are trivial to find and change. Go for it!

### ⁉ Why?

Yes, I could simply hard code the address of the broker into my IOT projects. But then I have to go fix this everywhere if a change is made to my network configuration, if I want to try it on a different network, etc. Since I need to install an MQTT broker anyway to use MQTT, it's not any extra burden to also install this little bit of code next to it.

### 🚧 Build and Install

Building is simple, just clone this project to your raspberry, switch to the base project directory, and type `make`. A few seconds later you should have a shiny, new executable in the `bin` directory.

You can simply run this file, and if you're happy to log into your raspberry and run it when you need it then you're done.

If you want it to start when your raspberry boots, then do the following:

Install the executable binary:

 `sudo cp bin/mqtt-discovery /usr/local/bin`

Install the service description file: 

`sudo cp mqtt-discovery.service /etc/systemd/system`

Check that things ended up in their proper place by attempting to start the service: 

`sudo systemctl start mqtt-discovery.service`

Look at the system log to see if it started: 

`sudo less +G /var/log/messages`

You should see something similar to:

`Nov 24 11:26:23 octopi mqtt-discovery[661]: listening at: 192.168.31.2:2112`

### 👀 Find it

Later I will need to create a little standalone project to show how to use this from the client side. For now, please refer to my other project: [ve.direct-hex-mqtt](https://github.com/pmcg31/ve.direct-hex-mqtt). Connecting to this server and obtaining the address of the broker are handled in the files `mqtt-discovery.hpp` and `mqtt-discovery.cpp`. They are used in `main.cpp` in the setup function.

<p align="center" style="padding-top: 50">🍀 Good Luck! 🍀
