/*
    COPYRIGHT 2021 DAVID BAYLIES
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Audio.h>

// Automatically generated by Teensy Audio Design Tool. xy coordinates represent object locations
// in design tool GUI, are useful for importing back to GUI
AudioInputI2S            i2s1;           //xy=1248,470
AudioOutputI2S           i2s2;           //xy=1483,515
AudioFilterBiquad        biquad1;        //xy=1490,391
AudioAnalyzeFFT1024      myFFT;          //xy=1652,450
AudioConnection          patchCord1(i2s1, 0, i2s2, 0);
AudioConnection          patchCord2(i2s1, 0, i2s2, 1);
AudioConnection          patchCord3(i2s1, 0, biquad1, 0);
AudioConnection          patchCord4(biquad1, myFFT);
AudioControlSGTL5000     sgtl5000;     //xy=1403,624

// memory for MIDI note number
int noteold = 0;

void setup() {
    Serial.begin(250000);

    AudioMemory(60);

    sgtl5000.enable();
    sgtl5000.inputSelect(AUDIO_INPUT_MIC);
    // Set amplifier gain (in decibels, from 0 to 63)
    sgtl5000.micGain(40);
    // Just sets headphone jack volume
    sgtl5000.volume(0.5);

    // Butterworth filter, 12 dB/octave, filters out unnecessary frequencies (trumpet mouthpiece range is 87Hz -> 932 Hz)
    biquad1.setLowpass(0, 1000, 0.707);

    // Configure the FFT window algorithm to use
    myFFT.windowFunction(AudioWindowHanning1024);
}

// Main code loop
void loop() {
    // Initialize to a number that is unused. No note will sound if mindex does not change, and remains 512.
    int mindex = 512;
    // once new buffer fills, run it through fft
    if (myFFT.available()) {
        bool endcapture = false;
        bool peakdetect = false;

        float m = 0;

        // find and store peak the value and index if it is loud and a lower frequency than the last recorded value.
        // This ensures that the fundmental is recorded, and not a harmonic.
        // Additionally, don't include very low frequencies because it is likely noise and not the intended pitch
        // Higher i means higher frequencies - I believe there are 26 bins
        for (int i = 0; i < 26; i++) {
            // Include fft correction factor? You should research what the magnitude here represents. What are the units?
            float n = (float)myFFT.read(i); // 1.0 represents a full scale sine wave
            int istart = 0;

            // A peak has begun
            if (n > 0.07 && i > 4 && !peakdetect && !endcapture) {
                peakdetect = true;
                istart = i;
            }

            // The peak has ended
            if (n < 0.07 && i > 4 && peakdetect && !endcapture) {
                endcapture = true;
                int iend = i;
                // find number of indexes in the peak
                int peaklength = iend - istart;
                for (int ipk = 0; ipk < peaklength; ipk++) {
                    if (myFFT.read(istart + ipk) > m) {
                        m = myFFT.read(ipk + istart);
                        mindex = ipk + istart;
                    }
                }
            }
        }

        // Find velocity with the peak FFT value. Also acts as MIDI volume.
        // TODO: Change denominator to current max FFT bin amplitude
        int16_t velocity = m*127/0.6;

        int valveState = ReadValves();
        int midiNote = determineMIDINote(valveState, mindex);
        sendMIDIdata(midiNote, velocity, peakdetect);
    }
}

int ReadValves(){
    // Check pot inputs, assigning valve variables booleans based on their state
    // TODO: Allow half-valving, valves shouldn't just be boolean
    int centerPosition = 512; // potentiometer reports 1024 possible values
    bool valve1 = analogRead(16) < centerPosition;
    bool valve2 = analogRead(14) < centerPosition;
    bool valve3 = analogRead(17) < centerPosition;

    // Combine all valve states into one three bit value
    return (valve1 << 2) + (valve2 << 1) + valve3;
}

int determineMIDINote(int valveState, int fftIndex)
{
    int note = 0;
    // Assign note based on fftIndex and valve combination
    switch (valveState) {
        case 0:
                if (fftIndex >= 2 && fftIndex <= 4) {
                    note = 40; // E3
                } else if (fftIndex >= 5 && fftIndex <= 6) {
                    note = 47; // B3
                } else if (fftIndex >= 7 && fftIndex <= 8) {
                    note = 52; // E4
                } else if (fftIndex >= 9 && fftIndex <= 10) {
                    note = 56; // Ab4 (G#4)
                } else if (fftIndex >= 11 && fftIndex <= 13) {
                    note = 59; // B4
                } else if (fftIndex >= 14 && fftIndex <= 20) {
                    note = 64; // E5
                }
                break;
        case 2:
                if (fftIndex >= 2 && fftIndex <= 5) {
                    note = 41; // F3
                } else if (fftIndex >= 6 && fftIndex <= 7) {
                    note = 48; // C4
                } else if (fftIndex >= 8 && fftIndex <= 9) {
                    note = 53; // F4
                } else if (fftIndex >= 10 && fftIndex <= 11) {
                    note = 57; // A4
                } else if (fftIndex >= 12 && fftIndex <= 14) {
                    note = 60; // C5
                } else if (fftIndex >= 15 && fftIndex <= 20) {
                    note = 65; // F5
                }
                break;
        case 4:
                if (fftIndex >= 2 && fftIndex <= 5) {
                    note = 42; // Gb3
                } else if (fftIndex >= 6 && fftIndex <= 7) {
                    note = 49; // Db4
                } else if (fftIndex >= 8 && fftIndex <= 9) {
                    note = 54; // Gb4
                } else if (fftIndex >= 10 && fftIndex <= 11) {
                    note = 58; // Bb4
                } else if (fftIndex >= 12 && fftIndex <= 14) {
                    note = 61; // Db5
                } else if (fftIndex >= 15 && fftIndex <= 20) {
                    note = 66; // Gb5
                }
                break;
        case 6:
        case 1:
                if (fftIndex >= 2 && fftIndex <= 5) {
                    note = 43; // G3
                } else if (fftIndex >= 6 && fftIndex <= 7) {
                    note = 50; // D4
                } else if (fftIndex >= 8 && fftIndex <= 10) {
                    note = 55; // G4
                } else if (fftIndex >= 11 && fftIndex <= 12) {
                    note = 59; // B4
                } else if (fftIndex >= 13 && fftIndex <= 15) {
                    note = 62; // D5
                } else if (fftIndex >= 16 && fftIndex <= 22) {
                    note = 67; // G5
                }
                break;
        case 3:
                if (fftIndex >= 2 && fftIndex <= 5) {
                    note = 44; // Ab3
                } else if (fftIndex >= 6 && fftIndex <= 8) {
                    note = 51; // Eb4
                } else if (fftIndex >= 9 && fftIndex <= 10) {
                    note = 56; // Ab4
                } else if (fftIndex >= 11 && fftIndex <= 13) {
                    note = 60; // C5
                } else if (fftIndex >= 14 && fftIndex <= 16) {
                    note = 63; // Eb5
                } else if (fftIndex >= 17 && fftIndex <= 22) {
                    note = 68; // Ab5
                }
                break;
        case 5:
                if (fftIndex >= 2 && fftIndex <= 6) {
                    note = 45; // A3
                } else if (fftIndex >= 7 && fftIndex <= 8) {
                    note = 52; // E4
                } else if (fftIndex >= 9 && fftIndex <= 11) {
                    note = 57; // A4
                } else if (fftIndex >= 12 && fftIndex <= 13) {
                    note = 61; // Db5
                } else if (fftIndex >= 14 && fftIndex <= 17) {
                    note = 64; // E5
                } else if (fftIndex >= 18 && fftIndex <= 22) {
                    note = 69; // A5
                }
                break;
        case 7:
                if (fftIndex >= 2 && fftIndex <= 6) {
                    note = 46; // Bb3
                } else if (fftIndex >= 7 && fftIndex <= 9) {
                    note = 53; // F4
                } else if (fftIndex >= 10 && fftIndex <= 12) {
                    note = 58; // Bb4
                } else if (fftIndex >= 13 && fftIndex <= 14) {
                    note = 62; // D5
                } else if (fftIndex >= 15 && fftIndex <= 20) {
                    note = 65; // F5
                } else if (fftIndex >= 21 && fftIndex <= 22) {
                    note = 70; // Bb5
                }
                break;
    }

    return note;
}


void sendMIDIdata(int noteNumber, int16_t velocity, bool peakdetect){
    const int midiChannel = 1;

    // Send volume signals whenever a peak is detected
    if (peakdetect == true) {
        usbMIDI.sendControlChange(07, velocity, midiChannel);
        // Serial.println(velocity);
    }

    // Send note on/off data if the note has changed
    if (noteNumber != noteold) {
        // Only send note on message if a peak was detected and note !=0
        // TODO: note != 0 part is probably a sloppy way of fixing a bug where playing a note about a high Bb triggers a very low note on (probably note 0)
        if (peakdetect == true && noteNumber != 0) {
            usbMIDI.sendNoteOn(noteNumber, velocity, midiChannel);
            //Serial.print("note on: ");
            //Serial.println(note);
        }
        if (noteold != 0) {
            usbMIDI.sendNoteOff(noteold, velocity, midiChannel);
            //Serial.print("note off: ");
            //Serial.println(noteold);
        }
    }

    // store note and valve information for next loop's comparisons
    noteold = noteNumber;
}
