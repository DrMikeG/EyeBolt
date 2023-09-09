import numpy as np
import wave
import math

# Video settings
run = 30
output_audio_file = "./Mk5/Code/19_QuarterTurnsTakingPictures/run_0{}.wav".format(run)  # Name of the output video file
duration_per_note = 1.0  # Duration of each note in seconds
num_notes = 20
num_repeats = 4
audio_sample_rate = 44100  # Audio sample rate in Hz

# Define musical note frequencies (equal temperament tuning)
note_names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
base_frequency = 261.63  # Frequency of middle C (C4) in Hz

# Generate audio tones
audio_frames = []
for _ in range(num_repeats):
    for i in range(num_notes):
        note_index = i % len(note_names)
        octave = 3 + i // len(note_names)  # Start an octave below middle C (C4)
        frequency = base_frequency * (2 ** (1/12)) ** (note_index + 12 * octave)
        
        t = np.linspace(0, duration_per_note, int(duration_per_note * audio_sample_rate), endpoint=False)
        audio_waveform = np.sin(2 * np.pi * frequency * t)
        audio_frames.extend(audio_waveform)

# Normalize audio frames
max_amplitude = np.max(np.abs(audio_frames))
audio_frames = (audio_frames / max_amplitude).astype(np.float32)

# Create and configure the audio writer
audio_writer = wave.open(output_audio_file, 'wb')
audio_writer.setnchannels(1)  # Mono audio
audio_writer.setsampwidth(2)  # 16-bit audio
audio_writer.setframerate(audio_sample_rate)
audio_writer.writeframes(audio_frames.tobytes())

# Close the audio writer
audio_writer.close()

print(f'Audio saved as {output_audio_file}')
