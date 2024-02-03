Start by running ./motortest 

It will try to find the motor id by scanning the RS485 and should look like this:
```
==========================================================
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=0 does not reply
ERROR motor_id#0
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=1 does not reply
ERROR motor_id#1
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=2 does not reply
ERROR motor_id#2
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=3 does not reply
ERROR motor_id#3
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=4 does not reply
ERROR motor_id#4
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=5 does not reply
ERROR motor_id#5
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=6 does not reply
ERROR motor_id#6
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=7 does not reply
ERROR motor_id#7
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=8 does not reply
ERROR motor_id#8
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=9 does not reply
ERROR motor_id#9
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=10 does not reply
ERROR motor_id#10
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=11 does not reply
ERROR motor_id#11
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=12 does not reply
ERROR motor_id#12
[WARNING] SerialPort::recv, unblock version, wait time out
[WARNING] motor id=13 does not reply
ERROR motor_id#13


FOUND MOTOR: 14
Degrees: -84.287717
Temp:    23
degrees: -64.300548        
```

If successful you can run it with an extra position argument like this:
```
./motortest -20
```
This should slowly move the motor from -64 (see above) to -20. The code switches the input to non-canonical mode so any press on the keyboard will stop the motor and abort the program.

I also included the bin directory from the unitree sdk. I made the code try to discover the ID instead of just blindly changing it with (doesn't always work):
```
./bin/changeid /dev/ttyUSB0 15 0
```
This is the broadcast version of changeid and will change the motorid of any motor on the bus to 0. I would recommend leaving the motor id unchanged in case you want to put your dog back together again.

After running changid the status light will blink three times rapidly because the motor is in bootloader mode, you have to run to switch back to motor mode:
```
./bin/swmotor /dev/ttyUSB0
```
