# captive portal on an ESP32 C1

![IMG_20260326_163038_308.jpg](https://github.com/user-attachments/assets/262f062a-7a67-43ac-a6e9-22671c049136)


Since the HTML, CSS and JS files are on a data folder and need to be accessed, a filesystem needs to be created.
Upload the firmware as follows: 

```
~/.platformio/penv/bin/pio run --target upload && ~/.platformio/penv/bin/pio run --target uploadfs
```

