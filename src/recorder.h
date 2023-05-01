#include <SD.h>

const int chipSelect = SS;

// Filename where JSON logs are recorded to
//const char* logs_filename = "/logs.txt";//data.txt";
const char* logs_filename = "/data.txt";
//File file;

class Recorder {
    public:
        void initSdCard();
        File openFile();
        void closeFile(File f);
        void appendValues(String time, float temperature, float humidity, float pressure);
        void displayImage (String file_path);
    private:
        File file;
};

void Recorder::initSdCard() {

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

        file = SD.open(logs_filename, FILE_WRITE);
        if (!file) {
            file.println("Time Stamp, Temperature (C), Humidity (%), Pressure (kPa)");
            //file.close();
        }
        file.close();
        
    }   
    //SD.close();
    //SD.end();
    Serial.println("\n==== SD Card Initialized ====");
}

void printFile (File f, const char* filename) {
    f = SD.open(filename);
    if (!f) {
        Serial.println(F("Failed to read file"));
        return;
    }

    while (f.available()) {
        Serial.print((char)f.read());
    }
    Serial.println();
    f.close();
}

File Recorder::openFile() {
    return SD.open(logs_filename, FILE_APPEND);
}
void Recorder::closeFile(File f) {
    f.close();
}
void Recorder::appendValues(String time, float temperature, float humidity, float pressure) {
    String data_string, temperature_s, humidity_s, pressure_s;

    //floatToString(temperature_s, temperature, 4);
    Serial.println("\n==== recorder.h Appending sensor values. ====");

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
        file = SD.open(logs_filename, FILE_APPEND);

    //File file = SD.open(logs_filename, FILE_WRITE);
    //file = SD.open(logs_filename, FILE_APPEND);
    
        if (!file) {
            //if (!SD.exists(logs_filename)) {
            Serial.println(F("File does not exist."));
            file.println("Sensors values.");
            file.println("Time | Sensor Value");
        }
        else {
            Serial.println("Appending sensor values ...");
            data_string = time + ", "
            +String(temperature) + "C, "
            +String(humidity) + "%, "
            +String(pressure) + "kPa";
            file.print(millis()+", ");
            file.println(data_string);
            file.flush();
        }
        file.close();        
    }
    //SD.end();

    //printFile(logs_filename);
    //return true;
    
    Serial.println("Time: "+time+" "+"   Temperature: "+temperature+"C   Humidity: "+humidity+"%   Pressure: "+pressure+"kPa");
    Serial.println("==== recorder.h Appended sensor values. ====");
}

void updateData() {
    String data_string;
    Serial.println("\n==== recorder.h Writing data file. ====");
    //File file = SD.open(logs_filename, FILE_WRITE);
    File file = SD.open(logs_filename, FILE_APPEND);
    
    if (!file) {
    //if (!SD.exists(logs_filename)) {
        Serial.println(F("File does not exist."));
        file.println("Sensors values.");
        file.println("Time | Sensor Value");
    }
    else {
        data_string = "57715," + String(10.0);
        file.println(data_string);
        /*file.print("Time, ");
        file.print(millis());
        file.println("10.0");*/
    }
    file.close();

    printFile(file, logs_filename);
}

void updateJson () {

    Serial.println("\n==== recorder.h Writing JSON file. ====");

    DynamicJsonDocument jdoc(1024);//2048);
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

void Recorder::displayImage (String file_path) {
    //File dir =  SD.open("/");
    file = SD.open(file_path);
    drawLogo(file);
    file.close();
    //SD.end();
    Serial.println("\n==== Logo Displayed ====");
}

/*void displayUi () {
    displayImage("/ui-002.bmp");
}
void displayLogo () {
    displayImage("/picture-001.bmp");
}*/