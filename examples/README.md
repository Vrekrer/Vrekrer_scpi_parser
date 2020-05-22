# SCPI Stepper motor controller
Simple SCPI Stepper motor controller using Vrekrer scpi parser.
I needed to controll 3 motors with linear stages with very crude coding skill.
So I made different command set for each axis - I will modify that later. 

This is the list of commands. 

## *IDN? 
Device identification. 

## MOTOr:XMOVe STEP\#
Revolve X axis (axis 1) step motor with designated step numer. 
## MOTOr:XLOCation STEP\# 
Set X axis (axis 1) location in step unit.
## MOTOr:XLOCation? 
Get X axis (axis 1) location in step unit.
## MOTOr:XSPEed SPEED\#
Set X axis (axis 1) location in RPM unit.
## MOTOr:XSPEed?
Get X axis (axis 1) location in RPM unit.

## MOTOr:YMOVe STEP\#
Revolve Y axis (axis 1) step motor with designated step numer. 
## MOTOr:YLOCation STEP\# 
Set Y axis (axis 1) location in step unit.
## MOTOr:YLOCation? 
Get Y axis (axis 1) location in step unit.
## MOTOr:YSPEed SPEED\#
Set Y axis (axis 1) location in RPM unit.
## MOTOr:YSPEed?
Get Y axis (axis 1) location in RPM unit.

## MOTOr:ZMOVe STEP\#
Revolve Z axis (axis 1) step motor with designated step numer. 
## MOTOr:ZLOCation STEP\# 
Set Z axis (axis 1) location in step unit.
## MOTOr:ZLOCation? 
Get Z axis (axis 1) location in step unit.
## MOTOr:ZSPEed SPEED\#
Set Z axis (axis 1) location in RPM unit.
## MOTOr:ZSPEed?
Get Z axis (axis 1) location in RPM unit.
