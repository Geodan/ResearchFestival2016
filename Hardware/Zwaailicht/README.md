Alarm light that can be turned on or off over internet using a NodeMCU V3 (LoLin) and a relais-module

# Turn the light on-off
---
### MQTT Topic
GOST/Datastreams(35)/Observations

### HTTP POST
http://gost.geodan.nl/v1.0/Datastreams(35)/Observations

### Payload to turn on the light
```{ "result": true }```

### Payload to turn off the light
```{ "result": false }```