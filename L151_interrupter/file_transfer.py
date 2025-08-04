import serial
import sys
import os


'''
s = "hello world"
s = s[:-1]
print(s)

exit(0)
'''

C_LS   = bytearray(['@', '@'])

C_DONE = bytearray(['d', 'd'])

#open a serial port with the microcontroller
ser = serial.Serial("COMx", 115200, serial.EIGHTBITS, serial.PARITY_NONE, serial.STOPBITS_ONE)

#256 byte packets
#first 252 bytes data
#last 4 bytes checksum

print("Welcome to the STM32 Tesla Coil Interrupter File Controller")

while(1):
    print("Choose an action:")
    print("  1. List directory contents from SD card")
    print("  2. Delete a file")
    print("  3. Send a file")
    print("  4. Exit")
    choice = input()
    #filePath = sys.argv[1]
    #fileSize = os.stat(filePath).st_size\
    match choice:
        
        #List directory contents
        case 1: 
            #Send ls command
            ser.write(C_LS)
            
            #read lines from stm32 until end sequence detected
            #make sure to add newline character to end of string in STM32
            lineRead = ser.readline()
            while(lineRead[:-1] != "end of files"):
                print(lineRead)
                lineRead = ser.readline()

            



        #Delete file from SD card
        case 2: 
            fileToDelete = input("Enter name of file to delete (exact name)\n")


        #Send a file to the SD card
        case 3:
            pathToSend = input("Enter path of file to send\n")

        #Close terminal and exit
        case 4:
            ser.write(C_DONE)
            exit(0)
        case _:
            print("No valid input detected.\n\n")


    input("Press enter to continue")