fixstim - Displays a fixation point and a stimulus. The fixation point is 
          optional. The stimulus can be a grating, a series of gratings 
          with varying contrast, temporal frequency, spatial frequency (NI), 
          orientation (NI) or area (NI). The stimulus can also be a contrast
          reversing grating, where a static grating alternates between a 
          specified contrast and its inverse according to a given sequence.


Command Line options:

-F <filename>      command line arguments can be written to an ascii text file
                   and specified on the command line like this. The file will be
                   read and processed as if its contents were on the command line.
                   This is useful when tuning curve commands are longer than the 
                   windows limit for command line args.
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
[ old version in use on AcuteRig, not used on alert rigs - see -P ]
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
-P <tf,s|q,phase0,phase1,...>   Counterphase tuning curve. tf is the frequency of the 
                            phase transition, and "s"("q") indicates that the transition 
                            should be a sin (square) wave. Phase0 is the initial spatial phase
                            of the grating. The temporal frequency (drift velocity)
                            of the grating spec is ignored (and is set to 0).
-Z <x0,y0,x1,y1,x2,y2,...> Grating position tuning curve. Grating at (x0,y0), then (x1,y1), ...
-H <od0,od1,od2,...> Danish tuning curve. "od" is outer diameter of donut. See below. 


Danish stimulus

A danish is a stimulus with a "donut" shaped grating and an optional "hole" grating.
To specify the gratings, use two "-s" grating specs on the command line prior to the 
"-H" specification of the diameters. The first grating specified is the donut; the
second is the hole. The diameters specified after "-H" are the outer diameters of the 
donut grating. There is no requirement that the donut and hole be concentric, and the 
donut doesn't even need to be a donut - it can be an ordinary grating. 

This stimulus has an on/off trigger "s" for the donut grating. After the stimulus is 
displayed it can be toggled on/off with the "s" trigger. The frame trigger line can be
used to determine the onset/offset of the donut. 

Example: 
fixstim -a -v -b gray -p 2 -d 500 -f 0,0,1,green -s 0,0,6,6,4,4,100,.2,1,135,b,s,e \
                                                 -s 0,0,4,4,0,0,100,.2,1,45,b,s,e \
                                                 -H 6.0,7.0,8.0

Here, the first stim (donut) has an inner diameter of 4, outer diameter will be
6, 7, 8 as the trials advance. 



How to specify sequences

The Contrast Reversing Grating Stimulus and the Full Field Flash stimulus
each rely on a sequence. By default the full msequence is used, but you can 
specify another sequence with the -e argument. It has two forms:

-e 01...
-e f=filename.txt

In the first form, the sequence is an ASCII string. The elements of the string 
must be compatible with the stimulus you specify. In the case of the full field
flash stimulus, the elements of the sequence must match the number of colors
specified. In other words, if your -L argument specifies 3 colors, then your 
sequence must only consist of the characters '0', '1', and '2'.

For both of these stimuli, the -e argument is optional. Fixstim will use the msequence 
if no other sequence is specified. 



Contrast Reversing Grating Stimulus

To specify a contrast reversing grating stimulus, first give a grating 
specification using the "-s" option, and either the -R or -B 
options:

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


Full Screen Flash Stimulus

To specify a full screen flash stimulus (which will cycle through full screen colors),
use the -L option:

-L frames_per_term
-L frames_per_term,color0,color1,...
-L frames_per_term,first_term,number_of_terms
-L frames_per_term,first_term,number_of_terms,color0,color1,...

In all cases, the sequence used is the msequence (the default) or a user-specified sequence (-e). 

The sequence must be a string of ASCII characters consisting of 0,1,... up to the number of colors
specified. NO COMMAS OR OTHER SEPARATORS! If no colors are specified, then color0=black and 
color1=white; in that case, the sequence must consist of 0 and 1.

If first_term and number_of_terms are not specified, then the entire sequence is used. 


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

Each stimuli grating can have a companion cue circle drawn (see -Q below). The colors and
diameter (relative to the enclosed grating) can be specified. The cue circles are drawn 
with the fixation point - hence they appear on the fixpt page as well as the stimuli 
pages (before and after contrast change). Cue circles are not drawn if their corresponding
stimulus grating is switched OFF for a given trial. 

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


To specify the cue circles, use the -Q option:

-Q rad_extra_0,linewidth_0,color_0,radius_extra_1,linewidth_1,color_1,...

To specify cue circles with cue points (fixation points placed at center of circle, with 
same diameter as the fixation point, and the same color as the corresponding cue
circle), use the same args with the -q option. 

where the parameters are:

radius_extra_0 - radius of circle is this plus the radius of corresponding grating
linewidth_0 - linewidth to use for cue circle, in pixels (s/b an integer)
color_0 - color of circle

The -Q option must come after all gratings have been specified (-s, once for each 
grating) and before the -J option. The cues may be omitted, but if they are used, 
then they must be specified for each grating. 

There may be multiple sets of cues specified - if so the number of cues specified 
must be an exact multiple of gratings specified (e.g. if 2 gratings specified, 
you can specify 2, 4, 6, ... cues, but not 3, 5, 7...). 

When multiple sets of cues are specified, each set is used together in a single
trial. Specify the set to use in the 'off_bits' arg for the trial. The second byte
of off_bits is treated as an integer. If it is 0, then we use the first set
of cues specified (the default). If it is 1 (off_bits = 256), then the second set
of cues is used. Note that bits in the low-order byte indicate which stimuli should
be turned OFF. 

Flashies

Flashies are gratings that flash on/off during an attention trial. In order to use you must
specify gratings with "-k". The first gratings so specified is referred to as index "0" in 
flashy configs, the second grating is referred to as "1", and so on. 

All spatio-temporal parameters for the grating (except for x,y and w,h) are preserved.

================================================================================
THE FLASHY ARG "-j" MUST COME BEFORE THE ATTENTION ARG "-J" ON THE COMMAND LINE!
================================================================================
The flashy arg format is going to be hard to write up. An explanation may work better.

-j <flashy config for first trial>,<flashy config for second trial>,...

The flashy config for each trial looks like this:

NF,f0,x0,y0,w0,h0,on0,off0,f1,x1,y1,w1,h1,on1,off1,...

NF = of flashies to be shown this trial
f0 = index of first flashy for this trial
x0,y0 = position of first flashy
w0,h0 = width, height of first flashy
on0 = time (measured from stim onset) that flashy should turn ON
off0 = time (measured from stim onset) that flashy should turn OFF

If there are NO FLASHIES for a trial, then use NF=0 and nothing else for that trial. The
next value should be NF for the next trial.

Example: Here is a flashy config for 4 trials. Assume that there are 4 "-k" gratings specified.

-j 2,0,-6,7,1,1,.5,1.0,1,-4,7,1,1,1.1,1.5,4,0,-6,7,1,1,.5,.8,1,-4,7,1,1,.9,1.2,2,4,7,1,1,1.5,1.7,3,6,7,1,1,1.8,1.9,0,3,0,-6,7,1,1,.5,1,1,-4,7,1,1,1.5,2.5,2,4,7,1,1,3.0,3.5

Breaking this down the individual trials look like this:

2, 0,-6,7,1,1,.5,1.0, 1,-4,7,1,1,1.1,1.5,
4, 0,-6,7,1,1,.5,.8,  1,-4,7,1,1,.9,1.2,  2,4,7,1,1,1.5,1.7,  3,6,7,1,1,1.8,1.9,
0,
3, 0,-6,7,1,1,.5,1,   1,-4,7,1,1,1.5,2.5, 2,4,7,1,1,3.0,3.5

The first trial has two flashies, the second has 4. The third trial has no flashies, and the 4th has 3. 






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