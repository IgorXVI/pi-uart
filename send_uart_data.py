import serial

message = b"-"

ser = serial.Serial("/dev/ttyUSB0")
print(ser.name)
ser.write(message)
ser.close()