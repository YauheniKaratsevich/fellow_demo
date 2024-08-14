## Description

    This  demo code  is a  simple  thermostat that  controls a  heater based  on
temperature sensor  data. The  thermostat has  a CLI  interface for  setting the
setpoint temperature and hysteresis. The core  of the firmware is the thermostat
task, which  includes a cli  command parser and  a primitive state  machine. The
thermostat  can be  turned  on and  off,  when  it is  off,  the temperature  is
monitored  (state  MONITOR). When  it  is  on,  the  heater is  controlled.  The
thermostat  state  has the  states  WARM_UP,  COLD_DOWN  and MONITOR.  When  the
thermostat is  on and  the state  is WARM_UP, the  heater will  be on  until the
current temperature  is more than the  setpoint plus the hysteresis  value. When
the temperature is  greater than the setpoint+hysteresis, the  state will change
to COLD_DOWN, and the heater will be  off until the temperature is less than the
setpoint  minus the  hysteresis value.  When the  temperature is  less than  the
setpoint-hysteresis, the state will change to WARM_UP and so on.
    Using  cli commands,  the user  can set  the septoint  value and  hysteresis
value, as well as turn the thermostat on/off.

## How to build
    This code was built using the  official espressif Docker image, running from
the project directory:

```
 docker run --rm -v $PWD:/project -w /project espressif/idf:release-v4.4 idf.py -DIDF_TARGET=esp32s3 build
```

## Disclaimer
    The code has not been tested on real hardware. This is just a demo project.
