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
-p      ready pulse. This should be an int; the binary value specifies the trigger
		lines which are pulsed when the application is ready to receive triggers. 
		In Spike2, the ports correspond to bit numbers in this arg - hence "-p 2"
		will pulse port 1 on the 1401. 
-w		Use step temporal waveform, not sinusoid (default) for counterphase tuning. 
		Ignored for all other types. Must be issued before the "-P" arg!

        
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
-Z <x0,y0,x1,y1,x2,y2,...> Grating position tuning curve. Grating at (x0,y0), then (x1,y1), ...


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
-R frames_per_term,first_term,n_terms[,contrast0,contrast1,contrast2,...] 
                      Each of these three values is an integer. The sequence 
                      terms are numbered starting at 0. It is an error to 
                      specify n_terms greater than the number of terms in the
                      sequence file, or to specify a first term which is less 
                      than 0 or greater than the number of terms in the 
                      sequence file. If a list of contrast values are given, 
                      then the 'a' trigger will advance the stimulus through
                      the contrasts, assigning 'contrast0' and -1*'contrast0' 
                      to the stimulus on the first trial. After advancing with
                      'a', then we use 'contrast1', and so on. 
-B frames_per_term,first_term,n_terms[,contrast0,contrast1,contrast2,...] 
                      Same as "-R", except the sequence presented is played 
                      twice, with the second iteration reversed. This ensures 
                      a balanced sequence overall. Note that the number of 
                      terms shown in the complete sequence is 2*n_terms. 
                      If a list of contrast values are given, the behavior is the 
                      same as that for -R above.


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


Attention Stimulus

This stimulus is designed for Henry's attention experiment. The stimulus is a set
of gratings (I've tested with up to 6 and it works fine). 

The gratings can be specified to start with a specific spatial phase at onset (when
it first appears on the screen). 

Any (or all) of the stimuli may have a contrast change at a specified time after onset.

With clever figuring, one can arrange for the contrast change to occur at a specific 
phase. The program knows nothing of this - it will arrange for the contrast change 
to occur at the time specified. It is up to the caller to make sure that the drift
velocity, initial phase and the time-to-contrast-change will lead to the intended 
phase at contrast change time. 

To specify this type of stimulus, use the -J option:

-J sec_max,fix_color_0,init_phase_0,sec_to_chg_0,off_bits_0,base_0_0,chg_0_0,base_0_1,chg_0_1,....

where the first parameter applies to all trials:

sec_max - max in seconds that the stim remains after contrast change; same for all
          trials

the remaining parameters come in groups, one for each trial. The size of each group
depends on the number of stimuli specified (-s) on the command line PRIOR TO THE -J
flag. The sampling trigger "a" advances to the next stim parameter group. 

The per-trial parameters are:

fix_0    - fixation color, can be red,blue,green,r,g,b,or an rgb triplet specified
           as r/g/b e.g. 122/124/200
init_phase_0 - initial spatial phase for all gratings
sec_to_chg_0 - seconds to display gratings at base contrast
off_bits_0   - each bit set means that stim is OFF for this trial; 1=first stim off,
               2=second stim off, 4=third stim off, ...
base_0_0   - base contrast, trial 0, stim 0
chg_0_0    - contrast to change to after sec_to_chg_0, trial 0, stim 0
base_0_1   - base contrast, trial 0, stim 1
chg_0_1    - contrast to change to after sec_to_chg_0, trial 0, stim 1
...        - more. 

For a stim with N gratings and M trials, there should be a total of 1 + M*(4 + N*2)
comma-separated parameters following the -J. 




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