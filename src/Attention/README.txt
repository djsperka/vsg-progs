Attention

Designed for the attention task. 
Handles variations for both Cornelius (Corey) and for Betty (Farran). 

Two gratings are displayed. The grating specified with "s" ("g") has its contrast
changed up/down with C/c (D/d). 

Single stim option (-S) means only one of the two stim are shown with the "S" trigger.
Which stim displayed depends on the cue used: If red cue ("F" trigger), then the 
grating specified with "s" is displayed. Only the C/c contrast change triggers will 
have an effect. If green cue ("G" trigger), then the grating specified with "g" is 
displayed. Only the D/d contrast change triggers will have an effect. 

White spot fixation: When cue circles are NOT used, the "W" trigger will turn on a
white fixation point. This was used as an initial fixation point. After fixation, 
the "F" or "G" was issued to change the fixpt to the colored cue. When cue circles
are used this option is not available due to VSG limitations. 

Cue circles: Using -Q option (specify -c as well) allows use of cue circles. The cue
circle is drawn (on the "W" trigger) around the stim indicated by the fixation point
trigger. So, if an "F" is sent, then the "W" will draw the cue circle around the "s" 
stim. The "w" will turn off the cue circle. 

Lollipops: These go on and off with the fixation point. Does not have a separate trigger. 
This and cue circles are mutually exclusive. The -c, -l and -G options can
be used here (though they have defaults of -c .1 -l .75 -G .5). The length of the stick 
is the distance from the edge of the cue circle to the fixpt center MINUS the gap. The 
fraction "-l" specifies how to place the line in that space; .75 centers it with equal 
spaces at either end. 

Command Line options:

-a		Use ascii triggers.
-v		Verbose. Might write more msgs, might not. 
-f		Fixation point parameters (required). 
		Format: -f x,y,diam,color
		x,y,diam are in degrees, color can be red, blue or green 
		or custom color {r,g,b}, 0 <= r,g,b <= 255
-b		background color, gray, black or white or custom (required)
-d		distance to screen in MM (required)
-s		stimulus grating specifier
-g		distractor grating specifier
-t		contrast triplet, format -t down,base,up : 
		0 <= down,base,up <= 100 e.g. -t 50,75,100
-A		answer point offset, in degrees (offset up/down from fixpt)
-D		answer point diameter, in degrees
-c		circle diameter differential; difference (in degrees) between 
		diameter of stimulus and the cue circle (if used)
-l		cue line fraction; fraction of distance from fixpt to cue circle 
		that cue line covers
-G		cue line gap
-L      use lollipop cues
-p		lollipop contrast
-P		lollipop stick contrast
-N		no answer points
-Q		use cue circles
-S		use single stim (may be stim or distractor depending on fixpt cue used)
-h		print help


Triggers

		The ready pulse issued is 0x20, seen on CED port 6.
        This program uses the following triggers and trigger responses:
        
        Action/visual    | FixptR| FixptG| Stim  |CueCircle|CueCircle|Chg - C  | Chg - c |Chg - D  |Chg - d  | AllOff   | quit     
        ascii trigger    | F     | G     | S     | W (on)  | w (off) | C       | c       | D       | d       | X        | q        
        input mask/val   |0x2/0x2|0x4/0x4|0x8/0x8|0x10/0x10|0x10/0x0 |0xe0/0x20|0xe0/0x40|0xe0/0x60|0xe0/0x80|0x6/0x6   | 0xf0/0xf0
        out mask/val     |0x1/0x1|0x1/0x1|0x2/0x2|0x4/0x4  | 0x4/0x0 |0x8/0x8  |0x8/0x8  |0x8/0x8  |0x8/0x8  |0xf/0x0   | 0xff/0x0
        spike2 port      | 1     | 1     | 2     | 3       | 3       | 4       | 4       | 4       | 4       |          |
        
