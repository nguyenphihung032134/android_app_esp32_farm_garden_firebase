#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <FirebaseESP32.h>

#include <EEPROM.h>
#include <Adafruit_GFX.h>       // include Adafruit graphics library
#include <Adafruit_ILI9341.h>   // include Adafruit ILI9341 TFT library
//#include <Fonts/FreeSerif9pt7b.h> // phông chữ 
#include <EEPROM.h>
#include "DHT.h"
#include "RTClib.h"
#include <Wire.h>
// thư viện để lấy thời gian và chuyển đổi ra ngày tháng từ dữ liệu trên google 
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>
// thư viện để truy cập linh cho chức năng google sheet
#include <HTTPClient.h>
// thư viện tạo tiếng còi 
#include <Tone32.h>

#include "web.h"
#include "image1.h"
#include "image2.h"
#include "image3.h"
#include "image4.h"
#include "icon.h"
#include "iconthietbi.h"

// Thiết lập tên và mật khẩu mạng Wi-Fi để phát
const char* ssid = "DOANHUNG123";
const char* pass = "xinchaoban";

// thiet lập firebase
#define FIREBASE_HOST "https://doanvuoncay-578e7-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "GI2XBkB8tpGbb83cicFfg0Z5eYIInwZbwu0jgTem"

//tao doi tg tên là firebaseData để lưu các giá trị đọc và ghi lên firebase
FirebaseData firebaseData ;
FirebaseJson json;

//
//thiet lập cho lcd 
#define TFT_CS    4     // TFT CS  pin is connected to NodeMCU pin D0
#define TFT_RST   2     // TFT RST pin is connected to NodeMCU pin D3
#define TFT_DC    15     // TFT DC  pin is connected to NodeMCU pin D4
// initialize ILI9341 TFT library with hardware SPI module
// SCK (CLK) ---> NodeMCU pin D18 (GPIO18)
// MOSI(DIN) ---> NodeMCU pin D23 (GPIO23)
// MISO() ---> NodeMCU pin D19 (GPIO19)
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
// khai báo mã màu 
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define BROWN   0x79E0
////
#define DHTPIN 32 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

RTC_DS1307 DS1307_RTC;

const long muigio = 3600*7; // GMT+7
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org",muigio);

// độ ẩm đất ;
int land=0 ,real_land=0 , land1 =0;

//nhiệt độ , độ ẩm không khí 
float t =0 ;
float h = 0;
//nhiệt độ trước 
float pretemp =0;
float preland =0;
// nhiệt độ sau
float currtemp =50;
float currland =100;
//biến trung gian để vẽ biểu đồ 
float i=0;
float j=0;
//biến thời gian 
int gio , phut , giay , gio1 , phut1 ,giay1 ,ngay ,thang ,nam , ngay1 , thang1 ,nam1;

//cam bien lưu lượng nước 
#define  sensorInterrupt 34 // 0 = digital pin 2
#define  sensorh20  34       //--------------------------------------------------chân tín hiệu

float hieuchuan = 4.5;

volatile byte xung=0;

float tocdo=0 , luuluong=0,sudung=0;
int tocdo1=0;
unsigned long oldTime=1000;
// BIẾN DÙNG CHO THỜI GIAN MILLI
unsigned long tgrelay=4000 , st=60000 , kiemtra1=60000;

// đường dẫn đến trang tính google sheet 
String GOOGLE_SCRIPT_ID = "AKfycbxX_DeXl8CGXi2MUsW1yKji811bNyulh8bkXpHJF_57hR1dc5OgaMUjzVRUr2deL-s";

// biến cho các chân relay coi và nút nhấn 
#define  relayquat  26
#define  relaybom  25
#define  ttbom  36
#define  ttquat 39 
#define  mucnuoc 33

#define BUZZER_PIN 12
#define BUZZER_CHANNEL 0
#define  buton      13
// các biến firebase 
byte trangthaithucong , butonbom , butonquat;
byte trangthaiauto , doamonbom , doamofbom ,nhietdoonquat,nhietdoofquat;
byte trangthaihengio ,hengiobom , hengioquat ;
byte gioofbom , phutofbom , gioonbom , phutonbom , gioofquat , phutofquat , gioonquat , phutonquat;

// biến cho eeprom
int i1;
//

// ham setup ////

void setup() {
  // set chân cam biến lưu lượng nước 
  pinMode(sensorh20,  INPUT);
  pinMode(relayquat, OUTPUT); // chân relay1
  pinMode(relaybom , OUTPUT); // chân relay2
  pinMode(buton,      INPUT); // chân button 1
  pinMode(ttbom   ,   INPUT); // chân button 1
  pinMode(ttquat   ,  INPUT); // chân button 1
  pinMode(mucnuoc   , INPUT); // chân kiểm tra nước còn không 


  digitalWrite(sensorh20, HIGH);
  digitalWrite(relaybom , LOW);
  digitalWrite(relayquat, LOW);

  // khai báo ngắt cho chức năng đo lưu lượng nước
  attachInterrupt( sensorInterrupt, pulseCounter, FALLING);
  
  
  // khởi  động cổng serial 
  Serial.begin(115200);
  // khởi động ham eeprom
  EEPROM.begin(512);
  // khởi động dht11
  dht.begin();
  // khởi động ds1307
  DS1307_RTC.begin();
  // khởi động lcd ili9341
  tft.begin();

  tft.setRotation(3);  // set chiều hiển thị cho lcd 
  //tft.setFont(&FreeSerif9pt7b);  // chọn phông chữ
 
  tft.drawRGBBitmap(0, 0,wif, 320 ,240);
  hieuungcoi();
 
  

  // lấy wifi trong eeprom
 
  for(i1=0;i1<30;i1++){if(EEPROM.read(i1)==0)break;tenwf1[i1]=EEPROM.read(i1);}
  for(i1=30;i1<60;i1++){if(EEPROM.read(i1)==0)break;passwf1[i1-30]=EEPROM.read(i1);}

  //hiển thị wifi kết nối tới 
  tft.setCursor(10, 190);
  tft.setTextColor(ILI9341_RED);  tft.setTextSize(2);
  tft.print("SSID: ");
  tft.println(tenwf1);
  tft.setCursor(10, 220);
  tft.setTextColor(ILI9341_RED); tft.setTextSize(2);
  tft.print("PASS: ");
  tft.println(passwf1);

  // Cập nhật tên và mật khẩu mạng Wi-Fi trong ESP8266
  WiFi.disconnect(); /// ngat wifi hiện tại 
  WiFi.begin(tenwf1, passwf1); // kết nối đến wifi
  delay(4000);// Chờ cho đến khi kết nối được thiết lập
 
  //neu ket noi wifi thanh cong và không thanh công 
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
      
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(45, 30);tft.setTextColor(ILI9341_GREEN); tft.setTextSize(2); 
      tft.print("KET NOI THANH CONG ");
      tft.drawBitmap(125, 100, wifion, 70, 70, ILI9341_GREEN);

       // ket noi firebase
      Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
      Firebase.reconnectWiFi(true);

      //lấy thời gian và dữ liệu ngày trên web 
      timeClient.begin();
      timeClient.update();
      gio =timeClient.getHours();
      phut =timeClient.getMinutes();
      giay =timeClient.getSeconds();

      //lay gia tri ngay thang nam duoi dang  Epoch time,và dung thu vien time để chuyển 
      time_t epochTime = timeClient.getEpochTime();
      struct tm * timeinfo;
      timeinfo = localtime(&epochTime);
      nam=timeinfo->tm_year + 1900;
      thang=timeinfo->tm_mon + 1;
      ngay=timeinfo->tm_mday;
      DS1307_RTC.adjust(DateTime(nam, thang, ngay, gio, phut, giay));
      tone(BUZZER_PIN,5,1000, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      //
      delay(3000);
     
  }else{  // neu kết nối wif thất bại 
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(50, 30);tft.setTextColor(ILI9341_RED);  tft.setTextSize(2);
      tft.print("KET NOI THAT BAI ");
      tft.drawBitmap(125, 100, wifiof, 70, 70, ILI9341_RED);
      tone(BUZZER_PIN,1,1000, BUZZER_CHANNEL);
      noTone(BUZZER_PIN, BUZZER_CHANNEL);
      delay(3000);
  }

   // Thiết lập các đường dẫn của trang web
  server.on("/", handleRoot);
  server.on("/connect", handleConnect);

  // Bắt đầu phục vụ trang web
  server.begin();

    // xoa man hình và hiện biểu tượng máy bơm và quạt 
    tft.fillScreen(ILI9341_BLACK);
    mocgiatri();
    tft.drawRGBBitmap(1, 1,fan, 40 ,40);
    tft.drawRGBBitmap(81, 1,pump, 40 ,40);
    
    
  

}
// chương trinhg chính ///////////////////////////////////////////////
void loop() {
   
    h = dht.readHumidity();
    t = dht.readTemperature();
    if(t>50){t = 50;}
    if(t<0) {t = 0 ;}
    bieudodht11();
    bieudo();
    delay(50);
    thoigian();
    ttbomquat();
////////////////////////////////////////////////////////////////////////////////////
    // nếu kết nối wifi thành công 
  if (WiFi.waitForConnectResult() == WL_CONNECTED){
    ghifirebase();
    chedothucong();
    chedotudong();
    chedohengio();
    //in dữ liệu lên google sheet
     if ((unsigned long)(millis()-st) >60000){
      st = millis();
      print_speed();
    }
    //
    if(trangthaiauto==0 && trangthaihengio==0 && trangthaithucong==0){
      digitalWrite(relayquat , LOW);
      digitalWrite(relaybom  , LOW);
    }
    delay(50);
  }
////////////////////////////////////////////////////////////////////////////////////
  
nutnhanwifi();
//
kiemtramucnuoc();
kiemtraongdan();

}
//

////////////////////////////////ham nút nhấn set up wif kết nối tới /////////////////////////////////
void nutnhanwifi(){
 //nut nhấn kích hoạt chế độ thiết lập wif cho thiết
if(digitalRead(buton)== LOW){
  
if ( (unsigned long)(millis()- tgrelay) > 2000){ // nếu nhấn nút trong vòng 6s
   tgrelay = millis();
   hieuungcoi1();
   
   // xóa màn hình hiển thị chữ 
   tft.fillScreen(ILI9341_BLACK);tft.drawRGBBitmap(0, 0,wifi2, 320 ,240);
   
   //Phát  Wi-Fi
  WiFi.mode(WIFI_AP);
  WiFi.softAPdisconnect(true);  // ngắt hết kết nối các thiết bị ở chế độ phát wifii
  delay(500);
  WiFi.softAP(ssid , pass);
  
   while(true){  // vòng lặp

     server.handleClient(); // ham duy tri web

     if(digitalRead(buton)== LOW){ // nut nhan thoát che do cai dat wif
        hieuungcoi1();
        WiFi.mode(WIFI_OFF);// tắt chế độ phát wifi
        // lấy wifi trong eeprom
        for(i1=0;i1<30;i1++){if(EEPROM.read(i1)==0)break;tenwf1[i1]=EEPROM.read(i1);}
        for(i1=30;i1<60;i1++){if(EEPROM.read(i1)==0)break;passwf1[i1-30]=EEPROM.read(i1);}
        // Cập nhật tên và mật khẩu mạng Wi-Fi trong ESP8266
        WiFi.disconnect(); /// ngat wifi hiện tại 
        WiFi.begin(tenwf1, passwf1); // kết nối đến wifi
        delay(2000);// Chờ cho đến khi kết nối được thiết lập
        if (WiFi.waitForConnectResult() == WL_CONNECTED){tone(BUZZER_PIN,5,1000, BUZZER_CHANNEL); noTone(BUZZER_PIN, BUZZER_CHANNEL); }
        // khôi phục màn hình hiển thị 
         tft.fillScreen(ILI9341_BLACK);
         mocgiatri();
         tft.drawRGBBitmap(1, 1,fan, 40 ,40);
         tft.drawRGBBitmap(81, 1,pump, 40 ,40);
         pretemp =0; preland =0; currland =100;currtemp =50;
        break;}
   }    
}}
}

/////////////////////////////ham cảnh báo hết nước trong bể chứa ///////////////////////////////////////////////
void kiemtramucnuoc(){
  if (digitalRead(ttbom)== HIGH){
   if(digitalRead(mucnuoc)== HIGH){
     tft.fillScreen(ILI9341_BLACK);
     tft.drawRGBBitmap(0, 0,bechua, 320 ,240);
     tft.setCursor(200, 210);tft.setTextColor(RED);  tft.setTextSize(2);
     tft.print("HET NUOC ");
    if(WiFi.waitForConnectResult() == WL_CONNECTED){Firebase.setFloat(firebaseData, "canhbao/bechua",0);} // in lên firebase trạng thái 
     while(true){ // khi hết nước phát ra cảnh báo và tắt relay bơm 
           
           tone(BUZZER_PIN,5,1000, BUZZER_CHANNEL);
           noTone(BUZZER_PIN, BUZZER_CHANNEL);
           digitalWrite(relaybom     , LOW);
           if(WiFi.waitForConnectResult() == WL_CONNECTED){Firebase.setInt(firebaseData, "trangthaithietbi/bom",0);}
           if(digitalRead(mucnuoc)== LOW || digitalRead(buton)== LOW){  /// nếu bể chứa có nước trở lại thì thoát khỏi vòng lặp cảnh báo 
             // in lên firebase trạng thái 
            if(WiFi.waitForConnectResult() == WL_CONNECTED){Firebase.setFloat(firebaseData, "canhbao/bechua",1);} 
              
              // khôi phục màn hình hiển thị 
           tft.fillScreen(ILI9341_BLACK);
           mocgiatri();
           tft.drawRGBBitmap(1, 1,fan, 40 ,40);
           tft.drawRGBBitmap(81, 1,pump, 40 ,40);
           pretemp =0; preland =0; currland =100;currtemp =50;
           break;}
     }
   }
   // KHI CHƯA hết nước thì in lên firebase trạng thái 
   if(digitalRead(mucnuoc)== LOW){
     if(WiFi.waitForConnectResult() == WL_CONNECTED){Firebase.setFloat(firebaseData, "canhbao/bechua",1);}
   }
}}

/////////////////////////// hàm kiểm tra xem hệ thống dẫn nước /////////////
void kiemtraongdan(){

   if (digitalRead(ttbom)== HIGH){

     if((unsigned long)(millis()- kiemtra1 )>60000 ){
       kiemtra1=millis();
       if (tocdo<=0){
         if(WiFi.waitForConnectResult() == WL_CONNECTED){Firebase.setFloat(firebaseData, "canhbao/ongdan",0);}
         tft.fillScreen(ILI9341_BLACK);
         tft.drawRGBBitmap(0, 0,ongdan, 320 ,240);
         tft.setCursor(70, 120);tft.setTextColor(RED);  tft.setTextSize(2);
         tft.print("KIEM TRA ONG DAN");
         while(true ){
           digitalWrite(relaybom , LOW);
           if(WiFi.waitForConnectResult() == WL_CONNECTED){Firebase.setInt(firebaseData, "trangthaithietbi/bom",0);}
           tone(BUZZER_PIN,5,1000, BUZZER_CHANNEL);
           noTone(BUZZER_PIN, BUZZER_CHANNEL);
           // nếu đường ống k bị dỉ nữa thì thoát vòng lặp 
           redcambiennuoc();
           if(tocdo>0.2 || digitalRead(buton)== LOW ){
             if(WiFi.waitForConnectResult() == WL_CONNECTED){Firebase.setFloat(firebaseData, "canhbao/ongdan",1);}
              // khôi phục màn hình hiển thị 
           tft.fillScreen(ILI9341_BLACK);
           mocgiatri();
           tft.drawRGBBitmap(1, 1,fan, 40 ,40);
           tft.drawRGBBitmap(81, 1,pump, 40 ,40);
           pretemp =0; preland =0; currland =100;currtemp =50;
           break;}
           }
         } 
          else {
            if(WiFi.waitForConnectResult() == WL_CONNECTED)
            {Firebase.setFloat(firebaseData, "canhbao/ongdan",1);}
            } // nếu 
       }
     }
    //
   }


// ham đoc du lieu firebase////////////////////////////
void ghifirebase(){

 // lưu giá trị nhiệt độ ,độ ẩm, do am đat , toc độ nước , lượng nước đã sử dụng   lên firebase
 Firebase.setFloat(firebaseData, "cambien/nhietdo",t); // nhiet độ môi trường 
 Firebase.setFloat(firebaseData, "cambien/doam",h);  // độ ẩm không khí 
 Firebase.setFloat(firebaseData, "cambien/doamdat",land); // độ ẩm đất 
 Firebase.setFloat(firebaseData, "cambien/tocdonuoc",tocdo); // tốc độ dòng chảy 
 Firebase.setFloat(firebaseData, "cambien/tongluongnuoc",sudung/1000);// lượng nước đã sử dụng 
}

//ham che do thu cong //////////////////////////////////
void chedothucong(){

 // đọc dữ liệu chế độ thủ công trên điện thoại 
 Firebase.getInt(firebaseData, "che do thu cong/trang thai");
  trangthaithucong = firebaseData.floatData();
  
 //nếu sử dụng chế độ thủ công trên điện thoại 
 if (trangthaithucong==1){
    // đọc trạng thái buton bơm và quạt 
 Firebase.getInt(firebaseData, "che do thu cong/bom");
  butonbom = firebaseData.floatData();
 Firebase.getInt(firebaseData, "che do thu cong/quat");
  butonquat = firebaseData.floatData();

  if(butonbom==1){digitalWrite(relaybom , HIGH);}
  else { digitalWrite(relaybom     , LOW);}
  
  if(butonquat==1){digitalWrite(relayquat, HIGH);}
  else{digitalWrite(relayquat, LOW);}
  }
    
}

// che độ  tự động///////////////////////////////////////////////////////////
void chedotudong(){

// đọc dữ liệu chế độ tự động trên điện thoại 
 Firebase.getInt(firebaseData, "che do tu dong/trang thai");
  trangthaiauto = firebaseData.floatData();
  // dữ liệu cài đặt giá trị cảm biến bật tắt thiết bị 
  if(trangthaiauto==1){
    // bom 
 Firebase.getInt(firebaseData, "giatricaidatcambien/bom/doamofbom");
  doamofbom = firebaseData.floatData();
 Firebase.getInt(firebaseData, "giatricaidatcambien/bom/doamonbom");
  doamonbom = firebaseData.floatData();
 //quat
 Firebase.getInt(firebaseData, "giatricaidatcambien/quat/nhietdoofquat");
  nhietdoofquat = firebaseData.floatData();
 Firebase.getInt(firebaseData, "giatricaidatcambien/quat/nhietdoonquat");
  nhietdoonquat = firebaseData.floatData();
  Serial.println(t);
  Serial.println(land);
  Serial.println("---------");
  //quat
 if( t >= nhietdoonquat) { digitalWrite(relayquat , HIGH);}
 else if(t<=nhietdoofquat){digitalWrite(relayquat , LOW);} 

 //bom
 if( land >= doamofbom) { digitalWrite(relaybom , LOW);}
 else if(land<= doamonbom){digitalWrite(relaybom , HIGH);} 
   
   }
  
} 

// che do hẹn giờ //////////////////////////////////////////////////////////////
void chedohengio(){

// dữ liệu chế độ hẹn giờ trên điện thoại 
 Firebase.getInt(firebaseData, "che do hen gio/trang thai");
  trangthaihengio = firebaseData.floatData();

  if(trangthaihengio==1){
    // đọc trạng thái buton bơm và quạt 
 Firebase.getInt(firebaseData, "che do hen gio/bom");
  hengiobom = firebaseData.floatData();
 Firebase.getInt(firebaseData, "che do hen gio/quat");
  hengioquat = firebaseData.floatData();
  // dữ liệu cài  đặt thời gian cho chế độ hẹn giò  
////////////////////////////
  if(hengiobom==1){
    // bom off
    Firebase.getInt(firebaseData, "caidatthoigian/bom of/gio of bom");
  gioofbom = firebaseData.floatData();
 Firebase.getInt(firebaseData, "caidatthoigian/bom of/phut of bom");
  phutofbom = firebaseData.floatData();
 // bom on 
 Firebase.getInt(firebaseData, "caidatthoigian/bom on/gio on bom");
  gioonbom = firebaseData.floatData();
 Firebase.getInt(firebaseData, "caidatthoigian/bom on/phut on bom");
  phutonbom = firebaseData.floatData();

  // nếu thời gian ds1307 bằng với dữ liệu thì bật tat thiết bị 
  //bom
   if ( gio1==gioonbom && phut1==phutonbom){
     digitalWrite(relaybom , HIGH);
   }
   else if (gio1==gioofbom && phut1==phutofbom){
     digitalWrite(relaybom , LOW);
   }}
////////////////////////////////
 if(hengioquat==1){
   // quat off 
 Firebase.getInt(firebaseData, "caidatthoigian/quat of/gio of quat");
  gioofquat = firebaseData.floatData();
 Firebase.getInt(firebaseData, "caidatthoigian/quat of/phut of quat");
  phutofquat = firebaseData.floatData();
 // quat on 
 Firebase.getInt(firebaseData, "caidatthoigian/quat on/gio on quat");
  gioonquat = firebaseData.floatData();
 Firebase.getInt(firebaseData, "caidatthoigian/quat on/phut on quat");
  phutonquat = firebaseData.floatData();

  // nếu thời gian ds1307 bằng với dữ liệu thì bật tat thiết bị 
  //quat
   if ( gio1==gioonquat && phut1==phutonquat){
     digitalWrite(relayquat , HIGH);
   }
   else if (gio1==gioofquat && phut1==phutofquat){
     digitalWrite(relayquat , LOW);
   }}
 
}} 


////hiển thị các moc gia tri và kẻ đường biên ///////////////
void mocgiatri(){
  // hiển thi các mốc giới hạn nhiệt độ , độ ẩm 
  //tepm 
    tft.fillScreen(BLACK);
    tft.setTextColor(GREEN);tft.setTextSize(2);
    tft.setCursor(60, 170);
    tft.print("0");
    tft.setCursor(80, 170);
    tft.print("50");
    tft.setCursor(55, 110);
    tft.print("TEMP");

    //humi đất
    tft.setTextColor(MAGENTA);tft.setTextSize(2);
    tft.setCursor(220, 170);
    tft.print("0");
    tft.setCursor(240, 170);
    tft.print("99");
    tft.setCursor(215, 110);
    tft.print("Land");

    //humi không khí 
    tft.setTextColor(YELLOW);tft.setTextSize(2);
    tft.setCursor(5, 200);
    tft.print("0");
    tft.setCursor(135, 200);
    tft.print("99");
    tft.setCursor(10, 220);
    tft.print("Humi:");

    //lưu lượng nước
    tft.setTextColor(CYAN);tft.setTextSize(2);
    tft.setCursor(165, 200);
    tft.print("0");
    tft.setCursor(295, 200);
    tft.print("99");
    tft.setCursor(170, 220);
    tft.print("H20:");

    //tổng lượng nước đã sử dụng 
    tft.setCursor(5,43);
    tft.print("SUM:");




    //tft.drawLine(0  , 191,320 ,191, WHITE);
    tft.drawLine(0  , 60 ,320 ,60 , BLUE);
    tft.drawLine(160, 0  ,160 ,240, BLUE);
    tft.drawLine(80 , 0  ,80  ,41 , BLUE);
    tft.drawLine(0  , 41 ,160 ,41 , BLUE);
    
    tft.drawLine(170, 30 ,310 ,30 , YELLOW);
  
   }


   // biểu đồ nhiệt độ và độ ẩm đất ////////////////////////////////////
 void bieudodht11(){
    // DỌC GIA TRI CAM BIEN DHT11
    t = dht.readTemperature();
    if(t>50){t = 50;}
    if(t<0) {t = 0 ;}
    //dọc giá trị cảm biến độ ẩm đất 10 lần và lấy giá trị trung bình 
    
     for(int i=0;i<=9;i++){
      real_land +=analogRead(35);
     }
     land1 = real_land/10;
     land = map(land1, 0,  3700, 99, 5);if(land<=5){land=5;} 
    
   //hiển thị giá trị nhiệt độ không khí 
    tft.setCursor(45, 135); tft.setTextColor(WHITE,BLACK);tft.setTextSize(2);
    tft.printf("%2.1f",t);tft.print("'C");
    // hiển thị giá trị độ ẩm đất 
    tft.setCursor(225, 135); tft.setTextColor(WHITE,BLACK);tft.setTextSize(2);
    tft.printf("%2d",land);tft.print("%");
    real_land=0;
    
    // vẽ đường hiển thị temp
    i=map(pretemp,0,50,0,300);
    j=map(currtemp,0,50,0,300);


   for (i; i<=j; i=i+0.1)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 50;
    float y1 = 130+sin(angle) * 50;
    float x2= 80 + cos(angle) * 65;
    float y2 = 130+sin(angle) * 65;
    tft.drawLine(x1, y1,x2,y2,GREEN);
    }

  for (i-2; i>=j; i=i-0.05)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 50;
    float y1 = 130+sin(angle) * 50;
    float x2= 80 + cos(angle) * 65;
    float y2 = 130+sin(angle) * 65;
    tft.drawLine(x1, y1,x2,y2, BROWN);
   
    }
    // VẼ ĐƯỜNG HUMI
    i=map(preland,0,100,0,300);
    j=map(currland,0,100,0,300);   
  for (i; i<=j; i=i+0.1)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 50;
    float y1 = 130+sin(angle) * 50;
    float x2= 80 + cos(angle) * 65;
    float y2 = 130+sin(angle) * 65;
    
    tft.drawLine(x1+160, y1,x2+160,y2, MAGENTA);
    }

   i=map(preland,0,100,0,300);
   j=map(currland,0,100,0,300);  
   for (i-2; i>=j; i=i-0.05)
    {
    float j=i-150 ;
    float angle = (j / 57.2958)  - 1.57; 
    float x1= 80 + cos(angle) * 50;
    float y1 = 130+sin(angle) * 50;
    float x2= 80 + cos(angle) * 65;
    float y2 = 130+sin(angle) * 65;
    
    tft.drawLine(x1+160, y1,x2+160,y2, BROWN);
    }
    pretemp=currtemp;
    preland=currland;

    currtemp=t;
    currland=land;
    
 }


 ///BIỂU ĐỒ LƯU LƯỢNG NƯỚC VÀ ĐỘ ẨM KHÔNG KHÍ //////////////////////////////////
 void bieudo(){
   
   h = dht.readHumidity(); // lây gia tri độ  ẩm không khí từ cảm biến 
   redcambiennuoc();

  //humi không khí và hiển thị lên màn hình 
    tft.setTextColor(YELLOW , BLACK);tft.setTextSize(2);
    tft.setCursor(75, 220);
    tft.printf("%2.1f",h);tft.print("%");

    // hiển thị tổng lượng nước đã sử dụng ;
    tft.setTextColor(YELLOW , BLACK);tft.setTextSize(2);
    tft.setCursor(55, 43);
    tft.print(sudung/1000);tft.print("L");

    // hiển thị tốc độ nước 
    tft.setTextColor(CYAN , BLACK);tft.setTextSize(2);
    tft.setCursor(220, 220);
    tft.printf("%3.1f",tocdo);tft.print("L/M");

   // vẽ nền của biểu đồ hiển thị độ ẩm đất
   tft.fillRect(30,200,100,15, BROWN);
   tft.fillRect(190,200,100,15,BROWN);

  // vẽ biểu đồ hiển thị độ ẩm đất 
   tft.fillRect(30,200, h ,15,YELLOW);
   tft.fillRect(190,200, tocdo1,15,CYAN);
  
 }

/// ham hiển thị thời gian từ ds1307///////////////////////////////////////
 void thoigian(){
   
      DateTime now = DS1307_RTC.now();
      gio1=now.hour();
      phut1=now.minute();
      giay1=now.second();
      //
      nam1=now.year();
      thang1=now.month();
      ngay1=now.day();
   // vẽ ô nền và hiển thị thời gian 
   //tft.fillRect(0, 201,320,30,BLACK);
   tft.setCursor(180, 35);tft.setTextSize(2);tft.setTextColor(CYAN,BLACK);
   tft.printf("%2d/%2d/%4d",ngay1,thang1,nam1);

   tft.setTextSize(3);tft.setTextColor(YELLOW,BLACK);
   tft.setCursor(200, 5);
   tft.printf("%2d:%2d",gio1,phut1);
   
 }

 // đọc giá tri cam bien lưu lương nươc///////////////////////////////////////////// 
 void redcambiennuoc(){

   if ((unsigned long)(millis() - oldTime) > 1000) // Only process counters once per second
    {
        detachInterrupt(sensorInterrupt);
        tocdo = ((1000.0 / (millis() - oldTime)) * xung) / hieuchuan;
        tocdo1= int(tocdo);
        oldTime = millis();
        luuluong = (tocdo / 60) * 1000;
        sudung += luuluong;
        // Reset xung về 0
        xung = 0;
        // kich hoat lại ngat
        attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
    }
 }

 // ham ngat de doc cam bien do luu luong nước 
 ICACHE_RAM_ATTR void  pulseCounter()
{
    // Increment the pulse counter
    xung++;
}

// ham lam việc với google sheet /////////////////////////////////////////
void print_speed(){

  if (WiFi.status() == WL_CONNECTED)
  {
    

    String param;
    param  = "nhietdo="+String(t);
    param += "&doam="+String(land);
    param += "&tocdo="+String(sudung/1000);
    Serial.println(param);
    write_to_google_sheet(param);
  }
  

}

// ham lam việc với gooogle sheet ////////////////////////////////////////////
void write_to_google_sheet(String params) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
   //Serial.print(url);
    Serial.println("du lieu da duoc luu len he thong ");
    //---------------------------------------------------------------------
    //starts posting data to google sheet
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();  
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);     
    }
    //---------------------------------------------------------------------
    http.end();
}
// HÀM HIỂN THỊ TRẠNG THÁI ON OFF BƠM VÀ QUAT //////////////////////////////////////
void ttbomquat(){
  //nếu bơm đang tắt 
if (digitalRead(ttquat)== LOW){
   tft.setCursor(45, 10);tft.setTextColor(RED,BLACK);  tft.setTextSize(2);
   tft.print("OFF");
   //in lên firebase thông tin về trạng thái thiết bị 
   if (WiFi.status() == WL_CONNECTED)
  {Firebase.setInt(firebaseData, "trangthaithietbi/quat",0);} // trạng thái tắt  
   
// nếu bơm đang bật 
}if(digitalRead(ttquat)== HIGH){
   tft.setCursor(45, 10);tft.setTextColor(GREEN,BLACK);  tft.setTextSize(2);
   tft.print("ON ");
    //in lên firebase thông tin về trạng thái thiết bị 
   if (WiFi.status() == WL_CONNECTED)
  {Firebase.setInt(firebaseData, "trangthaithietbi/quat",1);} // trạng thái bật   
}
//nếu quạt đang tắt 
if (digitalRead(ttbom)== LOW){
   tft.setCursor(125, 10);tft.setTextColor(RED,BLACK);  tft.setTextSize(2);
   tft.print("OFF");
    //in lên firebase thông tin về trạng thái thiết bị 
   if (WiFi.status() == WL_CONNECTED)
  {Firebase.setInt(firebaseData, "trangthaithietbi/bom",0);} // trạng thái tắt 
  // nếu quạt đang bật  
}if(digitalRead(ttbom)== HIGH){
   tft.setCursor(125, 10);tft.setTextColor(GREEN,BLACK);  tft.setTextSize(2);
   tft.print("ON ");
    //in lên firebase thông tin về trạng thái thiết bị 
   if (WiFi.status() == WL_CONNECTED)
  {Firebase.setInt(firebaseData, "trangthaithietbi/bom",1);} // trạng thái bật  
}
////////////////////
}
//// ham tạo hiệu ứng còi kêu //////////////////////////////////////////////
void hieuungcoi(){

  tone(BUZZER_PIN, NOTE_A4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_G5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_G5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4,200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_C5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_C5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_C5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_D5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_C5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);

  //
  
} 
// hieuj ứng nhạc 2 ///////////////////////////////////////////////
void hieuungcoi1 (){
 tone(BUZZER_PIN, NOTE_E4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_B4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_C5,200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_A4, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_F5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_D5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  tone(BUZZER_PIN, NOTE_E5, 200, BUZZER_CHANNEL);
  noTone(BUZZER_PIN, BUZZER_CHANNEL);
  
}
