cramp - Contrast Ramp

Presents a full screen that goes from black->white in a smooth ramp. 
When a black->white color vector is used (the default) this is effectively 
a luminance ramp from min->max. Triggers are provided (see below) at the 
bottom of the ramp, in addition to start/stop triggers. 

Command Line options:

-a		Use ascii triggers. This is useful for testing - 
		you can simulate triggers with keystrokes; no need for Spike2.
-v		Verbose. Might write more msgs, might not. 
-f      Temporal frequency (required) in cycles/sec. One cycle runs through
        the color vector from start to end. 
-c      color vector. Default is black/white. Standard values are 
		"b"(black/white), "l"(l-cone), "m"(m-cone), "s"(s-cone). 
		Custom values specified by giving a pair of RGB values for the 
		ends of the vector in this format: (r1/g1/b1)-(r2/g2/b2). 
		Screen is initially at midpoint of vector (and is put there
        after "X" is issued). 
        