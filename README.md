# Stella
Repository for the electronic trumpet project

Stella is an electronic trumpet that lets trumpet players control monophonic MIDI without changing their trumept technique. See openstella.com to learn more about Stella's technology.

This code is meant to be compiled in the Arduino IDE and used with a Teensy microcontroller. In order to compile it, you must have the appropriate Teensy libraries. If you have questions, ask on the openstella forums or email dbaylies@gmail.com.

The code can be summarized as follows:

    1. The valve state is continuously read.

    2. FFT results are continuously analyzed.

        -The frequency peak is determined, though not precisely

    3. The valve position and FFT results are combined to determine the note being played.

    4. Corresponding MIDI data is sent via USB, consisting of:

        -note on signals

        -note off signals

        -note velocity signals

        -volume signals

It is a work in progress, and will continue to develop in such a way that the user's music intent is interpreted as clearly as possible.
