#ifndef DASHBOARD_H
#define DASHBOARD_N

#include <GxEPD.h>
#include <GxGDEW042Z15/GxGDEW042Z15.h>    // 4.2" b/w/r
#include GxEPD_BitmapExamples
// FreeFonts from Adafruit_GFX
/*#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>*/
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <SPI.h>
#include <SD.h>



class Dashboard {
    private:
        static const uint16_t input_buffer_pixels = 800; // may affect performance
        static const uint16_t max_row_width = 1448; // for up to 6" display 1448x1072
        static const uint16_t max_palette_pixels = 256; // for depth <= 8

        uint16_t read16(File& f);
        uint32_t read32(File& f);        
        uint8_t input_buffer[3 * input_buffer_pixels]; // up to depth 24
        uint8_t output_row_mono_buffer[max_row_width / 8]; // buffer for at least one row of b/w bits
        uint8_t output_row_color_buffer[max_row_width / 8]; // buffer for at least one row of color bits
        uint8_t mono_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 b/w
        uint8_t color_palette_buffer[max_palette_pixels / 8]; // palette buffer for depth <= 8 c/w
        uint16_t rgb_palette_buffer[max_palette_pixels]; // palette buffer for depth <= 8 for buffered graphics, needed for 7-color display
        void drawBitmapFromSD(File file, int16_t x, int16_t y, bool with_color);

    public:
        Dashboard();
        void drawBitmapFile(File file);
        void drawLogo(File f);

};
#endif