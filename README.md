# MBDC
Multiband Direct Conversion Receiver

This is the firmware and design files for a multiband direct conversion receiver for amateur bands. Bands covered are 80M, 40M, 20M, 15M, and 10M.

# Build Environment by Earle F. Philhower, III

Built using the Pi Pico arduino environment by Earle F. Philhower, III. See install details at:

https://github.com/earlephilhower/arduino-pico]


Here is the "Additional Boards Manager URL" for quick start:

https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

# Libraries Used
The following libraries are needed to work with the MS5351M and the analog inputs:

https://github.com/etherkit/Si5351Arduino

https://github.com/dxinteractive/ResponsiveAnalogRead

Note, although not necessary, I modified the Si5351 library to remove the c++ *new* and *delete* operators and replaced them with a stack based array.
```
uint8_t *params = new uint8_t[20];
```
was replaced with:
```
uint8_t params[20];
```
And removed:
```
delete params;
```

# Main Tuning


# Band Change and Fine Tuning


# Frequency Display


73, VK7IAN
