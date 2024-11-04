/*
 Adapted from the Adafruit and Xark's PDQ graphicstest sketch.

 See end of file for original header text and MIT license info.
 
 This sketch uses the GLCD font only.

 Make sure all the display driver and pin connections are correct by
 editing the User_Setup.h file in the TFT_eSPI library folder.

 #########################################################################
 ###### DON'T FORGET TO UPDATE THE User_Setup.h FILE IN THE LIBRARY ######
 #########################################################################
 */

#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>

#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
    char userName[50];
    char message[150];
} struct_message;

struct_message incomingMessage;
struct_message sendMessage;

esp_now_peer_info_t peerInfo;

#include <vector>
#include <string>

std::vector<std::string> bannedUsernames = {"user1", "user2", "user3"};

#define TFT_GREY 0x5AEB // New colour

TFT_eSPI tft = TFT_eSPI();  // Invoke library

int i = 0;

#define MAX_LINES 15
String lines[MAX_LINES];
int lineCount = 0;

void addLine(String line) {
  if (lineCount < MAX_LINES) {
    lines[lineCount] = line;
    lineCount++;
  } else {
    for (int i = 0; i < MAX_LINES - 1; i++) {
      lines[i] = lines[i + 1];
    }
    lines[MAX_LINES - 1] = line;
  }
}

void displayScrollback() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  for (int i = 0; i < lineCount; i++) {
    tft.println(lines[i]);
  }
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingMessage, incomingData, len);

  for (auto it = bannedUsernames.begin(); it != bannedUsernames.end(); ++it) {
    if (*it == std::string(incomingMessage.userName)) {
      return;
    }
  }


  Serial.print(incomingMessage.userName);
  Serial.print(": ");
  Serial.println(incomingMessage.message);
  Serial.println();

  addLine(String(incomingMessage.userName) + ": " + String(incomingMessage.message));
  displayScrollback();

}

void setup(void) {
  tft.init();
  tft.setRotation(1);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  strcpy(sendMessage.userName, "UnoMartino");

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);

}




void loop() {

  char text[150] = {0};

  if (Serial.available() > 0)
  { 

    int size = Serial.available();
    Serial.readBytes(text, size);

    if (strcmp(text, "/clear") == 0) {
      lineCount = 0;
      displayScrollback();
      tft.fillScreen(TFT_BLACK);
    } 
    else if (strncmp(text, "/ban ", 5) == 0) {
      String usernameToBan = String(text).substring(5);
      bannedUsernames.push_back(std::string(usernameToBan.c_str()));
      addLine("User " + usernameToBan + " has been banned.");
      displayScrollback();
    }
    else if (strncmp(text, "/unban ", 7) == 0) {
      String usernameToUnban = String(text).substring(7);
      std::string usernameToUnbanStr = std::string(usernameToUnban.c_str());
      for (auto it = bannedUsernames.begin(); it != bannedUsernames.end(); ++it) {
        if (*it == usernameToUnbanStr) {
          bannedUsernames.erase(it);
          addLine("User " + usernameToUnban + " has been unbanned.");
          displayScrollback();
          break;
        }
      }
    }
    else {
      strcpy(sendMessage.message, text);
      addLine("Ja: " + String(text));
      displayScrollback();
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendMessage, sizeof(sendMessage));
    }
  }
  
  // Fill screen with grey so we can see the effect of printing with and without 
  // a background colour defined
  // tft.fillScreen(TFT_GREY);
  
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)

  // Set the font colour to be white with a black background, set text size multiplier to 1
  
  // We can now plot text on screen using the "print" class

  // while (true) {
  //   addLine("Hello World! " + String(i));
  //   displayScrollback();
  //   i++;
  //   delay(1000);
  // }
  
  // Add text scrollback
  


  
  // // Set the font colour to be yellow with no background, set to font 7
  // tft.setTextColor(TFT_YELLOW); tft.setTextFont(7);
  // tft.println(1234.56);
  
  // // Set the font colour to be red with black background, set to font 4
  // tft.setTextColor(TFT_RED,TFT_BLACK);    tft.setTextFont(4);
  // //tft.println(3735928559L, HEX); // Should print DEADBEEF

  // // Set the font colour to be green with black background, set to font 4
  // tft.setTextColor(TFT_GREEN,TFT_BLACK);
  // tft.setTextFont(4);
  // tft.println("Groop");
  // tft.println("I implore thee,");

  // // Change to font 2
  // tft.setTextFont(2);
  // tft.println("my foonting turlingdromes.");
  // tft.println("And hooptiously drangle me");
  // tft.println("with crinkly bindlewurdles,");
  // // This next line is deliberately made too long for the display width to test
  // // automatic text wrapping onto the next line
  // tft.println("Or I will rend thee in the gobberwarts with my blurglecruncheon, see if I don't!");
  
  // // Test some print formatting functions
  // float fnumber = 123.45;
  //  // Set the font colour to be blue with no background, set to font 4
  // tft.setTextColor(TFT_BLUE);    tft.setTextFont(4);
  // tft.print("Float = "); tft.println(fnumber);           // Print floating point number
  // tft.print("Binary = "); tft.println((int)fnumber, BIN); // Print as integer value in binary
  // tft.print("Hexadecimal = "); tft.println((int)fnumber, HEX); // Print as integer number in Hexadecimal
  // delay(10000);
}




// #define UP      13
// #define DOWN    1
// #define LEFT    42
// #define RIGHT   4
// #define CTRL    14
// #define A       40
// #define B       41
// #define X       2
// #define Y       5

// #define LCD_DC    33
// #define LCD_CS    34
// #define LCD_CLK   35
// #define LCD_DIN   36
// #define LCD_RST   37
// #define LCD_BL    38


// void setup() {
//   Serial.begin(115200);
//   pinMode(UP, INPUT_PULLUP);
//   pinMode(DOWN, INPUT_PULLUP);
//   pinMode(LEFT, INPUT_PULLUP);
//   pinMode(RIGHT, INPUT_PULLUP);
//   pinMode(CTRL, INPUT_PULLUP);
//   pinMode(A, INPUT_PULLUP);
//   pinMode(B, INPUT_PULLUP);
//   pinMode(X, INPUT_PULLUP);
//   pinMode(Y, INPUT_PULLUP);
// }

// void loop() {
//   if(!digitalRead(UP)){
//     Serial.println("UP");
//   }
//   if(!digitalRead(DOWN)){
//     Serial.println("DOWN");
//   }
//   if(!digitalRead(LEFT)){
//     Serial.println("LEFT");
//   }
//   if(!digitalRead(RIGHT)){
//     Serial.println("RIGHT");
//   }
//   if(!digitalRead(CTRL)){
//     Serial.println("CTRL");
//   }
//   if(!digitalRead(A)){
//     Serial.println("A");
//   }
//   if(!digitalRead(B)){
//     Serial.println("B");
//   }
//   if(!digitalRead(X)){
//     Serial.println("X");
//   }
//   if(!digitalRead(Y)){
//     Serial.println("Y");
//   }

//   delay(100);
// }