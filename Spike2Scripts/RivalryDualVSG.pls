; $Id: RivalryDualVSG.pls,v 1.1 2010-07-28 01:38:24 devel Exp $

            SET      1.000 1 0     ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]
            HALT

; zero out all trigger lines
E0:     '0  DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ;End of this sequence section

; Stimulus 0/0
E5:     '5  DIGOUT [...01011]
            DELAY  10
            HALT   

; Stimulus 1/1
EA:     'A  DIGOUT [...10101]
            DELAY  10
            HALT   

; Stimulus 0/1
E6:      '6 DIGOUT [...01101]
            DELAY  10
            HALT   

; Stimulus 1/0
E9:      '9 DIGOUT [...10011]
            DELAY  10
            HALT   


; Stimulus off request
EX:     'X  DIGOUT [00000001]
            DELAY  10
            HALT   

; Turn on stim
EG:     'g  DIGOUT [001....1]
            DELAY  10
            HALT                   ;End of this sequence section

; turn off stim
EY:     'x  DIGOUT [010....1]
            DELAY  10
            HALT                   ;End of this sequence section


; Quit stimulus application
EQ:     'q  DIGOUT [10000001]
            DELAY  10
            HALT                   ;End of this sequence section

; Reward
ER:     'R  DIGOUT [.......1]
            DIGOUT [.......0]           ; downward pulse delivers juice
            DELAY  5
            DIGOUT [.......1]
            DELAY  100
            DBNZ   V1,ER                ; decrement V1, more juice unless V1==0
            HALT                   ;End of this sequence section


