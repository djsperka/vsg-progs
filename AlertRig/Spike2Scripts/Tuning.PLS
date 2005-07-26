; $Log: not supported by cvs2svn $

                SET      1.000 1 0     ;Get rate & scaling OK

0000 E0:     '0 DIGOUT [00000001]
0001            DELAY  s(0.996)-1
0002            HALT                   ;End of this sequence section

0003 EF:     'F DIGOUT [......11]      ;Signals the fixation point on
0004            DELAY  s(0.996)-1
0005            HALT   

0006 E1:     'f DIGOUT [......01]      ;Signals the fixation point off
0007            DELAY  s(0.996)-1
0008            HALT   

0009 ES:     'S DIGOUT [.....1.1]      ;Signals the stimuli on
0010            DELAY  s(0.996)-1
0011            HALT   

0012 E2:     's DIGOUT [.....0.1]      ;Signals the stimuli off
0013            DELAY  s(0.996)-1
0014            HALT   

0015 Ea:     'a DIGOUT [....1..1]      ;Signals the tuned parameter to advance
0016            DELAY  s(0.996)-1
0017            HALT   

0033 EJ:     'J DIGOUT [.......1]      ;Administers juice
0034            DIGOUT [.......0]
0035            DELAY  s(0.005)-1
0036            DIGOUT [.......1]
0037            DELAY  s(0.989)-1
0038            HALT   

0039 EX:     'X DIGOUT [00000001]      ;Clears all
0040            DELAY  s(0.996)-1
0041            HALT                   ;End of this sequence section

