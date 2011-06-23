starstim - Can display a fixation point and targets that are placed around
           a circle. The targets are specified with the same parameters as 
           fixation points. The number of targets is also specified, as well
           as an optional offset (in degrees). The target positions are then
           determined by rotating the position of the target specified on the 
           command line by the offset value plus integer multiples of 360 
           divided by the number of targets.
           
           The number of targets divides the target positions into evenly 
           spaced positions on the circumference of the circle. The radius
           of the circle is determined by the distance from the fixation 
           point to the position specified for the target on the command line. 
           
           This scheme for specifying the position of the targets allows you 
           to specify an exact position for one of the targets (by specifying
           the position for the target on the command line) and to tweak that 
           position in one of two ways: either specify a slightly different
           target position or specify an offset angle. 


Command Line options:

-a		Use ascii triggers. This is useful for testing - 
		you can simulate triggers with keystrokes; no need for Spike2.
-v		Verbose. Might write more msgs, might not. 
-d      distance to screen in mm.
-b      background color. Can be a standard color ("gray", "black", "white", 
        "red", "green", "blue"), or a custom color specified with an rgb 
        triplet, e.g. r/g/b, where each of r,g and b are in [0,255]. 
-f		Fixation point parameters.
-t      Target parameters. The diameter and color for all targets are taken from 
        this. The x,y specify the position of the first target (unless an offset
        is also specified), and the other targets are placed evenly spaced around
        a circle whose radius is determined by the position of the fixpt (the 
        center of the circle) and the position of the first target. 
-n      number of targets
-o      target offset, in degrees
        