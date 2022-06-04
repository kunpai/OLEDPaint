# OLED Paint

## An application of Paint on the Adafruit OLED

- Uses I2C to determine the accelerations of the tilts of the board which helps move the cursor, brush and eraser
- Uses IR remote detection to comprehend button presses from 0-9, VOL+/- and CHAN+/-, which are configured to be the features.
- Uses SimpleLink WiFi to connect to a custom backend server made using Flask, that interprets a 128px by 128px bitmap received from the CC3200 and converts it into an SVG and PNG.
- Displays converted PNG on a matplotlib instance, resets on closing the generated image.

## Features:

0 : Screenshot <br />
1 : Cursor <br />
2 : Brush <br />
3 : Eraser <br />
4 : Line Generator (from 2 points) <br />
5 : Triangle Generator (from 3 points) <br />
6 : Circle Generator <br />
7 : Quadrilateral Generator (from 4 points) <br />
8 : Fill Screen with Cursor Color <br />
9 : Reset to Black Canvas <br />
<br />
Volume + : Increase Brush Size <br />
Volume - : Decrease Brush Size <br />
<br />
Channel + : Move Forward in the Color Palette <br />
Channel - : Move Back in the Color Palette <br />
