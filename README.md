# rfidAuth
Toy rfid authentication system with an ESP8266

This repository contains material for this [blog article](https://project-kaat.github.io/articles/rfidAuth/rfidAuth.html).


## Structure

**hwCode** - Arduino IDE project. This is the device code

**gdm** - GDM automatic user selection workaround

**this directory** - Custom PAM module code


## About custom PAM module

Build and install with `make build && sudo make install`.
To use, pam configuration for the specific authentication system you're using, must be edited.
For gdm, edit _/etc/pam.d/gdm-password_ to include `auth    sufficient  pam_rfbs.so address port`, address and port being you device's ip and authentication port (80 by default).
