import serial

message = b"Igor de Almeida\n"

ser = serial.Serial("/dev/ttyUSB0", timeout=30)
print(ser.name)
ser.write(message)
ser.close()