#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"
#include <WiFi.h>
#include <MQTT.h>
#include <MQTTClient.h>
#include "esp_camera.h"
#include <ArduinoJson.h>

const char ssid[] = "XX";
const char pass[] = "160802160802";
int flag ;
int flagwf;

WiFiClient net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  //Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    
    //Serial.print(".");
    delay(1000);
  }

  //Serial.print("\nconnecting...");
  while (!client.connect("arduino", "public", "public")) {
    //Serial.print(".");
    delay(1000);
  }
  client.subscribe("/clientSub");
  // client.unsubscribe("/hello");
}
void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}
//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  
   Serial.begin(9600);
   WiFi.begin(ssid, pass);
  client.begin("27.72.56.161",net);
  client.onMessage(messageReceived);
  connect();
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;  //0-63 lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  Serial.println("ok"); 
}

void loop()
{
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability
  if (!client.connected()) {
    Serial.println("wf");
    connect();
  }
  if(Serial.available ()) //nhan data tu STM32
  {
   String value;
   value=Serial.readString();
   if(value.length()> 72 && flag == 1)
   {  Serial.println("no");
     // Tìm vị trí của dấu ngoặc mở đầu của JSON thứ hai
   int secondJsonStart = value.indexOf("{", value.indexOf("{") + 1);

  if (secondJsonStart != -1) 
   {
    // Lấy chuỗi JSON thứ nhất
    String firstJson = value.substring(0, secondJsonStart - 1);// trich xuat tu vi tri 0 den vị tri truoc chuoi json thu 2

    // Lấy chuỗi JSON thứ hai
    String secondJson = value.substring(secondJsonStart);// trich xuat chuoi json thu 2

    // Xử lý chuỗi JSON thứ hai
     Photo2Base64();
  // Tìm vị trí của dấu ngoặc đóng }
     int closingBracket = secondJson.lastIndexOf("}");
   
    // Chèn chuỗi mới vào trước dấu đóng ngoặc }
  
     secondJson = secondJson.substring(0, closingBracket) + "," + Photo2Base64() + secondJson.substring(closingBracket);
     client.publish("/dataGPS_RFID",secondJson);
     //Serial.println(value);
     delay(5000);
     Serial.println("ok");
     flag=0; 
    } 
else 
    {
    // Trường hợp chỉ có một chuỗi JSON, không xử lý chuỗi JSON thứ hai
    String firstJson = value;
     Photo2Base64();
   int closingBracket = firstJson.lastIndexOf("}");
   firstJson = firstJson.substring(0, closingBracket) + "," + Photo2Base64() + firstJson.substring(closingBracket);
     client.publish("/dataGPS_RFID",firstJson);
     //Serial.println(value);
     delay(5000);
     Serial.println("ok");
     flag=0; 
    }
  }    
    else // chi nhan gps
    {
     flag = 0;
     client.publish("/dataGPS",value);
     //Serial.println("dataGPS");
     flag = 1; // de gui id len
     Serial.println("ok");
    }
  }
}
String Photo2Base64() 
{
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();  
    if(!fb) {
      //Serial.println("Camera capture failed");
      return "";
    }
    const char* fieldName = "\"image";
    String base64 = "";
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) base64 += urlencode(String(output));
    }
   //Serial.println(String("Size of the image...")+String(fb->len));
    esp_camera_fb_return(fb);
    String imageFile = String(fieldName) + "\":\"" + base64 + "\"";
   return imageFile;
}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}
