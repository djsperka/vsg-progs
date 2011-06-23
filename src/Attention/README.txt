Attention

Designed for the attention task. Handles variations for both Cornelius (Corey) and for Betty (Farran). 

Command Line options:

-a		Use ascii triggers. This is useful for testing - you can simulate triggers with keystrokes; no need for Spike2.
-v		Verbose. Might write more msgs, might not. 
-f		Fixation point parameters (required). 
		Format: -f x,y,diam,color
		x,y,diam are in degrees, color can be red, blue or green or custom color {r,g,b}, 0 <= r,g,b <= 255
-b		background color, gray, black or white or custom (required)
-d		distance to screen in MM (required)
-s		stimulus grating specifier (see GRATINGS below)
-g		distractor grating specifier (see GRATINGS below)
-t		contrast triplet, format -t down,base,up : 0 <= down,base,up <= 100 e.g. -t 50,75,100
-A		answer point offset, in degrees (offset up/down from fixpt)
-D		answer point diameter, in degrees
-c		circle diameter differential; difference (in degrees) between diameter of stimulus and the cue circle (if used)
-l		cue line fraction; fraction of distance from fixpt to cue circle that cue line covers
-G		cue line gap
-L      use lollipop cues
-p		lollipop contrast
-P		lollipop stick contrast
-N		no answer points
-Q		use cue circles
-S		use single stim (may be stim or distractor depending on fixpt cue used)
-h		print help
