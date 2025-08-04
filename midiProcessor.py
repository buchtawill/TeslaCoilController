import py_midicsv as pm 
#import serial
import time as bruh
from tkinter import filedialog as fd

#"C:\Users\bucht\OneDrive\Desktop\Tesla Coil\my_midi_list\99 red balloons.mid"
#midiPath = "C:\\Users\\bucht\\OneDrive - Worcester Polytechnic Institute (wpi.edu)\\Projects\\Tesla Coil\\5 coils\\5 coil midi\\Dueling Banjos.mid"
#csv_string = pm.midi_to_csv("C:\\Users\\bucht\\OneDrive - Worcester Polytechnic Institute (wpi.edu)\\Projects\\Tesla Coil\\5 coils\\5 coil midi\\Dueling Banjos.mid")

if __name__ == '__main__':
	midiPath = fd.askopenfilename()
	csv_string = pm.midi_to_csv(midiPath)

	# CSV looks like:
	# 2, 864, Note_on_c, 0, 35, 100
	# 2, 864, Note_on_c, 0, 76, 100
	# 2, 911, Note_off_c, 0, 76, 48
	# 2, 923, Note_off_c, 0, 35, 48	
 
	# Read the header
	header = csv_string[0]
	header = header.split(',')
	tpqn = int(header[-1]) #ticks per quarter note
	numTracks = int(header[4])

	# Find the tempo
	line = ''
	i = 0
	while not "Tempo" in line:
		line = csv_string[i]
		i= i + 1

	tempo = int(line.split(',')[-1])
	bpm=60e6/tempo
	msPerTick = 60000/(bpm*tpqn)

	# Find the first note_on_c event
	while not "Note_on_c" in line:
		line = csv_string[i]
		i = i + 1

	i = i - 1
	#i is now at the first line of events in the file

	# Each CSV string looks like: 
	# track, time, note_on_c (or note_off_c), channel, note, velocity
	# Coils will be organized into tracks

	listOfListOfEvents = []
	for lineNum in range(i, len(csv_string)):
		line = csv_string[lineNum]
		listOfListOfEvents.append(line.split(','))
		#print(line.split(','))

	# Sort events by time
	listOfListOfEvents = sorted(listOfListOfEvents, key=lambda listOfListOfEvents:int(listOfListOfEvents[1]))
 
	# Now loop thru list of events, extract relevant information (only note events)
	onlyNotes = []
	for arr in listOfListOfEvents:
		string = arr[2]
		if "Note" in string:
			onlyNotes.append(arr)

	# finalList is human readable
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


	### SERIAL FORMAT ###
	#	   3 bytes for time
	# track, {t0, t1, t2}, note, vel

	# list of byte arrays is to be sent to output file
	listOfByteArrays=[]
	for arr in finalList:
		print(arr)
		track = arr[0]
		t0 = arr[1]  & 0xFF
		t1 = (arr[1] & 0xFF00) >> 8
		t2 = (arr[1] & 0xFF0000) >> 16
		note = arr[2]
		vel = arr[3]
		bruhArray = bytearray([track, t0, t1, t2, note, vel])
		listOfByteArrays.append(bruhArray)

	outPath = midiPath[0:-4] + ".will" #.will file
	newFile = open(outPath, "wb")

	##################################################################
	# now finally write to the file
	# first two bytes are number of events
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

	print()
	print("ms per tick: .......... {}".format(msPerTick))
	print("Number of tracks: ..... {}".format(numTracks))
	print("Ticks per quarter Note: {}".format(tpqn))
	print("Song length: .......... {}s".format(songLength))
	print(f"Number of events: {numEvents}")
	print(f"Wrote to {outPath}")
