; $Id: ASLCalibration.pls,v 1.2 2010-07-26 17:05:01 devel Exp $

            SET      1.000 1 0          ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]           ; zero out all trigger lines
            HALT

E0:     '0  DIGOUT [00000001]           ; zero all trig lines
            DELAY  10
            HALT

EQ:     'q  DIGOUT [.1......]      ;exit program
            DELAY  10
            HALT   

EM:     'M  DIGOUT [......1.]      ;Signals master fixation point on
            DELAY  10
            HALT   

EMM:    'm  DIGOUT [......0.]      ;Signals master fixation point off
            DELAY  10
            HALT   

ES:     'S  DIGOUT [.....1..]      ;Signals master fixation point on
            DELAY  10
            HALT   

ESS:    's  DIGOUT [.....0..]      ;Signals master fixation point off
            DELAY  10
            HALT   


; Reward
ER:     'R  DIGOUT [.......1]
            DIGOUT [.......0]           ; downward pulse delivers juice
            DELAY  5
            DIGOUT [.......1]
            DELAY  200
            DBNZ   V1,ER                ; decrement V1, more juice unless V1==0
            HALT                   ;End of this sequence section



