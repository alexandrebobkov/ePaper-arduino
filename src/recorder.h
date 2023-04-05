#include <SD.h>

const int chipSelect = SS;

// Filename where JSON logs are recorded to
const char* logs_filename = "/sensors.txt";
File file;

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
    //Serial.println();
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
    file = SD.open(filename);
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

    DynamicJsonDocument jdoc(2048);
    JsonObject obj;

    File file = SD.open(logs_filename, FILE_WRITE);
    
    if (!file) {
    //if (!SD.exists(logs_filename)) {
        Serial.println(F("File does not exist."));
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
        Serial.println(F("Not found data array! Creted one!"));
        data = obj.createNestedArray(F("data"));
    } else {
        Serial.println(F("Found data array!"));
        data = obj[F("data")];
    }

    JsonArray sensors;
    if (!obj.containsKey(F("sensors"))) {
        Serial.println(F("Not found sensors array! Creted one!"));
        sensors = obj.createNestedArray(F("sensors"));
    } else {
        Serial.println(F("Found sensors array!"));
        sensors = obj[F("sensors")];
    }

 
    // create an object to add to the array
    JsonObject objArrayData = data.createNestedObject(); 
    objArrayData["prevNumOfElem"] = data.size();
    objArrayData["newNumOfElem"] = data.size() + 1;

    JsonObject objSensorsData = sensors.createNestedObject();
    objSensorsData["Humidity"] = "26%";
    objSensorsData["Temperature"] = "10";
    objSensorsData["Pressure"] = "10";
 
    SD.remove(logs_filename);
 
    // Open file for writing
    file = SD.open(logs_filename, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to create file");
    }

    file.println("Sensors values.");
 
    /*// Serialize JSON to file
    if (serializeJson(jdoc, file) == 0) {
        Serial.println(F("Failed to write to file"));
        return;
    }*/
 
    // Close the file
    file.close();

    Serial.println("\n==== recorder.h Writing JSON file. DONE ====");
}

void displayImage (String file_path) {
    File dir =  SD.open("/");
    drawLogo(SD.open(file_path));
    Serial.println("\n==== Logo Displayed ====");
}

void displayUi () {
    displayImage("/ui-002.bmp");
}
void displayLogo () {
    displayImage("/picture-001.bmp");
}