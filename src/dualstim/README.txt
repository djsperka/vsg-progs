dualstim - Displays a fixation point and a stimulus. The fixation point is 
          optional. The stimulus can be a grating, a series of gratings 
          with varying contrast, temporal frequency, spatial frequency,
          orientation, area, x position, y position and more. 


General command line options:

-F filename     
		response file. You can fill a text file with your command line options
        (if there are too many for the windows command line) and specify the 
        filename with this switch. The file is opened and any command line 
        options there are read and parsed. You may have other options on the 
        actual command line when using -F, but the response file itself may not
        contain the -F option. 
-a		Use ascii triggers. This is useful for testing - 
		you can simulate triggers with keystrokes; no need for Spike2.
-v		Verbose. Might write more msgs, might not. 
-d mm   distance to screen in mm.
-b color
		background color. Can be a standard color ("gray", "black", "white", 
        "red", "green", "blue"), or a custom color specified with an rgb 
        triplet, e.g. r/g/b, where each of r,g and b are in [0,255]. 
-f x,y,diam,color 
		Fixation point parameters. If supplied the fixation point is drawn on
        top of the stimulus (if they overlap). Thus, if you supply the fixation
        point and send the stimulus trigger ("S") before sending the fixpt
        trigger ("F"), the stimulus will have a hole corresponding to the 
        fixation point position. You can omit the fixation point altogether and 
        then the stimulus will have no hole. 
-p      ready pulse. This should be an int; the binary value specifies the trigger
		lines which are pulsed when the application is ready to receive triggers. 
		In Spike2, the ports correspond to bit numbers in this arg - hence "-p 2"
		will pulse port 1 on the 1401. 
-h x,y,r_inner,r_mid,r_outer,num_divisions,r1,r2
		Crosshair specification. The crosshair is drawn on the screen before the 
		fixation point is displayed, and remains there when the fixation point is 
		displayed (as well as when the stim is shown). If r1,r2 are omitted the 
		crosshair tick marks are not drawn.
-K      Attempt to lock frame refresh on the two screens. This application will 
		attempt to synchronize the presentation of stimuli 

Command line options controlling stimuli

To specify stimuli for either (or both) VSG screens, you must pass arguments indicating
the set of stimuli you want to use. As stimulus set arguments are encountered they are
applied to the currently 'active' screen. The 'active' screen can be the master (-M), 
slave (-V) or both (-D). The last parsed value for a fixation point and xhair are applied
to the stimulus set for the active screen.

If a stimulus is specified for just one of the two screens, the last fixation point from 
the command line is applied to the other screen. 

-D      stimulus set args will be applied to both screens
-M      stimulus set args will be applied to master screen only
-V      stimulus set args will be applied to slave screen only
-r file_or_xy
        offset for slave screen graphics, as a file containing x,y pair, or as a 
        numeric pair (no spaces) e.g. -r 0.1,0.3
Stimulus set options

-t sec             Stim time in seconds
-s grating_specification (see below)
                   This specification is used as the template for subsequent stim parameter value 
                   set args that follow. The tuning parameter list options below MUST be 
                   preceded by one of these (not -g). 
-C c1,c2,c3,...    Grating contrast list
-O o1,o2,o3,...    Grating orienation list
-A d1,d2,d3,...    Grating aperture/diameter list
-S s1,s2,s3,...    Grating spatial frequency list
-T t1,t2,t3,...    Grating temporal frequency list
-H od1,id1,od2,id2,....   Donut diamters, 'od' is outer diameter, 'id' is inner diameter.
                          Inner diameter of 0 means no hole - its a regular grating.
-X x1,x2,x3,...    Grating x position list
-Y y1,y2,y3,...    Grating y position list
-g grating_specification  To specify a single grating as the stimulus


Grating specification

        The grating specification gives the parameters for a grating. The format
        of the spec is

        x,y,w,h,[wd,hd,]contrast%,sf,tf,orientation[,initial_phase,][,color_vector,s|q,r|e]

        x,y,w,h in degrees
        wd,hd inner diameters (degrees) for a donut. If omitted or they're 0,0 you get a grating.
        contrast should be an integer from 0-100. 
        sf - spatial frequency in cycles per degree
        tf - temporal drift frequency in cycles/sec
        0 <= orientation < 360
        initial_phase is in degrees (default 0)
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



dualstim.exe -a -d 1000 -b gray -r 0,0 -f 0,0,0.2,red -M -s 0,0,5,5,100,.2,0,90 -A 1,2,4,8 -V -s 0,0,5,5,100,.2,0,90 -H 4,1,5,2,6,4,10,8

   Master screen: area tuning curve (diameters 1,2,4,8 degrees) 
   Slave screen: donut tuning curve
   ASCII triggers: S   stim ON
                   s   stim off
                   a   advance to next stimulus
                   X   all off
                   q   quit program

   Assumed that you give triggers in the order: Ssa. (The "a" trig has a
   delay as the grating is redrawn). 



dualstim.exe -a -d 1000 -b gray -r 0,0 -f 0,0,0.2,red -M -s 0,0,5,5,100,.2,0,90 -X -2,0,2,4 -Y -2,0,2,4 -V -s 0,0,5,5,100,.2,0,90 -T 0.2,0.5,1.0,4.0

   Master screen: stim varies in x,y position: (-2,-2), (0,0), (2,2), (4,4)
   Slave screen: temporal freq tuning curve
   ASCII triggers: S   stim ON
                   s   stim off
                   a   advance to next stimulus
                   X   all off
                   q   quit program

   Assumed that you give triggers in the order: Ssa. (The "a" trig has a
   delay as the grating is redrawn). 

	Note: using -X and -Y here is equivalent to using -Z -2,-2,0,0,2,2,4,4




psychophysics input file
------------------------

The format for the psychophysics input file (command line arg -P <filename> and
used with DonutHunt script) is as follows:

Input values are comma-separated on each line. No comment lines or blank lines allowed.

Line 1: Begins with the word "master" or "slave". This indicates which screen will have
the grating "core" stimulus (which follows on line 2). 

Line 2: Grating "core" stimulus spec. The position and contrast are ignored.

Line 3: x0,y0,x1,y1 - position of the two core grids (degrees)

Line 4: Grating "donut" spec. The position, contrast and od are ignored.

Line 5: x0,y0,x1,y1 - position of the two donut grids (degrees). Can be diff't than "core".

Line 6: t2,t3,cbase,nHC,tHC
        t2 = time from grating onset to contrast change
		t3 = time from grating contrast change to gratings off and HC grids appear
		cbase = base contrast of "core" and "donut" gratings
		nHC = number of Hi-Contrast grids to be flashed after contrast change
		tHC = sec (total) the Hi-Contrast grids are on screen. Each grid: tHC/nHC
		
		

** The next 5 lines are per-trial values. 
** Each line should have the same number of comma-separated items on it!



Line 7: t1 - time from fixpt onset to appearance of gratings
Line 8: CUp - contrast up, when CC happens, this is the contrast the core changes to
Line 9: Which - which of the two core grids changes contrast. Either 0 or 1, where
        0 refers to the core at x0,y0 and 1 refers to the core at x1,y1 (see Line 3 above)
Line 10: orientation of core grating, in degrees
Line 11: outer diameter of donut, in degrees
