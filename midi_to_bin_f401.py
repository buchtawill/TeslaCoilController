import os
import sys
import py_midicsv as pm 


SUPPORTED_EVENTS = {
    'Note_on_c',
    'Note_off_c'
    # 'Pitch_bend_c'
}

# Midi spec
EVENT_TO_STATUS = {
    'Note_on_c'    : 0x90,
    'Note_off_c'   : 0x80,
    'Pitch_bend_c' : 0xE0
}

def event_to_byte_array(event:dict, prev_time:int):
    """
    Convert a midi event to a byte array, fully packed
    
    Format:
        4 bytes, delta time in ms from previous event
        status byte (event, track)
        optional db1/db2
    """
    dt = event['time'] - prev_time
    dt_bytes = dt.to_bytes(4, byteorder="little", signed=False)
    
    # Extract note on, note off, pitch bend 
    # https://github.com/timwedde/py_midicsv/blob/master/py_midicsv/midi_converters.py
    # Each CSV string looks like: 
	# track, time, note_on/off_c, channel, note, velocity
    # track, time, Pitch_bend_c, channel, 14-bit pitch 
    
    # Convert midi events to byte packets
    # 4 bytes: delta time from prev event
    # next byte: midi status byte
    # optional db1/db2
    
    if(event['type'] in SUPPORTED_EVENTS):
        status = EVENT_TO_STATUS[event['type']]
        status += event['track']
        status = status.to_bytes(1, signed=False)
        db1 = event['db1'].to_bytes(1, signed=False)
        db2 = event['db2'].to_bytes(1, signed=False)
        
        packet = b''.join([dt_bytes, status, db1, db2])
        
    else:
        raise Exception(f"ERROR [event_to_byte_array] Unsupported event type: {event['type']}")
    
    return packet, dt

def midi_2_bin(midi_lines:str, out_path:str):
    """
    
    """
    header = csv_str[0].split(',')
    # Ticks per quarter note
    tpqn = int(header[-1])
    num_tracks = int(header[4])
    
    # Skip until we find Tempo
    line = ''
    i = 0
    while ("Tempo" not in line):
        line = csv_str[i]
        i += 1
    
    # https://stackoverflow.com/questions/2038313/converting-midi-ticks-to-actual-playback-seconds
    tempo = int(line.split(',')[-1])
    bpm = 60e6 / tempo
    ms_per_tick = 60000 / (bpm * tpqn)
    
    # Skip to the first on event
    while "Note_on_c" not in csv_str[i]:
        i += 1
    
    # Clean up into a new array
    list_of_events = []
    for linenum in range(i, len(csv_str)):
        event = {}
        event_line = csv_str[linenum].split(',')
        event['type']    = event_line[2].strip()
        
        if(event['type'] in SUPPORTED_EVENTS):
            
            # track, time, Pitch_bend_c, channel, 14-bit pitch 
            # track, time, note_on/off_c, channel, note, velocity
            event['track']   = int(event_line[0])
            if(event['track'] > 15):
                raise Exception(f"ERROR [midi_2_bin] Only 0-15 tracks accepted (track: {event['track']})")
            event['time']    = int(int(event_line[1]) * ms_per_tick)
            event['channel'] = int(event_line[3])
            event['db1'] = int(event_line[4])
            event['db2'] = int(event_line[5])
            
            list_of_events.append(event)
        
        if(event['type'] == 'Pitch_bend_c'):
            raise Exception("Pitch bend not yet supported")

    # Sort events by time (different channels appear as whole chunks)
    list_of_events = sorted(list_of_events, key=lambda e:e['time'])

    num_events = len(list_of_events)
    runtime = list_of_events[-1]['time']
    
    with open(out_path, 'wb') as f:
        f.write(num_events.to_bytes(4, 'little', signed=False))
        f.write(runtime.to_bytes(4, 'little', signed=False))
    
        prev_time = 0
        sum_dt = 0
        for event in list_of_events:
            packet, dt = event_to_byte_array(event, prev_time)
            prev_time = event['time']
            sum_dt += dt
            
            f.write(packet)
        
        # End of song
        f.write(0xF0.to_bytes(1, signed=False))
        
    # Temporary
    # with open("temp_99.txt", 'w') as out:
    #     with open(out_path, 'rb') as f:
            
    #         out.write(f'uint8_t luftballoons[{os.path.getsize(out_path)}] = {{\n')
            
    #         while(data := f.read(1)):
    #             out.write(f'    0x{data[0]:X},\n')
    #     out.write("}")
                    
    print(f"INFO [midi_2_bin] Number of Events: {num_events}")
    print(f"INFO [midi_2_bin] Song length: {(runtime/1000):0.3f} sec")
    print(f"INFO [midi_2_bin] ms per tick: {ms_per_tick}")

if __name__ == '__main__':
    
    if(len(sys.argv) < 2):
        print("Usage: python midi_to_bin_f401.py <midi file or directory of midi files>")
        exit(1)
        
    if(os.path.isdir(sys.argv[1])):
        basedir = os.path.basename(sys.argv[1])
        for file in os.listdir(sys.argv[1]):
            if(os.path.splitext(file)[1] == '.mid'):
                print(f"INFO [midi_to_bin_f401] Processing {os.path.basename(file)}")
                
                in_path = os.path.realpath(os.path.join(sys.argv[1], file))
                filename = os.path.splitext(os.path.basename(in_path))[0]
                out_path = os.path.join(os.path.dirname(in_path), filename + '.bin')
                try:
                    csv_str = pm.midi_to_csv(in_path)
                    midi_2_bin(csv_str, out_path)
                except Exception as e:
                    print("Encountered exception, ignoring...")
                
    # A file name was given on the command line        
    else:
        if(not os.path.exists(sys.argv[1])):
            print(f"ERROR Cannot find {sys.argv[1]}")
        in_path = os.path.realpath(sys.argv[1])
        filename = os.path.splitext(os.path.basename(in_path))[0]
        out_path = os.path.join(os.path.dirname(in_path), filename + '.bin')    
            
        print(f"INFO [midi_to_bin_f401] Processing '{filename}'")
        # A list of strings, each string is a line of csv
        csv_str = pm.midi_to_csv(in_path)
        midi_2_bin(csv_str, out_path)
    