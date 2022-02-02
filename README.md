# MBDC
Multiband Direct Conversion Receiver

This is the firmware and design files for a multiband direct conversion receiver for amateur bands. Bands covered are 80M, 40M, 20M, 15M, and 10M.

# Build Environment

Built using the Pi Pico arduino environment by Earle F. Philhower, III. See install details at:

https://github.com/earlephilhower/arduino-pico]


Here is the "Additional Boards Manager URL" for quick start:

https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json

# Libraries Used
The following libraries are needed to work with the MS5351M and the analog inputs:

https://github.com/etherkit/Si5351Arduino

https://github.com/dxinteractive/ResponsiveAnalogRead

Note, although not strickly necessary, I modified the Si5351 library to remove the c++ *new* and *delete* operators and replaced them with a stack based array. I think it is good practice to avoid calling dynamic memory allocation functions in embedded systems wherever possible.
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
The main tuning uses a 10 turn pot on an analog input. To smooth out the natural variations an average of 256 samples is taken before being passed to the *Resposive Analog Read* function.


# Band Change and Fine Tuning
The band change and fine tuning analog inputs are reduced to 8 bits and passed to the *Resposive Analog Read* function. The band change value is mapped to a value in the range 0 - 11 using a mapping array. This was done because the band potentiometer has 11 detents that do not produce an exact linear response.

The fine tune value of 0 - 255 is modified to a value of -100 to +100 with a 20 point gap in the centre. The first frequency value is then calculated as follows:
```
frequency = bands[band].start + bands[band].step * mainTune.getValue();
```
The bands[] arry contains the base frequency and step value for the band selected. The final frequency is then:
```
frequency = (frequency-(frequency % 1000)) + fine_tune * 5;
```
Thus the frequency is converted to a whole khz value before the fine tune value (which has a range of -500 to +500) is added. This makes it easier to tune in SSB stations that will usually be on whole khz frequency.

# Frequency Display


73, VK7IAN
