import serial

ser = serial.Serial("/dev/ttyUSB0", baudrate=9600, parity="N")

message = b"~Igor de Almeida"

print(message)

ser.write(message)

ser.close()