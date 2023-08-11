#include <EEPROM.h>
//#include <WebServer.h>
// Tạo đối tượng ESP32WebServer
WebServer server(80);
// mang lưu ssid và pass wif 
char tenwf1[30]; 
char passwf1[30];

// phần html cho tran webserver
void handleRoot() {
   // Phản hồi trang web với form để nhập thông tin mạng Wi-Fi mới
  String html = "<html><head><style>";
  html += "body { background-color: #F5F5F5; }";
  html += "h1 { color: #333; text-align: center; }";
  html += "input[type=text], input[type=password] { padding: 12px 20px; margin: 8px 0; box-sizing: border-box; border: 2px solid #ccc; border-radius: 4px; }";
  html += "input[type=submit] { background-color: #4CAF50; color: white; padding: 14px 20px; margin: 8px 0; border: none; border-radius: 4px; cursor: pointer; }";
  html += "input[type=submit]:hover { background-color: #45a049; }";
  html += "</style></head><body><h1>NHAP WIFI MA BAN MUON KET NOI  </h1>";
  html += "<form method='post' action='/connect'>";
  html += "SSID:<input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='password'><br>";
  html += "<input type='submit' value='Connect'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleConnect() {

   String html = "<html><body>";
  html += "<h1>Da cap nhap thanh cong he thong se khoi dong lai de ket noi den wifi moi !! </h1>";
  html += "</body></html>";
  server.send(200, "text/html", html);

  // Lấy tên và mật khẩu mạng Wi-Fi mới từ form
  String newSSID = server.arg("ssid");
  String newPassword = server.arg("password");
  ///////////////////////////////////////////////
   int i2;
   int dodaitk , dodaimk ;
   
  EEPROM.begin(512);
    for(i2=0;i2<100;i2++)EEPROM.write(i2, 0); //xóa eeprom
    for(i2=0;i2<50;i2++)tenwf1[i2]=0;         // xóa tkwf
    for(i2=0;i2<50;i2++)passwf1[i2]=0;        // xóa pass
    
  ////do do dai 
        dodaitk = newSSID.length(); 
        for(i2=0;i2<dodaitk;i2++){if(newSSID[i2]=='+')EEPROM.write(i2,' '); else EEPROM.write(i2,newSSID[i2]);}
        dodaimk = newPassword.length(); 
        for(i2=0;i2<dodaimk;i2++){if(newPassword[i2]=='+')EEPROM.write(i2,' '); else EEPROM.write(i2+30,newPassword[i2]);}
        EEPROM.commit();// lưu thay đổi 
        delay(1000);
        ESP.restart(); // tự reset lại esp để kết nối tới mạng vừa thay đổi
        
         

   
 
 
}