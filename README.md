# Greenhouse Display unit

Display unit for greenhouse weather station. <br>
Following data are displayed:
- Temperature inside
- Humidity inside
- Temperature outside
- Atmospheric pressure
- Light instensity
- Battery voltage
- Battery current
- Battery power

Measured data are received via I2C bus from DAQ Arduino<br>
Unit has min/max EERPOM memory for displayed parameters.

### Arduino Nano
**Tweaks:**
- removed PWR LED
- removed voltage regulator
- 3.3 V supply voltage
- LoWPower library

## Display:
### Waveshare 2.9" 296x128 e-Paper display
-
