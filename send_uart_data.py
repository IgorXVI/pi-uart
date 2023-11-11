import serial

ser = serial.Serial("/dev/ttyUSB0", baudrate=9600, parity="N")

try:
    while True:
        user_message = input("Your message: ")

        if(user_message == "~"):
            print("Erasing previous messages...")
            ser.write(b"~")
            continue

        print(f"Sending message: {user_message}")

        ascii_bytes = (user_message + "\n").encode("ascii")

        ser.write(ascii_bytes)

        print("Message was sent!")

except:
    print("Closing connection...")
    ser.close()