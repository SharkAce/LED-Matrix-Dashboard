# LED Matrix Dashboard

## Prerequisites

To run this project, you will need:

- A Raspberry Pi
- An RGB LED matrix panel of size 32x64

The panel needs to be connected to the Pi GPIOs according to [this schema](https://github.com/hzeller/rpi-rgb-led-matrix/blob/a3eea997a9254b83ab2de97ae80d83588f696387/wiring.md#connections-per-chain).

## Preview

[![dashboard preview](https://tinypic.host/images/2024/08/03/dashboard-preview.md.jpg)](https://tinypic.host/image/dashboard-preview.D6eWfx)

## Installation

To install the `matrix-dashboard` service, follow these steps:

1. **Clone the repository with submodules:**

    ```bash
    git clone --recurse-submodules https://github.com/SharkAce/LED-Matrix-Dashboard.git
    ```

2. **Build the project:**

    ```bash
    cd LED-Matrix-Dashboard
    make
    ```

3. **Install the service:**

    ```bash
    sudo make install
    ```

## Usage

Once installed, you can either start the service or run the executable in the current shell.

- **Start the service:**

    ```bash
    sudo systemctl start matrix-dashboard.service
    ```

- **Run the executable:**

    ```bash
    matrix-dashboard
    ```
    
## Configuration

The configuration is defined in JSON format and specifies the matrix options and the blocks to be displayed. After installation, the configuration file will be located at `/etc/matrix-dashboard/config.json`. A custom configuration file can also be specified using the `-c` flag. 

Below is a detailed explanation of each parameter.

*Properties are required unless specified otherwise.*

### `matrix`
- **Description**: Contains configuration options for the `rgbmatrix` library. Additional options can be specified by appending flags to the execution command.
- **Structure**:
  - `size` (object, optional): Specifies the size of the matrix.
    - `rows` (integer): Number of rows in the matrix.
    - `cols` (integer): Number of columns in the matrix.
  - `brightness` (integer, optional): Brightness level of the display.
  - `font` (string): Font file to be used.
  - `limit_refresh_rate_hz` (integer, optional): Refresh rate limit in Hertz.
  - `led_rgb_sequence` (string, optional): RGB sequence. Example: "RGB".

### `blocks`
- **Description**: An array of block objects that define each element displayed on the screen.
- **Structure**:
  - `type` (string): Specifies the type of block. Valid options are `"time"` and `"http"`.
  - `origin` (string, optional): Specifies the origin point for positioning the block. Can be `"top-left"` (default), `"top-right"`, `"bottom-left"`, or `"bottom-right"`.
  - `position` (object): Specifies the x and y coordinates for the block's position on the screen.
    - `x` (integer): The horizontal position on the screen.
    - `y` (integer): The vertical position on the screen.
  - `color` (array of three integers): Defines the RGB color of the block text. Example: `[64, 0, 128]` represents a purple color.
  - `interval` (integer): The time interval in seconds at which the block content is updated. For example, `600` means the block will update every 10 minutes.

### `time` Blocks:
- **Description**: Display the current date and/or time.
- **Additional Properties**:
  - `format` (string): Specifies how the date/time is formatted. The format follows the `strftime` syntax, such as `%A` for the full weekday name, or `%H:%M:%S` for hours, minutes, and seconds.
  - `rounding` (object, optional): Specifies rounding intervals for hours, minutes, and seconds.
    - `seconds` (integer, optional): Rounds the seconds to the nearest multiple of the specified value. Example: `{"seconds": 5}` will round the seconds to `0`, `5`, `10`, etc.
    - `minutes` (integer, optional): Rounds the minutes similarly to the nearest multiple.
    - `hours` (integer, optional): Rounds the hours to the nearest multiple.

### `http` Blocks:
- **Description**: Display data fetched from a specified URL.
- **Additional Properties**:
  - `url` (string): The URL from which data will be fetched.

### Example config
```json
{
  "matrix": {
    "size": {"rows": 32, "cols": 64},
    "brightness": 50,
    "font": "4x6.bdf",
    "limit_refresh_rate_hz": 120,
    "led_rgb_sequence": "RBG"
  },
  "blocks": [
    {
      "type": "time",
      "format": "%H:%M:%S",
      "rounding": {"seconds": 5},
      "position": {"x": 2, "y": 10},
      "color": [64, 0, 128],
      "interval": 5
    },
    {
      "type": "http",
      "url": "https://wttr.in?format=%t",
      "origin": "top-right",
      "position": {"x": 2, "y": 10},
      "color": [0, 128, 128],
      "interval": 600
    }
  ]
}
```

## Troubleshooting

On a default Raspbian setup, the `snd_bcm2835` module needs to be blacklisted to use the hardware pulse feature. See the [troubleshooting guide](https://github.com/hzeller/rpi-rgb-led-matrix/tree/a3eea997a9254b83ab2de97ae80d83588f696387?tab=readme-ov-file#bad-interaction-with-sound) for the solution.

## Credit

This project uses the `rgbmatrix` library and wiring schema from [hzeller/rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix/tree/a3eea997a9254b83ab2de97ae80d83588f696387).
