import serial

message = 0b11111111

ser = serial.Serial("/dev/ttyUSB0", bytesize=8)
print(ser.name)
ser.write(message)
ser.close()