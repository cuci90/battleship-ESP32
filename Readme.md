# Battleship ESP32 Game

## Introduction
This is a battlehisp game using ESP32-2432S028R (CYD).  WS2812 LED stripe is used to display the hits on the battlefield.
Follow the instructions on instructables to build one for you own:

## Dependencies
https://github.com/lvgl/lvgl <br>
https://github.com/Bodmer/TFT_eSPI <br>
https://github.com/PaulStoffregen/XPT2046_Touchscreen <br>
https://github.com/adafruit/Adafruit_NeoPixel <br>

## Implementation
For each Player in the game one ESP32-2432S028R is needed, so in total 2.<br>
- battleship_io: This ESP acts as the main game driver and contains all the game logic. 
- battleship_io_client: The second ESP acts as a client and just exectues commands which are coming from the main ESP.

You can use VS code with platformIO or Arduino to compile the code and upload it to the ESPs. Follow the instructions below.

## Using platformIO
- Download the project and open it in platformIO. Intall library dependencies if needed. 
- As the touchscreen driver is (currently) not available in the platformIO library manager, download this library (link above) manually and add it to your "lib" folder of your project.
- Go to your project lib dependency folder (usually `..\Documents\PlatformIO\Projects\battleship_io\.pio\libdeps\esp32dev`) and place the file `lv_conf.h` there.
- Go to your project lib dependency TFT_eSPI folder (`.. \Documents\PlatformIO\Projects\battleship_io\.pio\libdeps\esp32dev\TFT_eSPI`) and place the file `User_Setup.h` there.
- Do this for both project folders battleship_io and battleship_io_client!
- Change the texts in .cpp files according to your language.

## Using Arduino
- Download the project and put it into your Arduino project folder. Rename battleship.cpp and battleship_client.cpp to battleship.ino and battleship_client.ino.
- Open the battleship.ino file and remove the delaration of the methods (e.g. void initializeGame();). For battleship_client.ino there is no adaption needed.
- Use the library manager to download the libraries mentioned above.
- Go to your project lib dependency folder (usually `\Dokumente\Arduino\libraries`) and place the file `lv_conf.h` there.
- Go to your project lib dependency TFT_eSPI folder (`..\Dokumente\Arduino\libraries\TFT_eSPI`) and place the file `User_Setup.h` there.
- Change the texts in .ino and .cpp files according to your language.

## Additional hints
touchscreen calibration
