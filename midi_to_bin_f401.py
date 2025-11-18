import os
import sys
import py_midicsv as pm 
import json
import logging
import numpy as np
import scipy.io.wavfile as wavfile

DEBUG=1
if(DEBUG):
    if(os.path.exists('midi2bin.log')):
        os.remove('midi2bin.log')
    
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

def synthesize_wave(events, sample_rate=44100, waveform='sine', amplitude=0.3):
    """
    Generate a simple waveform from parsed MIDI events.
    events: list of dicts containing 'type', 'time_ms', 'db1' (note), and 'db2' (velocity)
    waveform: sine, square, or sawtooth
    
    Side Note: This function was entirely written by ChatGPT and worked on the first try, without
    ANY modification!! 10/26/2025
    """
    # Determine total duration
    duration_s = max(e['time_ms'] for e in events) / 1000.0
    audio = np.zeros(int(sample_rate * (duration_s + 1.0)))  # +1s margin
    
    # Simple note frequency mapping
    def midi_to_freq(note): return 440 * 2 ** ((note - 69) / 12)
    
    # Find all note-on events
    note_ons = [e for e in events if e['type'] == 'Note_on_c' and e['db2'] > 0]
    
    for on in note_ons:
        note = on['db1']
        start_s = on['time_ms'] / 1000.0
        velocity = on['db2'] / 127.0  # scale 0–1
        freq = midi_to_freq(note)

        # Find matching note-off
        offs = [
            e for e in events 
            if e['type'] == 'Note_off_c' and e['db1'] == note and e['time_ms'] > on['time_ms']
        ]
        if offs:
            end_s = offs[0]['time_ms'] / 1000.0
        else:
            end_s = start_s + 0.5  # fallback 0.5s note

        start_idx = int(start_s * sample_rate)
        end_idx = min(int(end_s * sample_rate), len(audio))

        # Time vector
        t = np.linspace(0, end_s - start_s, end_idx - start_idx, endpoint=False)

        # Choose waveform
        if waveform == 'sine':
            wave = np.sin(2 * np.pi * freq * t)
        elif waveform == 'square':
            wave = np.sign(np.sin(2 * np.pi * freq * t))
        elif waveform == 'saw':
            wave = 2 * (t * freq - np.floor(0.5 + t * freq))
        else:
            raise ValueError("Unsupported waveform type")

        # Apply amplitude envelope
        env = np.linspace(0, 1, int(0.05 * len(t)))  # 5% fade in/out
        envelope = np.ones_like(t)
        envelope[:len(env)] *= env
        envelope[-len(env):] *= env[::-1]

        audio[start_idx:end_idx] += amplitude * velocity * wave * envelope

    # Normalize and clip
    audio = np.clip(audio, -1, 1)
    return audio

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

def ms_to_time_string(ms: int) -> str:
    minutes = ms // 60000
    seconds = (ms % 60000) // 1000
    milliseconds = ms % 1000
    return f"{minutes:02d}:{seconds:02d}.{milliseconds:03d}"

def seconds_to_time_string(seconds: float) -> str:
    hours = int(seconds // 3600)
    minutes = int((seconds % 3600) // 60)
    seconds = seconds % 60
    milliseconds = int((seconds - int(seconds)) * 1000)
    return f"{hours:02d}h {minutes:02d}m {int(seconds):02d}.{milliseconds:03d}s"

def check_concurrent_events(midi_events:list):
    '''
    Parse the list of midi events and determine if there are any times where
    track 0 or track 2 has more than two notes at a time
    
    '''
    
    # Error if:
    #  more than 4 concurrent notes across all tracks
    #  num_on[3] > 1
    #  num_on[2] + num_on[3] > 2
    #  num_on[1] + num_on[2] + num_on[3] > 3
    # Warning if:
    #  more than two notes on any track at a time
    
    
    num_on = [0, 0, 0, 0]
    for event in midi_events:
        time_human = ms_to_time_string(event['time_ms'])
        tick = event['tick']
        if(event['type'] == 'Note_on_c'):
            num_on[event['track']] += 1
            
        elif(event['type'] == 'Note_off_c'):
            num_on[event['track']] -= 1
        
        total_on = sum(num_on)
        if(total_on > 4):
            err = f"Error: More than four concurrent notes at time {time_human} ({num_on} @ {tick})"
            raise Exception(err)
        
        # Sum of notes to the right
        error_case = (num_on[3] > 1) or ((num_on[3] + num_on[2]) > 2) or ((num_on[3] + num_on[2] + num_on[1]) > 3)
        if(error_case):
            err = f"ERROR: Too many notes {num_on} at time {time_human} ({num_on[2]} @ {tick})"
            raise Exception(err)
        
        if(max(num_on) > 2):
            idx = num_on.index(max(num_on))
            err = f"WARNING: More than two concurrent notes on track {idx} at time {time_human} ({num_on[0]} @ {tick})"
            print(err)
            
        # if(num_on[2] > 2):
        #     err = f"ERROR: More than two concurrent notes on track 2 at time {time_human} ({num_on[2]} @ {tick})"
        #     raise Exception(err)
        
        

            
    total_on = sum(num_on)
    if(total_on > 0):
        raise Exception(f"Error: there is more than one note still active. Tracks: {num_on}")

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

def sort_midi_events(list_of_dicts:list)->list:
    
    # Define type priority: lower number = higher priority
    # Process note off events before note on events
    PRIORITY = {
        'Note_off_c': 0,
        'Tempo': 1,
        'Note_on_c': 2
    }

    # Sort by tick first, then by type priority
    list_of_dicts.sort(
        key=lambda e: (
            e['tick'],
            PRIORITY.get(e['type'], 99)  # default low priority if not found
        )
    )

    return list_of_dicts

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
        event['track'] = int(event_line[0])-1
        event['tick'] = int(event_line[1])
        event['type'] = event_line[2].strip()
            
        if(event['type'] in SUPPORTED_EVENTS):
            # to be processed later
            event['event_line'] = event_line
            final_dicts.append(event)                
    
    # Sort by tick, with Tempo taking priority for same-tick events
    final_dicts = sort_midi_events(final_dicts)
    return final_dicts

def write_bin_will_file(list_of_events:list, out_path:str):
    '''
    Support old firmware
    '''
    
    out_path += '.will'
    # 2 bytes of number of events, little endian
    # 6 byte packets:
    #     3 byte timestamp, little endian
    #     note byte
    #     velocity byte
    # 0xFF at the end
    
    # L151 interrupter LUT starts at C1
    # FW subtracts 12 from the note number we write
    # E4 is midi note number 64, in FW table idx is 40, 12 is subtracted,
    # so subtract another 12 from here
    
    if(len(list_of_events) > 65535):
        raise Exception("Song too long, max supported 2^16 - 1 events")
    with open(out_path, 'wb') as f:
        f.write(len(list_of_events).to_bytes(2, 'little'))
        
        for event in list_of_events:
            t0 = event['time_ms'] & 0xFF
            t1 = (event['time_ms'] & 0xFF00) >> 8
            t2 = (event['time_ms'] & 0xFF0000) >> 16
            
            note = event['db1'] - 12
            velocity = event['db2']
            
            # track is 1 indexed in FW
            f.write(bytearray([event['track']+1, t0, t1, t2, note, velocity]))
        f.write(bytearray([255]))
        
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
    out_path += '.bin'
    
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

def duplicate_event(a, b)->bool:
    atick = int(a['tick'])
    btick = int(b['tick'])
    
    ach = a['channel']
    bch = b['channel']
    
    atype = a['type']
    btype = b['type']
    
    adb1  = a['db1']
    bdb1  = b['db1']
    
    adb2  = a['db2']
    bdb2  = b['db2']
    
    return (atick==btick and ach==bch and atype==btype and adb1==bdb1 and adb2==bdb2)
    

def midi_2_bin(midi_lines:list, out_path:str):
    """
    Return the run time of the song in seconds
    """
    midi_lines, ms_per_tick, tpqn = process_midi_csv_header(midi_lines)
    current_time_ms = 0
    delta_ticks = 0
    prev_tick = 0
    event_dicts = midi_lines_to_event_dict(midi_lines)
    note_events_only = []
    
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
                
            # Check if it's a duplicate event, if so, drop.
            # if(i > 0):
            #     prev_type = event_dicts[i-1]['type']
            #     prev_tick = event_dicts[i-1]['tick']
            #     prev_ch   = int(event_dicts[i-1]['channel'])
            #     if((event['type'] == prev_type) and (event['tick'] == prev_tick) and (event['channel'] == prev_ch)):
            #         pass
                
            note_events_only.append(event)
        prev_tick = event['tick']
        
        if(DEBUG):
            # print('delta ticks', delta_ticks)
            LOGGER.info("New event: ")
            LOGGER.info(f"delta_ticks: {delta_ticks}")
            LOGGER.info(f"Current time ms: {current_time_ms}")
            LOGGER.info(json.dumps(event, indent=4))
            if(delta_ticks < 0):
                LOGGER.error(f"delta_ticks < 0: {delta_ticks} csv linenum {i}")

    # Dump all the events
    if(DEBUG):
        with open('debug_events.txt', 'w') as f:
            for event in note_events_only:
                f.write(f'{event['track']} {event['time_ms']} {event['type']}\n')
        
    check_concurrent_events(note_events_only)
    write_bin_new_file(note_events_only, out_path)
    write_bin_will_file(note_events_only, out_path)
    audio = synthesize_wave(note_events_only, waveform='square')
    wavfile.write(out_path+'.wav', 44100, (audio*32767).astype(np.int16))
    
    # write_wav_file(note_events_only, out_path)

    num_events = len(note_events_only)
    runtime = note_events_only[-1]['time_ms']
    print(f"INFO [midi_2_bin] Number of Events: {num_events}")
    print(f"INFO [midi_2_bin] Song length: {(runtime/1000):0.3f} sec")
    
    return runtime / 1000

if __name__ == '__main__':
    if(len(sys.argv) < 2):
        print("Usage: python midi_to_bin_f401.py <midi file or directory of midi files>")
        exit(1)
        
    if(os.path.isdir(sys.argv[1])):
        basedir = os.path.basename(sys.argv[1])
        
        successful = []
        failed = []
        total_runtime = 0
        for file in os.listdir(sys.argv[1]):
            if(os.path.splitext(file)[1] == '.mid'):
                print(f"INFO [midi_to_bin_f401] Processing {os.path.basename(file)}")
                
                in_path = os.path.realpath(os.path.join(sys.argv[1], file))
                filename = os.path.splitext(os.path.basename(in_path))[0]
                out_path = os.path.join(os.path.dirname(in_path), filename)
                try:
                    csv_str = pm.midi_to_csv(in_path)
                    total_runtime += midi_2_bin(csv_str, out_path)
                    successful.append(filename)
                except Exception as e:
                    failed.append(filename)
                    print("WARNING: Encountered exception during processing")
        print("========================================")
        print("=              Summary                 =")
        print("========================================")
        print("Successfully processed:")
        for win in successful:
            print("  " + win)
            
        print("Failed to process:")
        for fail in failed:
            print("  "+fail)
        
        print(f"Total runtime: {total_runtime} seconds ({seconds_to_time_string(total_runtime)})")
                
    # A file name was given on the command line        
    else:
        if(not os.path.exists(sys.argv[1])):
            print(f"ERROR Cannot find {sys.argv[1]}")
        in_path = os.path.realpath(sys.argv[1])
        filename = os.path.splitext(os.path.basename(in_path))[0]
        out_path = os.path.join(os.path.dirname(in_path), filename)    
            
        print(f"INFO [midi_to_bin_f401] Processing '{filename}'")
        # A list of strings, each string is a line of csv
        csv_str = pm.midi_to_csv(in_path)
        midi_2_bin(csv_str, out_path)
