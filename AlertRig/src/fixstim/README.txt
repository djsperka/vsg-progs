fixstim - Displays a fixation point and a stimulus. The fixation point is 
          optional. The stimulus can be a grating, a series of gratings 
          with varying contrast, temporal frequency, spatial frequency (NI), 
          orientation (NI) or area (NI). The stimulus can also be a contrast
          reversing grating, where a static grating alternates between a 
          specified contrast and its inverse according to a given sequence.


Command Line options:

-a		Use ascii triggers. This is useful for testing - 
		you can simulate triggers with keystrokes; no need for Spike2.
-v		Verbose. Might write more msgs, might not. 
-d      distance to screen in mm.
-b      background color. Can be a standard color ("gray", "black", "white", 
        "red", "green", "blue"), or a custom color specified with an rgb 
        triplet, e.g. r/g/b, where each of r,g and b are in [0,255]. 
-f		Fixation point parameters. If supplied the fixation point is drawn on
        top of the stimulus (if they overlap). Thus, if you supply the fixation
        point and send the stimulus trigger ("S") before sending the fixpt
        trigger ("F"), the stimulus will have a hole corresponding to the 
        fixation point position. You can omit the fixation point altogether and 
        then the stimulus will have no hole. 
-P      specify spatial phase for gratings. OBSOLETE. This arg is allowed on the
		command line but is ignored by the program.
-p      ready pulse. This should be an int; the binary value specifies the trigger
		lines which are pulsed when the application is ready to receive triggers. 
		In Spike2, the ports correspond to bit numbers in this arg - hence "-p 2"
		will pulse port 1 on the 1401. 

        
To specify a single grating as the stimulus, use the "-g" option:

-g      Grating specifier, see below.

To specify a tuning curve-type stimulus, where one of the parameters in the 
grating specification can be stepped through a series of values, you must give 
the grating specification using the "-s" option
(NOTE: USE "-s" for tuning curves, NOT "-g"!!!) followed by one of the following:

-C <contrast0,contrast1,...>    Contrast tuning curve
-O <orientation0,orientation1,...>   Orientation tuning curve
-A <area0,area1,...>    Area tuning curve
-S <sf0,sf1,...>    Spatial frequency tuning curve
-T <tf0,tf1,...>    Temporal frequency tuning curve
-P <tf,phase0,phase1,...>   Counterphase tuning curve. tf is the frequency of the 
                            phase transition, and phase0 is the spatial phase
                            of the grating. The temporal frequency (drift velocity)
                            of the grating spec is ignored (and is set to 0).


Contrast Reversing Grating Stimulus

To specify a contrast reversing grating stimulus, first give a grating 
specification using the "-s" option AND a filename containing the sequence 
(as a series of 0's and 1's, with NO OTHER CHARACTERS), and either the -R or -B 
options:

-F <sequence_file>    sequence_file is the path to an ascii text file 
                      containing the sequence as a series of 0's and 1's. 
                      The grating specified with "-s" is taken as the "1", and 
                      the same grating with its contrast reversed is taken as 
                      the "0". The temporal frequency of the given grating 
                      specification is ignored (it is set to 0). 
-R frames_per_term,first_term,n_terms  
                      Each of these three values is an integer. The sequence 
                      terms are numbered starting at 0. It is an error to 
                      specify n_terms greater than the number of terms in the
                      sequence file, or to specify a first term which is less 
                      than 0 or greater than the number of terms in the 
                      sequence file. 
-B frames_per_term,first_term,n_terms 
                      Same as "-R", except the sequence presented is played 
                      twice, with the second iteration reversed. This ensures 
                      a balanced sequence overall. Note that the number of 
                      terms shown in the complete sequence is 2*n_terms. 


Drifting Bar Stimulus

This stimulus consists of a rectangular bar that drifts across the screen. The 
direction of the drift is perpendicular to the width of the bar, and the center
of the bar will always drift through the center of the screen. The color of the 
bar, its width and height, and its drift velocity must be specified, along with 
a list of direction angles. The zero degree orientation is when the width of the 
bar is horizontal and the direction is upwards on the screen. Positive angles
run counter-clockwise from there; thus a 90 degree orientation has a vertically
oriented bar travelling from right to left across the screen. 

Triggers are generated on the frame where any part of the bar first appears on the 
screen and on the frame when the last portion of the bar leaves the screen. 

To specify this type of tuning curve, use the -G option:

-G color,bar_width,bar_height,bar_speed,ori1,ori2,ori3,...

where bar_width and bar_height are in visual degrees, and bar_speed is in visual 
degrees per second. Orientation are all in degrees. The color may be specified 
with one of white, black, gray, red, green or blue, or as a custom color by giving
an rgb triplet (r/g/b), where each of r, g, and b are integers in [0, 255]. 



Grating specification

        The grating specification gives the parameters for a grating. The format
        of the spec is

        x,y,w,h,contrast%,sf,tf,orientation,color_vector,s|q,r|e

        x,y,w,h in degrees
        contrast should be an integer from 0-100. 
        sf - spatial frequency in cycles per degree
        tf - temporal drift frequency in cycles/sec
        0 <= orientation < 360
        color_vector should be b|w|black|white|gray|... for black/white
        s|q indicates pattern type, s for sine wave, q for square wave
        r|e indicates aperture type, (r)ectangular or (e)lliptical

        The last three args (color_vector, pattern, aperture) can be omitted. 
        The default values are black/white color vector, sine wave, elliptical 
        aperture. 


Triggers

		The ready pulse issued is 0x20, seen on CED port 6 (this may be 
		changed with the -p option - see above). 
        This program uses the following triggers and trigger responses:
        
        Action/visual    | Fixpt | Stim  | AllOff|Advance(*)| quit     
        ascii trigger    | F     | S     | X     | a        | q        
        input mask/val   |0x2/0x2|0x4/0x4|0x6/0x0|0x8/0x8(T)|0x10/0x10 
        out mask/val     |0x2/0x2|0x4/0x4|0x6/0x0|0x8/0x8(T)| n/a      
        spike2 port      | 2     | 3     | 2/3   | 4        | n/a      
        




Examples:



fixstim.exe -a -d 1000 -b gray -P 180 -s 0,0,5,5,100,.2,0,90 -A 1,2,4,8

   Single vsg, area tuning curve (diameters 1,2,4,8 degrees). 
   All gratings have spatial phase 180 degrees (should be black at center). 
   ASCII triggers: S   stim ON
                   s   stim off
                   a   advance to next diameter
                   X   all off
                   q   quit program

   Assumed that you give triggers in the order: Ssa. (The "a" trig has a
   delay as the grating is redrawn). 



fixstim.exe -a -d 1000 -b gray -f 0,0,1,red -P 180 -g 0,0,5,5,100,.2,0,90

   Single vsg, Fixpt and grating stim. 
   Grating have spatial phase 180 degrees (should be black at center). 
   ASCII triggers: F   fixpt on
                   S   stim ON
                   s   stim off
                   a   advance to next diameter
                   X   all off
                   q   quit program



fixstim.exe -a -d 1000 -b gray -r f:\experiments\offset.txt
            -f 0,0,1,red -P 180 -K
            -M -s 0,0,5,5,100,.2,0,90 -A 1,2,4,8 
            -V -s 0,0,5,5,100,.5,1,0 -H 4,1,8,1,6,5,3,0


   Dual vsg, offset taken from f:\experiments\offset.txt (file must exist)
   All gratings spatial phase 180
   The "-K" tells the prog to attempt master/slave timing (this causes 
   long startup time)
   Master screen - area tuning curve
   Slave screen - donut tuning curve
   same triggers as above   