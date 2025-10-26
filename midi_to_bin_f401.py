import os
import sys
import py_midicsv as pm 
import json
import logging

logging.basicConfig(filename='midi2bin.log', 
                        level=logging.INFO)
LOGGER = logging.getLogger(__name__)

NOTE_EVENTS = {
    'Note_on_c',
    'Note_off_c'
    # 'Pitch_bend_c'
}

SUPPORTED_EVENTS = NOTE_EVENTS.copy()
SUPPORTED_EVENTS.add(
    'Tempo'
)

# Midi spec
EVENT_TO_STATUS = {
    'Note_on_c'    : 0x90,
    'Note_off_c'   : 0x80,
    'Pitch_bend_c' : 0xE0
}

def event_to_byte_array(event:dict):
    """
    Convert a midi event to a byte array, fully packed
    
    Format:
        4 bytes, delta time in ms from previous event
        status byte (event, track)
        optional db1/db2
    """
    
    # Extract note on, note off, pitch bend 
    # https://github.com/timwedde/py_midicsv/blob/master/py_midicsv/midi_converters.py
    # Each CSV string looks like: 
	# track, time, note_on/off_c, channel, note, velocity
    # track, time, Pitch_bend_c, channel, 14-bit pitch 
    
    # Convert midi events to byte packets
    # 4 bytes: time in ms of this event
    # next byte: midi status byte
    # optional db1/db2
    if(event['type'] in NOTE_EVENTS):
        status = EVENT_TO_STATUS[event['type']]
        status += event['track']
        status = status.to_bytes(1, signed=False)
        db1 = event['db1'].to_bytes(1, signed=False)
        db2 = event['db2'].to_bytes(1, signed=False)
        
        timestamp = event['time_ms'].to_bytes(4, byteorder="little", signed=False)
        packet = b''.join([timestamp, status, db1, db2])
        return packet
        
    elif(event['type'] not in SUPPORTED_EVENTS):
        raise Exception(f"ERROR [event_to_byte_array] Unsupported event type: {event['type']}")

def process_midi_csv_header(midi_lines:list):
    '''
    Given a list of CSV lines, remove the header and calculate the first ms_per_tick
    value from the tempo.
    '''
    header = midi_lines[0].split(',')
    # Ticks per quarter note
    tpqn = int(header[-1])
    num_tracks = int(header[4])
    
    # Skip until we find Tempo
    line = ''
    i = 0
    while ("Tempo" not in line):
        line = midi_lines[i]
        i += 1
    
    # https://stackoverflow.com/questions/2038313/converting-midi-ticks-to-actual-playback-seconds
    tempo = int(line.split(',')[-1])
    bpm = 60e6 / tempo
    ms_per_tick = 60000 / (bpm * tpqn)
    
    # Skip to the first on event
    while "Note_on_c" not in midi_lines[i]:
        i += 1
        
    midi_lines = midi_lines[i:]
    
    return midi_lines, ms_per_tick, tpqn

def midi_lines_to_event_dict(midi_lines:list)->list[dict]:
    '''
    Process a list of comma separated values into a list of event dicts,
    sorted by absolute midi tick. Filter out non-supported event types.
    Tempo lines are already processed.
    
    Returned dictionary has keys:
    'type'
    'track'
    'tick'
    'event_line'
    
    '''
    # Each CSV string looks like: 
	# track, tick, note_on/off_c, channel, note, velocity
    # track, tick, Pitch_bend_c, channel, 14-bit pitch 
    # track, tick, Tempo, value
    
    final_dicts = []
    
    # For type hints
    line:str = ''
    for line in midi_lines:
        event = {}
        event_line = line.split(',')
        event['track'] = int(event_line[0])
        event['tick'] = int(event_line[1])
        event['type'] = event_line[2].strip()
            
        if(event['type'] in SUPPORTED_EVENTS):
            # to be processed later
            event['event_line'] = event_line
            final_dicts.append(event)                
    
    # Sort by tick, with Tempo taking priority for same-tick events
    final_dicts.sort(key=lambda e: (e['tick'], 0 if e['type'] == 'Tempo' else 1))
    return final_dicts

def write_bin_will_file():
    '''
    Support old firmware
    '''
    pass

def write_bin_new_file(list_of_events:list, out_path:str):
    '''
    List of events formatted as a list of dicts with keys:
    {
        'type'
        'tick'
        'time_ms'
        'track'
        'channel'
        'db1'
        'db2'
    }
    
    '''
    
    num_events = len(list_of_events)
    runtime = list_of_events[-1]['time_ms']
    
    with open(out_path, 'wb') as f:
        f.write(num_events.to_bytes(4, 'little', signed=False))
        f.write(runtime.to_bytes(4, 'little', signed=False))
    
        for event in list_of_events:
            packet = event_to_byte_array(event)
            if(packet is not None):
                f.write(packet)
        
        # End of song
        f.write(0xF0.to_bytes(1, signed=False))

def midi_2_bin(midi_lines:list, out_path:str):
    """
    
    """
    midi_lines, ms_per_tick, tpqn = process_midi_csv_header(midi_lines)
    current_time_ms = 0
    delta_ticks = 0
    prev_tick = 0
    event_dicts = midi_lines_to_event_dict(midi_lines)
    
    # Clean up into a new array of events 
    for i in range(len(event_dicts)):
        event = event_dicts[i]
        event_line = event['event_line']
        
        delta_ticks = event['tick'] - prev_tick
        current_time_ms += delta_ticks * ms_per_tick
        
        # if tempo, recalculate everything
        if(event['type'] == "Tempo"):
            tempo = int(event_line[-1])
            bpm = 60e6 / tempo
            ms_per_tick = 60000 / (bpm * tpqn)
            
        elif(event['type'] in SUPPORTED_EVENTS):
            # track, time, Pitch_bend_c, channel, 14-bit pitch 
            # track, time, note_on/off_c, channel, note, velocity
            if(event['track'] > 15):
                raise Exception(f"ERROR [midi_2_bin] Only 0-15 tracks accepted (track: {event['track']})")
            
            event['time_ms'] = int(current_time_ms)
            event['channel'] = int(event_line[3])
            event['db1'] = int(event_line[4])
            event['db2'] = int(event_line[5])
            
            # If it's note on c but velocity is zero, it's actually note off c
            if(event['type'] == 'Note_on_c' and event['db2'] == 0):
                event['type'] = 'Note_off_c'
        
        prev_tick = event['tick']
        
        # print('delta ticks', delta_ticks)
        LOGGER.info("New event: ")
        LOGGER.info(f"delta_ticks: {delta_ticks}")
        LOGGER.info(f"Current time ms: {current_time_ms}")
        LOGGER.info(json.dumps(event, indent=4))
        if(delta_ticks < 0):
            LOGGER.error(f"delta_ticks < 0: {delta_ticks} csv linenum {i}")
        
    write_bin_new_file(event_dicts, out_path)
    # write_bin_will_file()

    num_events = len(event_dicts)
    runtime = event_dicts[-1]['time_ms']
    print(f"INFO [midi_2_bin] Number of Events: {num_events}")
    print(f"INFO [midi_2_bin] Song length: {(runtime/1000):0.3f} sec")

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
                    print("WARNING: Encountered exception, ignoring...")
                
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
    