

#include <Arduino.h>
#include <ESP_Panel_Library.h>
#include <ESP_IOExpander_Library.h>
#include <ui.h>
#include <lvgl.h>
#include "lvgl_port_v8.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
#include <WiFi.h>              //Built-in
#include <ESP32WebServer.h>    //https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "CSS.h" //Includes headers of the web and de style file
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "PCF8575.h"  // https://github.com/xreef/PCF8575_library


void button1Pressed(lv_event_t * e);
void button2Pressed(lv_event_t * e);
void button3Pressed(lv_event_t * e);
void button4Pressed(lv_event_t * e);
void button5Pressed(lv_event_t * e);
void button6Pressed(lv_event_t * e);
void button7Pressed(lv_event_t * e);
void button8Pressed(lv_event_t * e);
void button9pressed(lv_event_t * e);
void button10Pressed(lv_event_t * e);
void button11Pressed(lv_event_t * e);
void button12Pressed(lv_event_t * e);
void wifiSwitchToggle(lv_event_t * e);
void colorChange(lv_event_t * e);
char buttonModifiers[12][20];
char buttonValues[12][200];  // Assuming values might be longer
int colors[][3] = {
    {255, 0, 0},   // Red
    {0, 255, 0},   // Green
    {0, 0, 255},   // Blue
    {255, 255, 0}, // Yellow
    {0, 255, 255}, // Cyan
    {255, 0, 255}, // Magenta
    {192, 192, 192}, // Silver
    {128, 128, 128}, // Gray
    {0, 128, 0},   // Dark Green
    {128, 0, 0},   // Maroon
    {128, 0, 128}, // Purple
    {0, 128, 128}, // Teal
    {0, 0, 128},   // Navy
    {255, 165, 0}, // Orange
    {255, 192, 203} // Pink
};

int colorIndex = 0; // Global variable to keep track of the current color
// Extend IO Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

// Extend IO Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5
USBHIDKeyboard Keyboard;
const int buttonPin = 0;  // input pin for pushbutton
#define RXD2 15
#define TXD2 16
const int slaveConnectionPin = 6;  // GPIO pin on Master to detect Slave connection
bool isSlaveConnected = false;     // Tracks whether the slave is connected

int buttonState = 0;  // Store received button states
#define SD_MOSI 11
#define SD_CLK  12
#define SD_MISO 13
#define SD_SS -1
// I2C Pin define
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_SCL_IO 9
ESP32WebServer server(80);
#define servername "keyboard" //Define the name to your server... 
bool   SD_present = false; //Controls if the SD card is present or not
char nwMode = 0;


void handleModifier(const char* modifier, const char* value) {
  if (strcmp(modifier, "paste") == 0) {
    if (value != NULL && strlen(value) > 0) {
      // If a value is provided, simulate typing the value (custom paste)
      Keyboard.println(value);  // Simulate typing the provided value
    } else {
      // If no value is provided, perform native paste (CTRL + V)
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('v');
      delay(200);  // Small delay to ensure key press is registered
      Keyboard.releaseAll();  // Release the keys
    }

  } else if (strcmp(modifier, "copy") == 0) {
    // Handle native copy action (CTRL + C)
    Keyboard.press(KEY_LEFT_CTRL);
    Keyboard.press('c');
    delay(200);  // Small delay to ensure key press is registered
    Keyboard.releaseAll();  // Release the keys

  } else if (strcmp(modifier, "open") == 0) {
    // Use Windows + R to open the Run dialog
    Keyboard.press(KEY_LEFT_GUI);
    Keyboard.press('r');
    delay(200);  // Allow time for the Windows + R to open the Run dialog
    Keyboard.releaseAll();

    delay(100);  // Wait for the Run dialog to appear

    Keyboard.println(value);  // Type the value (path)
    delay(50);

    Keyboard.press(KEY_RETURN);  // Press Enter to execute the command
    delay(100);
    Keyboard.releaseAll();  // Release all keys

  } else if (strcmp(modifier, "screenshot") == 0) {
    // Take a screenshot using Alt + Print Screen
    Keyboard.press(KEY_RIGHT_ALT);
    Keyboard.press(KEY_PRINT_SCREEN);
    delay(200);  // Allow time for the keys to register
    Keyboard.releaseAll();  // Release both keys
  
  } else {
    // Add a fallback for unsupported modifiers
    // Serial.println(modifier);  // Uncomment for debugging
  }
}
  


// Global variable to store the LED color in "R,G,B" format
String ledColorString = "0,0,0";  // Default to black

void readJsonFile(fs::FS &fs, const char *path) {
  // Open the JSON file from the SD card
  File file = fs.open(path);
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  // Calculate the size of the file
  size_t size = file.size();
  if (size == 0) {
    Serial.println("File is empty");
    return;
  }

  // Allocate a buffer to store the content of the file
  std::unique_ptr<char[]> buf(new char[size]);

  // Read all the content of the file into the buffer
  file.readBytes(buf.get(), size);

  // Parse the JSON data
  DynamicJsonDocument doc(4096);  // Adjust size based on the JSON complexity
  DeserializationError error = deserializeJson(doc, buf.get());

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.f_str());
    return;
  }

  // Iterate over all button key-value pairs in the JSON object
  for (JsonPair kv : doc.as<JsonObject>()) {
    const char* key = kv.key().c_str();
    JsonObject obj = kv.value().as<JsonObject>();

    // Check if "value", "modifier", and "text" keys exist
    if (obj.containsKey("modifier") && obj.containsKey("value") && obj.containsKey("text")) {
      const char* modifier = obj["modifier"];
      const char* value = obj["value"];
      const char* text = obj["text"];  // Read the 'text' key for the textarea content

      // Store the modifier and value based on the button key
      if (strcmp(key, "button1") == 0) {
        strncpy(buttonModifiers[0], modifier, sizeof(buttonModifiers[0]) - 1);
        strncpy(buttonValues[0], value, sizeof(buttonValues[0]) - 1);
        lv_textarea_set_text(ui_TextArea4, text);  // Update the textarea with the 'text' field
      } else if (strcmp(key, "button2") == 0) {
        strncpy(buttonModifiers[1], modifier, sizeof(buttonModifiers[1]) - 1);
        strncpy(buttonValues[1], value, sizeof(buttonValues[1]) - 1);
        lv_textarea_set_text(ui_TextArea5, text);
      } else if (strcmp(key, "button3") == 0) {
        strncpy(buttonModifiers[2], modifier, sizeof(buttonModifiers[2]) - 1);
        strncpy(buttonValues[2], value, sizeof(buttonValues[2]) - 1);
        lv_textarea_set_text(ui_TextArea6, text);
      } else if (strcmp(key, "button4") == 0) {
        strncpy(buttonModifiers[3], modifier, sizeof(buttonModifiers[3]) - 1);
        strncpy(buttonValues[3], value, sizeof(buttonValues[3]) - 1);
        lv_textarea_set_text(ui_TextArea7, text);
      } else if (strcmp(key, "button5") == 0) {
        strncpy(buttonModifiers[4], modifier, sizeof(buttonModifiers[4]) - 1);
        strncpy(buttonValues[4], value, sizeof(buttonValues[4]) - 1);
        lv_textarea_set_text(ui_TextArea8, text);
      } else if (strcmp(key, "button6") == 0) {
        strncpy(buttonModifiers[5], modifier, sizeof(buttonModifiers[5]) - 1);
        strncpy(buttonValues[5], value, sizeof(buttonValues[5]) - 1);
        lv_textarea_set_text(ui_TextArea9, text);
      } else if (strcmp(key, "button7") == 0) {
        strncpy(buttonModifiers[6], modifier, sizeof(buttonModifiers[6]) - 1);
        strncpy(buttonValues[6], value, sizeof(buttonValues[6]) - 1);
        lv_textarea_set_text(ui_TextArea10, text);
      } else if (strcmp(key, "button8") == 0) {
        strncpy(buttonModifiers[7], modifier, sizeof(buttonModifiers[7]) - 1);
        strncpy(buttonValues[7], value, sizeof(buttonValues[7]) - 1);
        lv_textarea_set_text(ui_TextArea11, text);
      } else if (strcmp(key, "button9") == 0) {
        strncpy(buttonModifiers[8], modifier, sizeof(buttonModifiers[8]) - 1);
        strncpy(buttonValues[8], value, sizeof(buttonValues[8]) - 1);
        lv_textarea_set_text(ui_TextArea12, text);
      } else if (strcmp(key, "button10") == 0) {
        strncpy(buttonModifiers[9], modifier, sizeof(buttonModifiers[9]) - 1);
        strncpy(buttonValues[9], value, sizeof(buttonValues[9]) - 1);
        lv_textarea_set_text(ui_TextArea13, text);
      } else if (strcmp(key, "button11") == 0) {
        strncpy(buttonModifiers[10], modifier, sizeof(buttonModifiers[10]) - 1);
        strncpy(buttonValues[10], value, sizeof(buttonValues[10]) - 1);
        lv_textarea_set_text(ui_TextArea14, text);
      } else if (strcmp(key, "button12") == 0) {
        strncpy(buttonModifiers[11], modifier, sizeof(buttonModifiers[11]) - 1);
        strncpy(buttonValues[11], value, sizeof(buttonValues[11]) - 1);
        lv_textarea_set_text(ui_TextArea15, text);
      }
    }
  }

  // Handle the LED color part
  if (doc.containsKey("ledColor")) {
    JsonArray ledColorArray = doc["ledColor"];

    if (ledColorArray.size() == 3) {  // Ensure it has three values
      int red = ledColorArray[0];
      int green = ledColorArray[1];
      int blue = ledColorArray[2];

      // Store the RGB values in a global string in "R,G,B" format
      ledColorString = String(red) + "," + String(green) + "," + String(blue);

      // Print the RGB values to the Serial port
      Serial.println("LED Color: " + ledColorString);
    } else {
      Serial.println("Invalid LED color format");
    }
  }

  file.close();
}



void startWebServer() {
  WiFi.softAP("KeyboardAP", "password"); //Network and password for the access point genereted by ESP32

  //Set your preferred server name, if you use "mcserver" the address would be http://mcserver.local/
  if (!MDNS.begin(servername))
  {
    Serial.println(F("Error setting up MDNS responder!"));
    ESP.restart();
  }
  server.on("/",         SD_dir);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST, []() {
    server.send(200);
  }, handleFileUpload);

  server.begin();
  nwMode = 1;
  Serial.println("HTTP server started");
}

void endProgram() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
   ESP.restart();
  //Serial.println("WiFi and server deinitialized. Program ended.");
  nwMode = 0;

  
}

 




void setup()
{
Serial.begin(9600);
  pinMode(GPIO_INPUT_IO_4, OUTPUT); 
    pinMode(slaveConnectionPin, INPUT_PULLUP);  // Set the GPIO 6 pin to input mode

  Keyboard.begin();
  USB.begin();
//Serial.println("Initialize IO expander");


  /* Initialize IO expander */
  ESP_IOExpander *expander = new ESP_IOExpander_CH422G((i2c_port_t)I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000, I2C_MASTER_SCL_IO, I2C_MASTER_SDA_IO);
  // ESP_IOExpander *expander = new ESP_IOExpander_CH422G(I2C_MASTER_NUM, ESP_IO_EXPANDER_I2C_CH422G_ADDRESS_000);
  expander->init();
  expander->begin();
  expander->multiPinMode(TP_RST | LCD_BL | LCD_RST | SD_CS | USB_SEL, OUTPUT);
  expander->multiDigitalWrite(TP_RST | LCD_BL | LCD_RST, HIGH);
  delay(100);
  expander->multiDigitalWrite(TP_RST | LCD_RST, LOW);
  delay(100);
  digitalWrite(GPIO_INPUT_IO_4, LOW);
  delay(100);
  expander->multiDigitalWrite(TP_RST | LCD_RST, HIGH);
  delay(200);
 expander->digitalWrite(SD_CS, LOW);
  expander->digitalWrite(USB_SEL, LOW);
  SPI.setHwCs(false);
  SPI.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
  if (!SD.begin()) {
    //Serial.println("Card Mount Failed");
    
    SD_present = false;
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    //Serial.println("No SD card attached");
    return;
  }
  else
  {
    //Serial.println(F("Card initialised... file access enabled..."));
   
    SD_present = true;
  } 

  // Serial.println("Initialize panel device");
  ESP_Panel *panel = new ESP_Panel();
  panel->init();
#if LVGL_PORT_AVOID_TEAR
  // When avoid tearing function is enabled, configure the RGB bus according to the LVGL configuration
  ESP_PanelBus_RGB *rgb_bus = static_cast<ESP_PanelBus_RGB *>(panel->getLcd()->getBus());
  rgb_bus->configRgbFrameBufferNumber(LVGL_PORT_DISP_BUFFER_NUM);
  rgb_bus->configRgbBounceBufferSize(LVGL_PORT_RGB_BOUNCE_BUFFER_SIZE);
#endif
  panel->begin();

  // Serial.println("Initialize LVGL");
  lvgl_port_init(panel->getLcd(), panel->getTouch());
  lvgl_port_lock(-1);
  ui_init();
 /* Release the mutex */
  lvgl_port_unlock();
  lv_label_set_text(ui_Label3, "My updated text");
//String full = String(servername) + ".local";
char full[50];
strcpy(full, servername);    // Copy the servername to 'full'
  strcat(full, ".local");      // Concatenate ".local" to 'full'
 readJsonFile(SD, "/button.json");
  lv_textarea_set_text(ui_TextArea3, "KeyboardAP");
  lv_textarea_set_text(ui_TextArea2, "password");
 lv_textarea_set_text(ui_TextArea1, full);
}

void loop()
{
delay(1);
if (digitalRead(slaveConnectionPin) == LOW) {
   if (!isSlaveConnected) {
      Serial.println("Slave connected. Initiating communication...");
      // Slave is newly connected, send RGB values
      
      Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
      delay(1000);
  Serial2.print(colors[colorIndex][0]);
  Serial2.print(",");
  Serial2.print(colors[colorIndex][1]);
  Serial2.print(",");
  Serial2.println(colors[colorIndex][2]);

      isSlaveConnected = true;  // Update state to indicate that slave is connected
    }

  if (Serial2.available() > 0) {
    int buttonPressed = Serial2.read();  // Read the button number (1 to 12, or 0 for no press)

    // Process the button number
    if (buttonPressed > 0) {
      Serial.print("Button ");
      Serial.print(buttonPressed);
       handleModifier(buttonModifiers[buttonPressed-1], buttonValues[buttonPressed-1]);
      Serial.println(" is pressed.");
    } else {
      Serial.println("No button is pressed.");
    }
  }




   if (Serial.available()) {
    String input = Serial.readStringUntil('\n');  // Read input until newline
    processRGBInput(input);  // Process the input and send the RGB data to the Slave
  }
    }
    else {
    if (isSlaveConnected) {
      // Slave has been disconnected
      Serial.println("Slave disconnected. Resetting communication...");
       Serial2.flush();  // Optional: clear any remaining data in the serial buffer
         Serial2.end();
      isSlaveConnected = false;  // Update state to indicate that slave is disconnected
    }
  }
  
  if (nwMode == 1) {
    server.handleClient(); //Listen for client connections
    //Serial.println("IDLE loop");
  }
}


void processRGBInput(String rgbData) {
  // Ensure the format is correct (e.g., 255,0,0)
  if (rgbData.indexOf(',') != -1 && rgbData.length() > 5) {
    Serial2.print(rgbData + "\n");  // Send the RGB data to the Slave via Serial2
    Serial.print("Sent RGB to Slave: ");
    Serial.println(rgbData);   // Print the sent data for debugging
  } else {
    Serial.println("Invalid RGB format. Please enter in the format R,G,B");
  }


}
void button1Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[0], buttonValues[0]);
}

void button2Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[1], buttonValues[1]);
}

void button3Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[2], buttonValues[2]);  // Corrected index
}

void button4Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[3], buttonValues[3]);  // Corrected index
}

void button5Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[4], buttonValues[4]);  // Corrected index
}

void button6Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[5], buttonValues[5]);  // Corrected index
}

void button7Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[6], buttonValues[6]);  // Corrected index
}

void button8Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[7], buttonValues[7]);  // Corrected index
}

void button9Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[8], buttonValues[8]);  // Corrected index
}

void button10Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[9], buttonValues[9]);  // Corrected index
}

void button11Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[10], buttonValues[10]);  // Corrected index
}

void button12Pressed(lv_event_t * e)
{
 handleModifier(buttonModifiers[11], buttonValues[11]);  // Corrected index
}

void colorChange(lv_event_t * e)
{Serial.println("pressed");
  // Print the current color in the format "R,G,B"
  Serial2.print(colors[colorIndex][0]);
  Serial2.print(",");
  Serial2.print(colors[colorIndex][1]);
  Serial2.print(",");
  Serial2.println(colors[colorIndex][2]);

  // Increment the index to move to the next color
  colorIndex++;

  // If the index reaches the end of the array, reset to 0
  if (colorIndex >= sizeof(colors) / sizeof(colors[0])) {
    colorIndex = 0;
  }
}

void wifiSwitchToggle(lv_event_t * e)
{
 if (lv_obj_has_state(ui_Switch1, LV_STATE_CHECKED)){
   startWebServer();
}
else{
   endProgram();
}
}


/*********  FUNCTIONS  **********/
//Initial page of the server web, list directory and give you the chance of deleting and uploading
void SD_dir()
{
  if (SD_present)
  {
    //Action acording to post, dowload or delete, by MC 2022
    if (server.args() > 0 ) //Arguments were received, ignored if there are not arguments
    {
     Serial.println(server.arg(0));

      String Order = server.arg(0);
      Serial.println(Order);

      if (Order.indexOf("download_") >= 0)
      {
        Order.remove(0, 9);
        SD_file_download(Order);
        Serial.println(Order);
      }

      if ((server.arg(0)).indexOf("delete_") >= 0)
      {
        Order.remove(0, 7);
        SD_file_delete(Order);
        Serial.println(Order);
      }
    }

    File root = SD.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/", 0);
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    }
    else
    {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();   //Stop is needed because no content length was sent
  } else ReportSDNotPresent();
}

//Upload a file to the SD
void File_Upload()
{
  append_page_header();
  webpage += F("<h3>Select File to Upload</h3>");
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:25%' type='file' name='fupload' id = 'fupload' value=''>");
  webpage += F("<button class='buttons' style='width:10%' type='submit'>Upload File</button><br><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  server.send(200, "text/html", webpage);
}

//Prints the directory, it is called in void SD_dir()
void printDirectory(const char * dirname, uint8_t levels)
{

  File root = SD.open(dirname);

  if (!root) {
    return;
  }
  if (!root.isDirectory()) {
    return;
  }
  File file = root.openNextFile();

  int i = 0;
  while (file) {
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if (file.isDirectory()) {
      webpage += "<tr><td>" + String(file.isDirectory() ? "Dir" : "File") + "</td><td>" + String(file.name()) + "</td><td></td></tr>";
      printDirectory(file.name(), levels - 1);
    }
    else
    {
      webpage += "<tr><td>" + String(file.name()) + "</td>";
      webpage += "<td>" + String(file.isDirectory() ? "Dir" : "File") + "</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes) + " B";
      else if (bytes < (1024 * 1024))        fsize = String(bytes / 1024.0, 3) + " KB";
      else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
      else                                  fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
      webpage += "<td>" + fsize + "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>");
      webpage += F("<button type='submit' name='download'");
      webpage += F("' value='"); webpage += "download_" + String(file.name()); webpage += F("'>Download</button>");
      webpage += "</td>";
      webpage += "<td>";
      webpage += F("<FORM action='/' method='post'>");
      webpage += F("<button type='submit' name='delete'");
      webpage += F("' value='"); webpage += "delete_" + String(file.name()); webpage += F("'>Delete</button>");
      webpage += "</td>";
      webpage += "</tr>";

    }
    file = root.openNextFile();
    i++;
  }
  file.close();


}

//Download a file from the SD, it is called in void SD_dir()
void SD_file_download(String filename)
{
  if (SD_present)
  {
    File download = SD.open("/" + filename);
    if (download)
    {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename=" + filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download");
  } else ReportSDNotPresent();
}

//Handles the file upload a file to the SD
File UploadFile;
//Upload a new file to the Filing system
void handleFileUpload()
{
  HTTPUpload& uploadfile = server.upload(); //See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
  //For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if (uploadfile.status == UPLOAD_FILE_START)
  {
    String filename = uploadfile.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    SD.remove(filename);                         //Remove a previous version, otherwise data is appended the file again
    UploadFile = SD.open(filename, FILE_WRITE);  //Open the file for writing in SD (create it, if doesn't exist)
    filename = String();
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    if (UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  }
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if (UploadFile)         //If the file was successfully created
    {
      UploadFile.close();   //Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>");
      webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename + "</h2>";
      webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br><br>";
      webpage += F("<a href='/'>[Back]</a><br><br>");
      append_page_footer();
      server.send(200, "text/html", webpage);
    }
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}

//Delete a file from the SD, it is called in void SD_dir()
void SD_file_delete(String filename)
{
  if (SD_present) {
    SendHTML_Header();
    File dataFile = SD.open("/" + filename, FILE_READ); //Now read data from SD Card
    if (dataFile)
    {
      if (SD.remove("/" + filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '" + filename + "' has been erased</h3>";
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
      else
      {
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='/'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSDNotPresent();
}

//SendHTML_Header
void SendHTML_Header()
{
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", ""); //Empty content inhibits Content-length header so we have to close the socket ourselves.
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Content
void SendHTML_Content()
{
  server.sendContent(webpage);
  webpage = "";
}

//SendHTML_Stop
void SendHTML_Stop()
{
  server.sendContent("");
  server.client().stop(); //Stop is needed because no content length was sent
}

//ReportSDNotPresent
void ReportSDNotPresent()
{
  SendHTML_Header();
  webpage += F("<h3>No SD Card present</h3>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//ReportFileNotPresent
void ReportFileNotPresent(String target)
{
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>");
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//ReportCouldNotCreateFile
void ReportCouldNotCreateFile(String target)
{
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>");
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}

//File size conversion
String file_size(int bytes)
{
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes) + " B";
  else if (bytes < (1024 * 1024))      fsize = String(bytes / 1024.0, 3) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
  else                              fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
  return fsize;
}
