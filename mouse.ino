#include <hidboot.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_MOSI  16
#define OLED_CLK   22
#define OLED_DC    26
#define OLED_CS    21
#define OLED_RESET 14
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

int left, middle, right, dx, dy, x, y;
void disp(void);

class MouseRptParser : public MouseReportParser
{
protected:
  void OnMouseMove  (MOUSEINFO *mi);
  void OnLeftButtonUp (MOUSEINFO *mi);
  void OnLeftButtonDown (MOUSEINFO *mi);
  void OnRightButtonUp  (MOUSEINFO *mi);
  void OnRightButtonDown  (MOUSEINFO *mi);
  void OnMiddleButtonUp (MOUSEINFO *mi);
  void OnMiddleButtonDown (MOUSEINFO *mi);
};
void MouseRptParser::OnMouseMove(MOUSEINFO *mi)
{
  dx = mi->dX;
  dy = mi->dY;
  x += dx;
  y += dy;
  disp();
};
void MouseRptParser::OnLeftButtonUp (MOUSEINFO *mi)
{
  left = false;
  disp();
};
void MouseRptParser::OnLeftButtonDown (MOUSEINFO *mi)
{
  left = true;
  disp();
};
void MouseRptParser::OnRightButtonUp  (MOUSEINFO *mi)
{
  x = 0;
  y = 0;
  right = false;
  disp();
};
void MouseRptParser::OnRightButtonDown  (MOUSEINFO *mi)
{
  right =true;
  disp();
};
void MouseRptParser::OnMiddleButtonUp (MOUSEINFO *mi)
{
  middle = false;
  disp();
};
void MouseRptParser::OnMiddleButtonDown (MOUSEINFO *mi)
{
  middle = true;
  disp();
};

USB     Usb;
USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_MOUSE>    HidMouse(&Usb);

MouseRptParser                               Prs;

void  dispSetup() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 start"));

  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(F("Mouse check"));
  display.display();
}

void disp(void) {
  display.clearDisplay();
  display.setCursor(0,0);             // Start at top-left corner
  display.println(String("left   : ") + (left ? "ON " : "OFF"));
  display.println(String("middle : ") + (middle ? "ON " : "OFF"));
  display.println(String("right  : ") + (right ? "ON " : "OFF"));
  display.println("dx = " + String(dx));
  display.println("dy = " + String(dy));
  display.println("x =" + String(x));
  display.println("y =" + String(y));
  display.display();
}

void setup() {
    dispSetup();

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay( 200 );
    
    HidMouse.SetReportParser(0, &Prs);
}

void loop()
{
  Usb.Task();
}
