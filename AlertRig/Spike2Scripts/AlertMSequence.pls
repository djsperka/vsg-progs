; $Id: AlertMSequence.pls,v 1.4 2006-09-14 18:31:55 dan Exp $
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

0009 EA:     'a DIGOUT [....i..1]      ;advance stim to next segment
0010            DELAY  s(0.996)-1
0011            HALT   

0012 ED:     'D DIGOUT [...1...1]      ;Signals the dot on
0013            DELAY  s(0.996)-1
0014            HALT   

0015 EE:     'd DIGOUT [.i.0...1]      ;Signals the dot off
0016            DELAY  s(0.996)-1
0017            HALT   

0018 EJ:     'J DIGOUT [.......1]      ;Administers juice
0019            DIGOUT [.......0]
0020            DELAY  s(0.005)-1
0021            DIGOUT [.......1]
0022            DELAY  s(0.989)-1
0023            HALT   

0024 EQ:     'q DIGOUT [10000001]      ;Signals the target contrast change
0025            DELAY  s(0.996)-1
0026            HALT   


