# teknic_hardware

This package contains a [ros2_control](https://control.ros.org/master/index.html) hardware interface to control Teknic ClearPath-SC servo motors with a `SystemInterface`. It was tested on a CPM-SCSK-2321S-EQNA.

> [!NOTE]
> This interface only works if you have bought the "Advanced" firmware option and not "Basic".

## SC4-Hub USB Driver
For communication with the SC4-Hub you need to install the USB driver.
```bash
cd ExarKernelDriver
sudo ./Install_DRV_SCRIPT.sh
```

Also, add your user to the dialout group so that it can access serial ports.
```bash
sudo usermod -aG dialout $USER
sudo reboot
```

## Motor Setup
The motor should be setup with the ClearView software. The details can be found in the Teknic [User Manual](https://teknic.com/files/downloads/Clearpath-SC%20User%20Manual.pdf). Make sure that the motor is tuned and homing parameters are set up.

An example motor configuration file can be found in [our main repo](https://github.com/OpenFieldAutomation-OFA/ros-weed-control/blob/main/.motor_params/teknic/axis.mtr).

## Interrupting Moves
The hardware interface uses the sFoundation software library and a feature called "interrupting" moves (not in the official documentation) which allows executing moves immediatly instead of storing them in a queue. This feature is the reason why the “Advanced” firmware option is needed.

## Hardware Interface
The following command interfaces are published:
- `position`
- `velocity`

Your `ros2_control` controller can claim either the position or the velocity command interface. Claiming both interfaces at the same time is not possible.

The following state interfaces are published:
- `position`
- `velocity`
- `effort` (if `peak_torque` specified)

The hardware interfaces can also be listed by starting the controller manager and running the following command.
```
ros2 control list_hardware_interfaces
```

## `ros2_control` Parameters
An example `ros2_control` URDF config with this hardware interface can be found in [our main repo](https://github.com/OpenFieldAutomation-OFA/ros-weed-control/blob/main/ofa_moveit_config/ros2_control/ofa_robot.ros2_control.xacro).

`joint` tag:
- `port`: The serial port of the connected SC4-Hub
- `node`: Node number of the motor
- `feed_constant`: Defines the conversion between one revolution of the output shaft to the distance traveled by the linear axis in $\frac{m}{rev}$. This needs to be set for `prismatic` joints and omitted for `revolute` joints.
- `vel_limit`: Velocity limit in $\frac{rad}{s}$ (without `feed_constant`) or $\frac{m}{s}$ (with `feed_constant`). Used for position moves.
- `acc_limit`: Acceleration limit in $\frac{rad}{s^2}$ (without `feed_constant`) or $\frac{m}{s^2}$ (with `feed_constant`). Used for position and velocity moves.
- `homing`: If set to 2, the motor is always homed on activation. If set to 1 the motor is only homed if it has not been homed yet. If set to 0 the motor is never homed.
- `read_only`: OPTIONAL. If set to 1, the motors are disabled after homing and the current position is logged.
- `peak_torque`: OPTIONAL. Peak torque of the motor in Nm. This is necessary if you want the `effort` state interface to work.

It is not possible to disable the trajectory planning on the motor, therefore `vel_limit` and `acc_limit` always have to be specified. When using MoveIt 2 with `joint_trajectory_controller` you should use lower joint limits for motion planning than the limits set here.