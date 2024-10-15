import py_midicsv as pm 
#import serial
import time as bruh
from tkinter import filedialog as fd

#"C:\Users\bucht\OneDrive\Desktop\Tesla Coil\my_midi_list\99 red balloons.mid"
#midiPath = "C:\\Users\\bucht\\OneDrive - Worcester Polytechnic Institute (wpi.edu)\\Projects\\Tesla Coil\\5 coils\\5 coil midi\\Dueling Banjos.mid"
#csv_string = pm.midi_to_csv("C:\\Users\\bucht\\OneDrive - Worcester Polytechnic Institute (wpi.edu)\\Projects\\Tesla Coil\\5 coils\\5 coil midi\\Dueling Banjos.mid")

midiPath = fd.askopenfilename()
csv_string = pm.midi_to_csv(midiPath)

#ser = serial.Serial('COM3', 115200)

#for string in csv_string:
#	print(string)

header = csv_string[0]
header = header.split(',')
tpqn = int(header[-1]) #ticks per quarter note
numTracks = int(header[4])

line = ''
i = 0
while not "Tempo" in line:
	line = csv_string[i]
	i= i + 1

tempo = int(line.split(',')[-1])
bpm=60e6/tempo
msPerTick = 60000/(bpm*tpqn)

while not "Note_on_c" in line:
	line = csv_string[i]
	i = i + 1


i = i - 1
#i is now at the first line of events in the file
#print(line)

#track, time, note_on_c (or note_off_c), channel, note, velocity
#for the purposes of this file, coils will be organized into tracks

listOfListOfEvents = []
for lineNum in range(i, len(csv_string)):
	line = csv_string[lineNum]
	listOfListOfEvents.append(line.split(','))
	#print(line.split(','))

#sort events by time
listOfListOfEvents = sorted(listOfListOfEvents, key=lambda listOfListOfEvents:int(listOfListOfEvents[1]))

'''for i in range(0, len(listOfListOfEvents)):
	if not "Note" in listOfListOfEvents[i][2]:
		del(listOfListOfEvents[i])'''
onlyNotes = []
for arr in listOfListOfEvents:
	string = arr[2]
	if "Note" in string:
		onlyNotes.append(arr)

#finalList is human readable
finalList = []
for arr in onlyNotes:
	track = int(arr[0])
	time = int(int(arr[1]) * msPerTick)
	note = int(arr[4]) - 12 #lowest note is C0
	velocity = int(arr[5])
	event = [track, time, note, velocity]

	#no songs longer than 3 minutes
	if(time < 180000):
		finalList.append(event)

print(finalList[0])
print(finalList[1])
#for event in finalList:
#	print(event)

### SERIAL FORMAT ###
#	   3 bytes for time
#track, t0, t1, t2, note, vel

#list of byte arrays is to be sent to output file
listOfByteArrays=[]
for arr in finalList:
	print(arr)
	track = arr[0]
	t0 = arr[1]  & 0xFF
	t1 = (arr[1] & 0xFF00) >> 8
	t2 = (arr[1] & 0xFF0000) >> 16
	note = arr[2] #offset for mcu
	vel = arr[3]
	bruhArray = bytearray([track, t0, t1, t2, note, vel])
	listOfByteArrays.append(bruhArray)

outPath = midiPath[0:-4] + ".will" #.will file
newFile = open(outPath, "wb")

##################################################################
#now finally write to the file
#first two bytes are number of events
numEvents = len(listOfByteArrays)
newFile.write(bytearray([numEvents&0xff, (numEvents&0xff00)>>8]))
songLength = round(finalList[-1][1]/1000)
####################################################
#delete this if it is fucked up after
#newFile.write(bytearray([songLength&0xFF]))
################################################

for arr in listOfByteArrays:
	newFile.write(arr)

#denote end of file by 0xFF
newFile.write(bytearray([255]))
newFile.close()




'''while eventPointer < len(finalList):
	eventTime = finalList[eventPointer][1]
	timeElapsed = (bruh.time()-timeStarted)*1000
	if timeElapsed >= eventTime:
		#if(finalList[eventPointer][0] == 1):
		#ser.write(bytearray([finalList[eventPointer][0],finalList[eventPointer][2],finalList[eventPointer][3]]))
		print([finalList[eventPointer][0],finalList[eventPointer][2],finalList[eventPointer][3], eventTime])
		eventPointer = eventPointer + 1'''
		



#ser.write(bytearray([0, ]))

'''time = 0
pointer = 0
while True:
	track = 
	time.sleep(0.010)
	time = time+10'''
	
#ser.write(finalList[0][2])
#print(finalList[0][2])

print("\nms per tick: .......... {}".format(msPerTick))
print("Number of tracks: ..... {}".format(numTracks))
print("Ticks per quarter Note: {}".format(tpqn))
print("Song length: .......... {}s".format(songLength))
print(f"Number of events: {numEvents}")
print(f"Wrote to {outPath}")
