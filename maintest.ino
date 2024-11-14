#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <ESP_Mail_Client.h>


#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL "sem6project1@gmail.com"
#define AUTHOR_PASSWORD "hwqxpwqvfpnouvtq"
#define RECIPIENT_EMAIL "heelghevariya@gmail.com"

// Insert your network credentials
#define echoPin 22
#define trigPin 23
#define WIFI_SSID "Tp-link"
#define WIFI_PASSWORD "Heel972697@_"
#define DATABASE_URL "https://postbox-db59f-default-rtdb.firebaseio.com"
#define API_KEY "AIzaSyAAVndktvx5oE2VFJjG0vIKlpStitXEgck"
#define USER_EMAIL "heelghevariya@gmail.com"
#define USER_PASSWORD "123123"

//Define Firebase Data object
SMTPSession smtp;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

long dur =0;
double distance =0;
unsigned long sendDataPrevMillis = 0;
unsigned long count = 0;
int notify=0;

void smtpCallback(SMTP_Status status);

double dist(){
    digitalWrite(trigPin,LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin,HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin,LOW);
    dur = pulseIn(echoPin,HIGH);
    Serial.println(dur);
    distance=dur*0.034/2; //speed of sound wave divided by 2 (go and back)
    Serial.print(distance);
    return distance;
}

void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
    
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Firebase.setDoubleDigits(5);
}

void loop(){
  
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 2000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    Serial.printf("Set Double... %s\n", Firebase.setDouble(fbdo, F("/User/distance"), dist()) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Get Double... %s\n", Firebase.getDouble(fbdo, F("/User/distance")) ? String(fbdo.to<double>()).c_str() : fbdo.errorReason().c_str());
 }

  double distance = dist();
  Serial.println(distance);

  if(distance < 8){
    notify=1;
    smtp.debug(1);
    smtp.callback(smtpCallback);
    ESP_Mail_Session session;
    session.server.host_name = SMTP_HOST;
    session.server.port = SMTP_PORT;
    session.login.email = AUTHOR_EMAIL;
    session.login.password = AUTHOR_PASSWORD;
    session.login.user_domain = "";
    SMTP_Message message;
    message.sender.name = "ESP";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "Post Notification";
    message.addRecipient("Heel", RECIPIENT_EMAIL);
    String htmlMsg = "<div style=\"color:#2f4468;\"><h1>New post received in your postbox</h1></div>";
    message.html.content = htmlMsg.c_str();
    message.html.content = htmlMsg.c_str();
    message.text.charSet = "us-ascii";
    message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
    if (!smtp.connect(&session))
      return;
    if(notify == 1){
      if (!MailClient.sendMail(&smtp, &message))
        Serial.println("Error sending Email, " + smtp.errorReason());
      }
      else
      {
          notify=0;
      }
    }
    delay(15000);
}

void smtpCallback(SMTP_Status status){
  Serial.println(status.info());
  if (status.success()){
    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;
    Serial.println("----------------\n");
  }
 
}
