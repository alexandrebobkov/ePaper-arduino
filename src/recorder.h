#include <SD.h>

const int chipSelect = SS;

// Filename where logs are recorded to
const char* logs_filename = "/logs.txt";

void initSdCard() {

    Serial.println("\n======================");
    Serial.print("\nInitializing SD card..."); 
    // Initialize SD library
    if (!SD.begin(chipSelect)) {
        Serial.println("initialization failed. Things to check:");
        Serial.println("* is a card inserted?");
        Serial.println("* is your wiring correct?");
        Serial.println("* did you change the chipSelect pin to match your shield or module?");
        while (1);
    }
    else {
        Serial.println("Wiring is correct and a card is present.");
    } 
    // print the type of card
    Serial.println();
    Serial.print("Card type:         ");
    switch (SD.cardType()) {
        case CARD_NONE:
            Serial.println("NONE");
            break;
        case CARD_MMC:
            Serial.println("MMC");
            break;
        case CARD_SD:
            Serial.println("SD");
            break;
        case CARD_SDHC:
            Serial.println("SDHC");
            break;
        default:
            Serial.println("Unknown");
    }
    Serial.print("Card size:  ");
    Serial.println((float)SD.cardSize()/1000); 
    Serial.print("Total bytes: ");
    Serial.println(SD.totalBytes()); 
    Serial.print("Used bytes: ");
    Serial.println(SD.usedBytes());

    // Remove previous json file
    //SD.remove(logs_filename);
    Serial.println("\n==== SD Card Initialized ====");
}

void printFile (const char* filename) {
    File file = SD.open(filename);
    if (!file) {
        Serial.println(F("Failed to read file"));
        return;
    }

    while (file.available()) {
        Serial.print((char)file.read());
    }
    Serial.println();
    file.close();
}

void updateJson () {

    Serial.println("\n==== recorder.h Writing JSON file. ====");

    DynamicJsonDocument jdoc(1024);
    JsonObject obj;

    File file = SD.open(logs_filename);
    if (!file) {
        Serial.println(F("Failed to create file"));
        obj = jdoc.to<JsonObject>();
    }
    else {
        DeserializationError error = deserializeJson(jdoc, file);
        if (error) {
            // if the file didn't open, print an error:
            Serial.println(F("Error parsing JSON "));
            Serial.println(error.c_str());
 
            // create an empty JSON object
            obj = jdoc.to<JsonObject>();
        }
        else {
            // GET THE ROOT OBJECT TO MANIPULATE
            obj = jdoc.as<JsonObject>();
        }
    }

    file.close();
 
    obj[F("millis")] = millis();
 
    JsonArray data;
    // Check if exist the array
    if (!obj.containsKey(F("data"))) {
        Serial.println(F("Not find data array! Crete one!"));
        data = obj.createNestedArray(F("data"));
    } else {
        Serial.println(F("Find data array!"));
        data = obj[F("data")];
    }
 
    // create an object to add to the array
    JsonObject objArrayData = data.createNestedObject();
 
    objArrayData["prevNumOfElem"] = data.size();
    objArrayData["newNumOfElem"] = data.size() + 1;
 
    SD.remove(logs_filename);
 
    // Open file for writing
    file = SD.open(logs_filename, FILE_WRITE);
 
    // Serialize JSON to file
    if (serializeJson(jdoc, file) == 0) {
        Serial.println(F("Failed to write to file"));
    }
 
    // Close the file
    file.close();

    Serial.println("\n==== recorder.h Writing JSON file. DONE ====");
}

void displayLogo () {
  File dir =  SD.open("/");
  drawLogo(SD.open("/picture-001.bmp"));
  delay(5000);
  Serial.println("\n==== Logo Displayed ====");
}