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
- mouse : Simple mouse button / motion on OLED, <a href="https://youtu.be/ue8MWqKrvLU">YouTube</a>
- calc : always accurate RPN calc, <a href="https://youtu.be/ymGBZuGeglc">YouTube</a>

Used parts:
- ESP32 https://ja.aliexpress.com/item/4000650306925.html
- USB host adapter https://www.amazon.co.jp/gp/product/B074HTPK13
- OLED display https://www.amazon.co.jp/gp/product/B08QZDJ4Q6
- 2.2uF capaciter
