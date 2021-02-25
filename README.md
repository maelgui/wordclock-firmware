# Firmare for Wordclock V2

## Dependencies
- cmake
- avr-gcc
- avrdude

## Prepare build environment
```
mkdir build
cd build
cmake ..
```

## Build
```
cd build
make
```

## Flash the firmware
On your device, turn the switch on USB position
```
cd build
make flash
```
