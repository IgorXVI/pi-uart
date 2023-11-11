import serial
from threading import Thread
import traceback

ser = serial.Serial("/dev/ttyUSB0", baudrate=9600, parity="N")

received_messages = [""]

def receive_messages():
    try:
        print("Listening for messages...")

        while True:
            received_message = ser.read_until(expected=b"\n")

            formated = str(received_message, encoding="ascii")

            received_messages.append(formated)

    except TypeError:
        pass

    except:
        print("\nError when trying to read messages:")

        traceback.print_exc()


t = Thread(target=receive_messages)

try:
    t.start()

    while True:
        user_message = input("Your message: ")

        if user_message == "??":
            print("Printing received messages...")

            formated = "\n".join(received_messages)

            print(formated)

            continue

        if user_message == "~":
            print("Erasing previous messages...")

            ser.write(b"~")

            continue

        ascii_bytes = (user_message + "\n").encode("ascii")

        ser.write(ascii_bytes)

        print("Message was sent!")

except KeyboardInterrupt:
    print("\n")

except:
    print("\nError when trying to write messages:")

    traceback.print_exc()

finally:
    print("Closing connection...")

    ser.close()