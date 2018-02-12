# robin
The ROS Offboard Integration for the Naze32 Rev.5 (or similar Naze32-based board that uses the MPU-6050).

## Preperation
#### Ubuntu
```sh
sudo apt install gcc-arm-none-eabi  stm32flash python3-yaml
mkdir -p ~/src
cd ~/src
git clone --recursive https://github.com/qutas/robin/
```

#### Arch
```sh
sudo apt install arm-none-eabi-gcc arm-none-eabi-newlib python-yaml
```
You'll need to get the `stm32flash` package from the AUR or compile it manually

```sh
mkdir -p ~/src
cd ~/src
git clone --recursive https://github.com/qutas/robin/
```

## Compiling
```sh
cd ~/src/robin
make
```

## Flashing

####NOTE: Some Naze32 models don not allow for flashing to be done via USB, and need to be connected to the UART1 port directly

Before you can flash the the Naze32, you must first put it into bootloader mode. For the initial flash, you can short out the bootloader pins and power on the device.

The makefile assumes that the device is connected as `/dev/ttyUSB0` and will use a baud rate of `921600`. You may have to adjust these to suit your device. If the flash is not successful, try using a slower baud rate.

Once in bootloader mode run the following command:
```sh
make flash
```

After the initial flash, you can use the MAVLINK command `MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN` to put the device into bootloader mode through the software. Additionally, the following command will attempt to send this directly from the CLI (but it may not work for all systems):
```sh
make reflash
```

## Interfacing
The Robin flight software can be interfaced using MAVLINK, and is compatible to much of the specification. Use of the MAVROS software is highly recommended.

#### Attitude/Rates Control
The primary input for controlling the UAV is through the `SET_ATTITUDE_TARGET` message. If using MAVROS, it is recommended that you use the plugin `setpoint_raw` for sending commands. All `type_mask` options are supported for this message, so it is possible to do full attitude control, full rates control, or anything in between.

The `TIMEOUT_OB_CTRL` and `STRM_NUM_OB_H` parameters can be used to set the timeout period and the number of messages needed to establish a stream respectively.

In the event of a stream timing out, the flight controller will enter a failsafe state. In this state, it will attempt to hold it's current attitude (0 rates) and apply the `FAILSAFE_THRTL` parameter as the throttle input.

---
**Note:** Ensure that the `FAILSAFE_THRTL` throttle value will cause the UAV to come to some form of landing. If it is set too high, your UAV may fly off!

---

#### External Heading Estimation
The primary input for providing an external heading estimation is through the `ATT_POS_MOCAP` message. If using mavros, is is recommended to use the plugin `mocap_pose_estimate` for sending an estimated pose to the UAV, from which the external heading estimation will be extracted.

The `TIMEOUT_EXT_P` and `STRM_NUM_EXT_P` parameters can be used to set the timeout period and the number of messages needed to establish a stream respectively.

The `FSE_EXT_HDG_W` parameter can be used to define the weighting of the heading reading to be used in the correction steps. Setting this closer to 0.0 will cause it to be "trusted less", allowing for some deviations, where as setting it closer to 1.0 will cause it to be "trusted more", causing a quicker snap to the estimated heading.

In the event of a stream timing out, the estimator will simply ignore this functionallity until the stream is re-established.

## Calibration
The robin flight software supports the `MAV_CMD_PREFLIGHT_CALIBRATION` command, and will issue instructions through the `STATUSTEXT` messages for the user to follow.

If using MAVROS, you should be able to use the following process to perform a calibration.

#### Gyroscope
To perform a gyroscope calibration:
1. Place the autopilot on a flat surface
2. Do not bump the device then issue the following command:
```sh
rosrun mavros mavcmd long 241 1 0 0 0 0 0 0
```
3. The calibration should only take a few seconds

#### Accelerometer
To perform a accelerometer calibration:
1. Place the autopilot on a flat surface, with the Z-axis pointing down (in the NED frame)
2. Hold the device steady, then issue the following command:
```sh
rosrun mavros mavcmd long 241 0 0 0 0 1 0 0
```
3. Repeat step 2, following the instructions sent back to MAVROS
4. The calibration should only take a few seconds per axis, but it will need to be done for each axis, in both directions

## Parameters
A parameter listing is dynamically generated on compile. During this time, the parameter headers, initializing functions, update funcitons, and a parameter reference document are generated and placed in `lib/param_generator`.

#### Parameter Reference Document
The generated reference document can be [found here](lib/param_generator/PARAMS.md), or at `lib/param_generator/PARAMS.md`. This document contains a listing of the following headings:
- Name: Parameter name as sent through MAVLINK
- Type: The type of parameter this is (uint, int, float)
- Description: A brief description of what this parameter does in the code
- Default: The default value or option for this parameter
- Unit: Information on the parameter unit (if applicable)
- Options: Provides informatiom about the parameter option (list values, min/max, etc.)
- Reboot: Whether the flight controller needs to be rebooted for the parameter to take effect

#### Saving Parameters to the On-Board Flash Memory
To save any parameters that have been changed to make them persist across power downs, they must be saved to the onboard memory. This can be done using the `MAV_CMD_PREFLIGHT_STORAGE` message. In MAVROS:
```sh
rosrun mavros mavcmd long 245 1 0 0 0 0 0 0
```

## Rebooting and Rebooting to Bootloader
The autopilot can be commanded to reboot (when not in flight) using the `MAV_CMD_PREFLIGHT_REBOOT_SHUTDOWN` message, and optionally, can be put into bootloader mode for flashing.

#### Reboot
In MAVROS:
```sh
rosrun mavros mavcmd long 246 1 0 0 0 0 0 0
```

#### Reboot to Bootloader
In MAVROS:
```sh
rosrun mavros mavcmd long 246 3 0 0 0 0 0 0
```







