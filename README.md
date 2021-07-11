# display-and-USB-host-with-ESP32
Example of ESP32 with USB host adapter (MAX3421E) and OLED display (SSD1309) 

An Example of combinations of
- ESP32 (ESP-WROOM-32)
- USB Host adapter board (MAX3421E)
- OLED display (controller : SSD1309), SPI connection
- Adafruit display control library
- <a href="https://github.com/felis/USB_Host_Shield_2.0">USB_Host_Shield_2.0 library</a>

Hardware SPI interface on ESP32 could not be shared by USB and OLED, so OLED is controlled via software SPI with remainging GPIO (general PIO) ports.

Two sample programs are included. 
- merge : Simple key code shown on OLED
- calc : always accurate RPN calc, <a href="https://youtu.be/ymGBZuGeglc">YouTube</a>
