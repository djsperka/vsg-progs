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
-P      specify spatial phase for gratings. Applies to any grating spec that 
        follows (when using a -g), or to the tuning curve set that follows
        (when using C|O|A|S|T|H). The value should be between -180 and +180. 
        The spatial phase can be specified more than once -- this may be 
        useful if using a dual vsg machine and you want the gratings on 
        the two screens to have different initial spatial phases.
        
        
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
-H <d0,h0,d1,h1,d2,h2...>   Donuts (see below)

Donut stim set is specified with a set of pairs of diameters. If the second diameter
of a pair is zero, then the first diameter is taken to be an aperture size
and a circular grating is presented. If the second diameter is greater than zero,
then it is the diameter of the annular hole in the donut. The second number in 
the pair must be less than the first!


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

Cool dual VSG options. The first option is required - it tells us how much the
coordinate offset is between the master and slave screen. The DMV options allow
you to specify what you want to display on either, or both, screens. When used, 
these options indicate the 'active screen' - the next stimulus type (COASTHg)
will be applied to that screen. If a fixpt has (not) been specified, then it will
(will not) be applied to that screen as well. There can be different stimulus 
sets on the master and slave screens. Alternatively, you may specify a stimulus 
set for the master but none for the slave. In this case the slave will display 
only the fixation point. Similarly you can specify a stimulus set for the slave 
but not the master. 

-r offset_filename
                      When using a dual VSG setup you have to provide a text file
                      that contains the offsets for stuff on the slave screen.
-D                    
                      Dual - put stim set on both master and slave
-M
                      Master only - put stim on master.
-V                   
                      Slave only - put stim on slave.


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

		The ready pulse issued is 0x20, seen on CED port 6.
        This program uses the following triggers and trigger responses:
        
        Action/visual    | Fixpt | Stim  | AllOff|Advance(*)| quit     
        ascii trigger    | F     | S     | X     | a        | q        
        input mask/val   |0x2/0x2|0x4/0x4|0x6/0x0|0x8/0x8(T)|0x10/0x10 
        out mask/val     |0x2/0x2|0x4/0x4|0x6/0x0|0x8/0x8(T)| n/a      
        spike2 port      | 2     | 3     | 2/3   | 4        | n/a      
        



