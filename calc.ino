#define DEBUG 0

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include <Wire.h>
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

///////
// 文字列で表現した非負整数のBCD計算
///////

#define VAL(a) ((a) - '0')

// 先行する 0 を除去する
String iNorm(String a) {
  while(a.length() > 1) {
    if(a.charAt(0) != '0') {
      break;
    }
    a = a.substring(1);
  }
  return a;
}

// 値の大小比較
int iComp(String a, String b) {
  a = iNorm(a);
  b = iNorm(b);
  int sub = (int)a.length() - (int)b.length();
  if(sub != 0) {
    return sub;
  }
  return a.compareTo(b);
}

// 非負の整数の和を計算
String iAdd(String a, String b) {
  int len = (int)a.length() - (int)b.length();
  int i, sum, carry = 0;
  String c = "";
  
  if(len > 0) { // 文字列の長さを揃える
    for(i = 0; i < len; i++) {
      b = "0" + b;
    }
  }
  else if(len < 0) {
    for(i = 0; i < -len; i++) {
      a = "0" + a;
    }
  }
  len = a.length();
  for(i = len-1; i >= 0; i--) {
    int sum = VAL(a.charAt(i)) + VAL(b.charAt(i)) + carry;
    if(sum > 9) {
      sum -= 10;
      carry = 1; // 繰り上がり
    }
    else {
      carry = 0;
    }
    c = String(sum) + c;
  }
  if(carry)
    c = "1" + c;
  return iNorm(c);
}

// 差の計算 a >= b でなければならない
String iSub(String a, String b) {
  int len = (int)a.length() - (int)b.length();
  int i, sub, borrow = 0;
  String c = "";
  
  if(len > 0) { // 文字列の長さを揃える
    for(i = 0; i < len; i++) {
      b = "0" + b;
    }
  }
  else if(len < 0) {
    for(i = 0; i < -len; i++) {
      a = "0" + a;
    }
  }
  len = a.length();
  for(i = len-1; i >= 0; i--) {
    int sub = VAL(a.charAt(i)) - VAL(b.charAt(i)) - borrow;
    if(sub < 0) {
      sub += 10;
      borrow = 1; // 繰り下がり
    }
    else {
      borrow = 0;
    }
    c = String(sub) + c;
  }
  return iNorm(c);
}

// 積の計算
String iMul(String a, String b) {
  String c, amul;
  int i, j;

  c = "0";
  amul = a;
  for(j = b.length() - 1; j >= 0; j--) {
    for(i = 0; i < VAL(b.charAt(j)); i++) {
      c = iAdd(c, amul);
    }
    amul = amul + "0"; // 左シフト
  }
  return iNorm(c);
}

// 整数の除算
String rem; // 剰余を格納
String iDiv(String a, String b) {
  String mult;
  String x, ans = "0";
  int i, j;

  if(iComp(b, "0") == 0) {
    return "0";
  }
  rem = "";
  while(a.endsWith("0") && b.endsWith("0")) { // 計算時間とスタックの節約
    a = a.substring(0, a.length() - 1);
    b = b.substring(0, b.length() - 1);
    rem += "0";
  }
  x = b;
  for(i = 0; ; i++) {
    x = x + "0";
    if(iComp(a, x) < 0) {
      break;
    }
  }
  for(; i >= 0; i--) {
    if(iComp(a, "0") == 0) {
      break;
    }
    mult = "1";
    x = b;
    for(j = 0; j < i; j++) {
      mult = mult + "0";
      x = x + "0";
    }
    for(j = 0; j < 10; j++) {
      if(iComp(a, x) >= 0) {
        a = iSub(a, x);
        ans = iAdd(ans, mult);
      }
      else {
        break;
      }
    }
  }
  rem = iNorm(a + rem);
  return iNorm(ans);
}

// 剰余を読み出す関数 iDiv の実行後似呼び出し
String iRem() {
  return rem;
}

// 最大公約数の計算（互除法） a, b は 0 不可
String iGCD(String a, String b) {
  String c, r;

  if(iComp(a, b) < 0) {
    c = b;
    b = a;
    a = c;
  }
  for(;;) {
    iDiv(a, b);
    r = iRem();
    if(iComp(r, "0") == 0) {
      return b;
    }
    a = b;
    b = r;
  }
}
 
///////
// 分数計算部
///////

// 分数の構造体定義　分母分子の非負整数と符号で構成
struct FRAC {
  String u, b;
  int sign;
};

// 小数を表す文字列を分数に変換
struct FRAC str2FRAC(String s) {
  struct FRAC a;
  int ptPos, mul, i;

  if(s.length() == 0) {
    s = "0";
  }
  a.sign = 1;
  if(s.charAt(0) == '-') {
    a.sign = -1;
    s = s.substring(1);
  }
  ptPos = s.indexOf(".");
  if(ptPos >= 0) {
    int mul = (int)s.length() - ptPos - 1;
    a.u = s.substring(0, ptPos) + s.substring(ptPos + 1);
    if(a.u.length() == 0) {
      a.u = "0";
    }
    a.b = "1";
    for(i = 0; i < mul; i++) {
      a.b = a.b + "0";
    }
  }
  else {
    a.u = s;
    a.b = "1";
  }
  return a;
}

// 符号の反転
struct FRAC fSign(struct FRAC a) {
  a.sign = -a.sign;
  return a;
}

// 約分
struct FRAC fReduce(struct FRAC a) {
  String gcd;

  if(iComp(a.u, "0") == 0 || iComp(a.b, "0") == 0) {
    return a;
  }
  gcd = iGCD(a.u, a.b);
  if(iComp(gcd, "1") == 0) {
    return a;
  }
  a.u = iDiv(a.u, gcd);
  a.b = iDiv(a.b, gcd);
  return a;
}

// 和
struct FRAC fAdd(struct FRAC a, struct FRAC b) {
  struct FRAC c;

  c.b = iMul(a.b, b.b);
  a.u = iMul(a.u, b.b);
  b.u = iMul(b.u, a.b);
  if(a.sign == b.sign) {
    c.u = iAdd(a.u, b.u);
    c.sign = a.sign;
  }
  else {
    if(iComp(a.u, b.u) > 0) {
      c.u = iSub(a.u, b.u);
      c.sign = a.sign;
    }
    else {
      c.u = iSub(b.u, a.u);
      c.sign = b.sign;
    }
  }
  return fReduce(c);
}

// 差
struct FRAC fSub(struct FRAC a, struct FRAC b) {
  return fReduce(fAdd(a, fSign(b)));
}

// 積
struct FRAC fMul(struct FRAC a, struct FRAC b) {
  struct FRAC c;

  c.sign = a.sign * b.sign;
  c.u = iMul(a.u, b.u);
  if(iComp(c.u, "0") == 0) {
    c.b = 1;
    c.sign = 1;
    return c;
  }
  c.b = iMul(a.b, b.b);
  return fReduce(c);
}

// 商
struct FRAC fDiv(struct FRAC a, struct FRAC b) {
  struct FRAC c;

  c.sign = a.sign * b.sign;
  c.u = iMul(a.u, b.b);
  c.b = iMul(a.b, b.u);
  if(iComp(c.b, "0") == 0) {
    c.u = "0"; // NaN
  }
  return fReduce(c);
}

// 分数から float の値を求める（現在は未使用）
float fNum(struct FRAC a) {
  float u = 0, b = 0;
  int i;

  for(i = 0; i < a.u.length(); i++) {
    u = u * 10.0 + VAL(a.u.charAt(i));
  }
  for(i = 0; i < a.b.length(); i++) {
    b = b * 10.0 + VAL(a.b.charAt(i));
  }
  u /= b;
  u *= a.sign;
  return u;
}

///// 分数ライブラリ終わり

// 以下 RPN電卓プログラム

// スタック周り
#define STACK_DEPTH 8
struct FRAC stack[STACK_DEPTH];

void push(struct FRAC val) {
  for(int i = STACK_DEPTH-1; i > 0; i--) {
    stack[i] = stack[i-1];
  }
  stack[0] = val;
}

struct FRAC pop(void) {
  int i;
  struct FRAC val;

  val = stack[0];
  for(int i = 0; i < STACK_DEPTH-1; i++) {
    stack[i] = stack[i+1];
  }
  // スタックの底から 0 が出てくる方式
  stack[STACK_DEPTH-1] = str2FRAC("0");
  return val;
}

void initStack(void) {
  for(int i = 0; i < STACK_DEPTH; i++) {
    stack[i] = str2FRAC("0");
  }
}

void waitRelease(int timeout) { // キーを離すまで待つ
  unsigned long time = millis();
  while(ispressed()) {
    usbloop();
    if(millis() > time + timeout) {
      break; // タイムアウト
    }
  }
}

// キー入力をつなげて文字列にする
boolean entering = false; // 文字入力中フラグ
String ent; // 入力中の文字
void addNum(char key) {
  if(!entering) {
    ent = "";
    entering = true;
  }
  if(key == '.') {
    if(ent.indexOf(".") >= 0) return; // 小数点は１個まで
  }
  ent = ent + String(key);
}

// 入力文字を確定してスタックに入れる
void enter(void) {
  if(entering) {
    entering = false;
    push(str2FRAC(ent));
    ent = "";
  }
}

// 整数を桁数制限 limit 内で E 表示する
String dispE(String s, int limit) {
  if(s.length() <= limit) {
    return s;
  }
  limit -= 2; // "En" のスペース
  if((int)s.length() - 9 > limit) // ...E10 みたいに２桁になる
    limit--;
  s = s.substring(0, limit) + "E" + String((int)s.length() - limit);
  return s;
}

#define DIGIT_LIM 21 // 表示桁数制限（表示デバイスとフォントサイズ依存）

// 分数の表示用文字列生成
String FRAC2fstr(struct FRAC a) {
  int lim = DIGIT_LIM;
  int reduce = false;
  String s = "";

  if(iComp(a.b, "0") == 0) { // 分母が 0 は NaN と表示
    return "NaN";
  }
  if(a.sign == -1) {
    lim--; // 負号 '-' のスペースを確保しておく
  }
  int blimit = max((int)(lim / 2 - 1), lim - 1 - (int)a.u.length()); // 分母に約半分の文字数を確保
  s = "/" + dispE(a.b, blimit);
  lim -= (int)s.length(); // 分子に使用できる桁数
  s = dispE(a.u, lim) + s;

  if(a.sign < 0) { // 負号をつける
    s = "-" + s;
  }
  return s;
}

// 小数点以下の '0' や末尾の '.' を取り除く
String trimZero(String s) {
  if(s.indexOf(".") < 0) { // 小数点がない場合は処理しない
    return s;
  }
  while(s.length() > 1) {
    if(s.endsWith("0")) { // 小数点以下の '0' を削除
      s = s.substring(0, (int)s.length() - 1);
    }
    else if(s.endsWith(".")) { // 末尾の '' を削除
      s = s.substring(0, (int)s.length() - 1);
      break; // 小数点より前は処理しない
    }
    else {
      break;
    }
  }
  return s;
}

// 小数の表示用文字列生成関数　14桁に収めて表示
String FRAC2dstr(struct FRAC a) {
  int limit = DIGIT_LIM;
  String u = a.u, b = a.b, s;
  int slide = 0, lendiff, i, ptPos;

  if(iComp(b, "0") == 0) { // 分母が 0 は NaN と表示
    return "NaN";
  }
  if(iComp(u, "0") == 0) {
    return "0";
  }
  if(a.sign < 0) {
    limit--; // 負号 '-' のスペースを確保しておく
  }
  // n.xxxx の形式を前提とするためにまず桁数を揃える
  lendiff = (int)u.length() - (int)b.length();
  if(lendiff > 0) { // 分子が長い
     for(i = 0; i < lendiff; i++) {
        b = b + "0";
        slide++;
    }
  }
  else if(lendiff < 0) { //分母が長い
    for(i = 0; i < -lendiff; i++) {
      u = u + "0";
      slide--;
    }
  }
  if(iComp(u, b) < 0) { // 分子の値が小さいと 0.xxx になってしまうため
    u = u + "0"; // １桁ずらして n.xxxxの形式にする
    slide--;
  }

  // 各桁の数値を求める
  for(i = 0; i <  DIGIT_LIM - 1; i++) { // 分子を大きくする（整数除算のため）
    u = u + "0";
    slide--;
  }
  s = iDiv(u, b); // 分母分子を除算して数字の並びを求める
  ptPos = slide + (int)s.length();// 小数点の位置（文字列の先頭からの位置）
      
  // フォーマッティング
  if(0 < ptPos && ptPos < limit) { // 小数点が表示桁内に置ける場合
    u = s.substring(0, ptPos); // 小数点より前の文字列
    b = s.substring(ptPos); // 小数点より後ろの文字列
    s = u + "." + b;
    s = trimZero(s); // 小数点より後ろの 0 を取り除く
    s = s.substring(0, limit);
  }
  else if(ptPos > limit || ptPos < -limit/3) { // 絶対値が非常に大きい・小さい場合
    // x.xxxEyy の形式で表示する
    s = s.substring(0, 1) + "." + s.substring(1);
    s = trimZero(s);
    if(ptPos > 0) {
        s = s.substring(0, limit - 3); // "Eyy" の文字数を確保する
    }
    else {
        s = s.substring(0, limit - 4); // "E-yy" の文字数を確保する
    }
    s += "E" + String(ptPos - 1);
  }
  else if(ptPos == limit) { // ちょうど小数点がない場合
    s = s.substring(0, limit);
  }
  else { // 0.0000xxx の形式の場合
    for(i = 0; i < -ptPos; i++) {
        s = "0" + s;
    }
    s = "0." + s;
    s = trimZero(s);
    s = s.substring(0, limit);
  }
  if(a.sign < 0) s = "-" + s; // 負号をつける
  return s;
}

boolean fracmode = false; // 分数表示モードフラグ
String FRAC2str(struct FRAC a) { // 切り替え表示
  if(fracmode) {
    return FRAC2fstr(a);
  }
  else {
    return FRAC2dstr(a);
  }
}

// FRAC2dstr() が遅いので，演算後に表示文字列を作っておく
String dispStr[8];
void refreshStr(void) {
  int i;
  for(i = 0; i < 8; i++) {
    dispStr[i] = FRAC2str(stack[i]);
  }
}

// 数値表示
void draw(void) {
  String s;
  display.clearDisplay();
  int i;
  
  if(entering) { // 数値の入力中
    s = ent;
    if(s.length() > DIGIT_LIM) { // 長過ぎるとき，文字の上位桁を省略表示
      s = ".." + s.substring((int)s.length() - DIGIT_LIM + 2);
    }
    display.setCursor(2, 54);
    display.println(s);
    for(i = 0; i < 6; i++) {
      display.setCursor(128 - dispStr[i].length() * 6, (5 - i) * 9);
      display.println(dispStr[i]);
    }
  }
  else {
    for(i = 0; i < 7; i++) {
      display.setCursor(128 - dispStr[i].length() * 6, (6 - i) * 9);
      display.println(dispStr[i]);
    }
  }
  display.display();
}

// 長押しの時間 ミリ秒
#define LONGPRESS 500

// メインループ
void loop(void) {
  struct FRAC a;
  int longpress = false;

  usbloop();
  char key = keycheck();
  if(!key)
    return;
  if(!entering) { // 長押し検出するのは入力モード以外
    if(key == '=' || key == '-' || key == '*' || key == '/') {
        // 長押し処理をするボタン
      unsigned long time = millis();
      waitRelease(LONGPRESS);
      if(millis() > time + LONGPRESS) {
        longpress = true;
      }
    }
  }
  switch(key) {
  case '0':  
  case '1':  
  case '2':  
  case '3':  
  case '4':  
  case '5':  
  case '6':  
  case '7':  
  case '8':  
  case '9':  
  case '.':
    addNum(key); // 新しい数値入力
    break;
  case 'e': // ENTER キー
    if(!longpress) {
      if(entering) {
        enter();
      }
      else { // 確定済みのときは DUP（複製）
        a = pop();
        push(a);
        push(a);  
      }
      break;
    } // 長押しのときは DROP
  case 'b': // 確定してDROP（削除）
    if(entering) {
      ent = ent.substring(0, ent.length() - 1);
      if(ent.length() == 0) {
        entering = false;
      }
    }
    else {    
      pop();
    }
    break;
  case '+':
    enter();
    a = pop();
    stack[0] = fAdd(stack[0], a);
    break;
  case '-': // 引き算または負号反転
    enter();
    if(!longpress) {
      a = pop();
      stack[0] = fSub(stack[0], a);
      break;
    } // 長押しのときは符号反転
  case 'C': // 符号反転
    enter();
    stack[0] = fSign(stack[0]);
    break;
  case '*': // 掛け算または表示切り替え
    enter();
    if(longpress) { // 長押し時は表示切り替え
      fracmode = !fracmode;
    }
    else {
      a = pop();
      stack[0] = fMul(stack[0], a);
    }
    break;
  case '/': // 割り算または SWAP
    enter();
    if(!longpress) {
      a = pop();
      stack[0] = fDiv(stack[0], a);
      break;
    } // 長押しのときは SWAP
  case '%': // SWAP
    enter();
    a = stack[0];
    stack[0] = stack[1];
    stack[1] = a;
    break;
  case 'S': // 平方根（未実装）
//    stack[0] = sqrt(stack[0]);
    enter();
    break;
  case 't': // 小数表示モード
    fracmode = !fracmode;
    break;
  }
  // 小数値を再計算
  if(!entering) {
    refreshStr();
  }
  draw();
}

/********* USB and display ***********/

#include <hidboot.h>
#include <usbhub.h>


String queue = "";
int pressed = false;

int ispressed(void) {
  return pressed;
}

void startScreen(void) {
  display.clearDisplay();
  display.display();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(20,24);             // Start at top-left corner
  display.println(F("exactRPN"));
  display.display();

  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setTextSize(1);             // Normal 1:1 pixel scale
}

void  dispSetup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 start"));

  startScreen();    // Draw 'stylized' characters
}

class KbdRptParser : public KeyboardReportParser
{
  protected:

    void OnKeyDown  (uint8_t mod, uint8_t key);
    void OnKeyUp  (uint8_t mod, uint8_t key);
};

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  char c;
  switch(key) {
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
    case 94:
    case 95:
    case 96:
    case 97:
      c = '1' + key - 89;
      break;
    case 98:
      c = '0';      
      break;
    case 99:
      c = '.';
      break;
    case 43: // tab
      c = 't';
      break;
    case 84:
      c = '/';
      break;
    case 85:
      c = '*';
      break;
    case 86:
      c = '-';
      break;
    case 87:
      c = '+';
      break;
    case 88: // enter
      c = 'e';
      break;      
    case 42: // backspace
      c = 'b';
      break;      
  }
  queue += String(c);
  pressed = true;
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{
  pressed = false;
}

char keycheck(void) {
  if(queue.length() == 0) {
    return 0;
  }
  char c = queue.charAt(0);
  queue = queue.substring(1);
  return c;
}

USB     Usb;
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

KbdRptParser Prs;

void setup()
{
  Serial.begin( 115200 );
  dispSetup();

  Serial.println("Start");
  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay( 200 );

  HidKeyboard.SetReportParser(0, &Prs);

  initStack();
  refreshStr();
}

void usbloop(void)
{
  Usb.Task();
}
