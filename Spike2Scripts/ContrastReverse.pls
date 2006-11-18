; $Id: ContrastReverse.pls,v 1.1 2006-11-18 00:48:58 dan Exp $
;
;

                SET      1.000 1 0     ;Get rate & scaling OK

0000 E0:     '0 DIGOUT [00000001]
0001            DELAY  s(0.996)-1
0002            HALT                   ;End of this sequence section

0003 ES:     'S DIGOUT [...0..i1]      ;Signals the stimulus on
0004            DELAY  s(0.996)-1
0005            HALT   

0006 ET:     's DIGOUT [...0.i.1]      ;Signals the stimulus and fixation point off
0007            DELAY  s(0.996)-1
0008            HALT   

0009 EU:     'x DIGOUT [...0.i.1]      ;Signals the stimulus and fixation point off - same as s 
0010            DELAY  s(0.996)-1
0011            HALT   

0012 ED:     'D DIGOUT [...1...1]      ;Signals the dot on
0013            DELAY  s(0.996)-1
0014            HALT   

0015 EJ:     'J DIGOUT [.......1]      ;Administers juice
0016            DIGOUT [.......0]
0017            DELAY  s(0.005)-1
0018            DIGOUT [.......1]
0019            DELAY  s(0.989)-1
0020            HALT   

0021 EQ:     'q DIGOUT [10000001]      ;Tells stim app to quit.
0022            DELAY  s(0.996)-1
0023            HALT   


