crg - Contrast Reversign Grating

Presents a fixation point and one of two possible stimuli: a drifting grating or a contrast reversing grating
tied to a portion of a sequence (like an msequence). 

Command Line options:

-a		Use ascii triggers. This is useful for testing - 
		you can simulate triggers with keystrokes; no need for Spike2.
-v		Verbose. Might write more msgs, might not. 
-d      distance to screen
-f		Fixation point parameters (required). 
-s      grating spec. Used for displaying grating. 
		Contrast reversing gratings are drawn with the high/low contrast pair.
-m      sequence filename, e.g. -m f:/filename.txt
-F      parameter filename. Terms of the sequence used are written to this file (required)
-l      segment time
-t      frames per term (applies to sequences only)
-C      high/low contrast values (applies to sequences only)
