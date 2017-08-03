# Paint Matrix

Use a joystick to paint a color on the [8x8 RGB Matrix](http://wiki.52pi.com/index.php/RPI-RGB-LED-Matrix_SKU:EP-0075) with ADC input, and you can change a color to cover on the previous canvas by pressing the button.

## Requirements

Configure the i2c and SPI on Pi.
```bash
sudo raspi-config nonint do_spi 0
sudo raspi-config nonint do_i2c 0
```

## Compile
```bash
gcc matrix.c -o matrix -lwiringPi -lm -O3
```

If nothing wrong happens, then the canvas is ready to run.
