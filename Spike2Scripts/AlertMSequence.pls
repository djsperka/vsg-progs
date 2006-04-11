; $Log: not supported by cvs2svn $
;

                SET      1.000 1 0     ;Get rate & scaling OK

0000 E0:     '0 DIGOUT [00000001]
0001            DELAY  s(0.996)-1
0002            HALT                   ;End of this sequence section

0003 ES:     'S DIGOUT [00000011]      ;Signals the fixation point on
0004            DELAY  s(0.996)-1
0005            HALT   

0006 ET:     's DIGOUT [00000101]      ;Signals the fixation point off
0007            DELAY  s(0.996)-1
0008            HALT   

0009 ED:     'D DIGOUT [00010001]      ;Signals the stimuli on
0010            DELAY  s(0.996)-1
0011            HALT   

0012 EE:     'd DIGOUT [01000001]      ;Signals the stimuli off
0013            DELAY  s(0.996)-1
0014            HALT   

0015 EQ:     'q DIGOUT [10000001]      ;Signals the target contrast change
0016            DELAY  s(0.996)-1
0017            HALT   


