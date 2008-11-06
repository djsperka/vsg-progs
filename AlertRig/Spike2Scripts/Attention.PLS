; $Id: Attention.PLS,v 1.6 2008-11-06 02:08:47 devel Exp $

            SET      1.000 1 0     ;Get rate & scaling OK

            VAR    V45,LoopC=0     ;Define variable for section loops
            VAR    V46,RampC=0     ;Define variable for ramp loops
            VAR    V47,DelayC=0    ;Define variable for delay loops
            VAR    V48,Delay2=0    ;and another one
            VAR    V49,Delay3=0    ;and another one
            VAR    V50,Delay4=0    ;and another one
            VAR    V51,Delay5=0    ;and another one

E0:     '0  DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ;End of this sequence section

EQ:     'q  DIGOUT [1111...1]      ;exit program
            DELAY  s(0.996)-1
            HALT   

EF:     'F  DIGOUT [...0.011]      ;Signals the fixation point on
            DELAY  s(0.996)-1
            HALT   

E1:     'f  DIGOUT [...1.111]      ;Signals the fixation point off
            DELAY  s(0.996)-1
            HALT   

EG:     'G  DIGOUT [...0.101]      ;Signals the fixation point on
            DELAY  s(0.996)-1
            HALT   

EH:     'g  DIGOUT [...0.001]      ;Signals the fixation point off
            DELAY  s(0.996)-1
            HALT   

EW:     'W  DIGOUT [...1.001]      ;Signals the fixation point on
            DELAY  s(0.996)-1
            HALT   

EY:     'w  DIGOUT [...0.001]      ;Signals the fixation point off
            DELAY  s(0.996)-1
            HALT   

ES:     'S  DIGOUT [....1..1]      ;Signals the stimuli and answer pts on
            DELAY  s(0.996)-1
            HALT   

ET:     's  DIGOUT [....0..1]      ;Signals the stimuli off (answer points remain on)
            DELAY  s(0.996)-1
            HALT   

E2:     'X  DIGOUT [00010111]      ;Signals all off
            DELAY  s(0.996)-1
            HALT   

EC:     'C  DIGOUT [..1....1]      ;stimulus contrast UP
            DELAY  s(0.996)-1
            HALT   

EN:     'c  DIGOUT [.1.....1]      ;stimulus contrast DOWN
            DELAY  s(0.996)-1
            HALT   

ED:     'D  DIGOUT [.11....1]      ;distractor contrast UP
            DELAY  s(0.996)-1
            HALT   

EP:     'd  DIGOUT [1......1]      ;distractor contrast DOWN
            DELAY  s(0.996)-1
            HALT   

EJ:     'J  DIGOUT [.......1]      ;Administers juice
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT   


