; $Log: not supported by cvs2svn $
;

                SET      1.000 1 0     ;Get rate & scaling OK

                VAR    V45,LoopC=0     ;Define variable for section loops
                VAR    V46,RampC=0     ;Define variable for ramp loops
                VAR    V47,DelayC=0    ;Define variable for delay loops
                VAR    V48,Delay2=0    ;and another one
                VAR    V49,Delay3=0    ;and another one
                VAR    V50,Delay4=0    ;and another one
                VAR    V51,Delay5=0    ;and another one

0000 E0:        DIGOUT [00000001]
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

0015 EC:     'C DIGOUT [....i..1]      ;Signals the target contrast change
0016            DELAY  s(0.996)-1
0017            HALT   

0018 ED:     'D DIGOUT [...i...1]      ;Signals the distractor contrast change (catch trial)
0019            DELAY  s(0.996)-1
0020            HALT   

0021 EH:     'H DIGOUT [..1....1]      ;Signals that the contrast will increase
0022            DELAY  s(0.996)-1
0023            HALT   

0024 EL:     'L DIGOUT [..0....1]      ;Signals that the contrast will decrease
0025            DELAY  s(0.996)-1
0026            HALT   

0027 EA:     'A DIGOUT [.1.....1]      ;Signals the answer points on
0028            DELAY  s(0.996)-1
0029            HALT   

0030 E3:     'a DIGOUT [.0.....1]      ;Signals the answer points off
0031            DELAY  s(0.996)-1
0032            HALT   

0033 EJ:     'J DIGOUT [.......1]      ;Administers juice
0034            DIGOUT [.......0]
0035            DELAY  s(0.005)-1
0036            DIGOUT [.......1]
0037            DELAY  s(0.989)-1
0038            HALT   

0039 EX:     'X DIGOUT [00000001]      ;Clears all
0040            DELAY  s(0.996)-1
0041            HALT                   ;End of this sequence section

