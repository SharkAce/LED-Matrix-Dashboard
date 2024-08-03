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

## Troubleshooting

On a default Raspbian setup, the `snd_bcm2835` module needs to be blacklisted to use the hardware pulse feature. See the [troubleshooting guide](https://github.com/hzeller/rpi-rgb-led-matrix/tree/a3eea997a9254b83ab2de97ae80d83588f696387?tab=readme-ov-file#bad-interaction-with-sound) for the solution.

## Credit

This project uses the `rgbmatrix` library and wiring schema from [hzeller/rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix/tree/a3eea997a9254b83ab2de97ae80d83588f696387).
