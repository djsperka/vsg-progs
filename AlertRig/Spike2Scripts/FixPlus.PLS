; $Id: FixPlus.PLS,v 1.2 2010-09-21 19:33:15 devel Exp $

            SET      1.000 1 0     ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]
            HALT

; zero out all trigger lines
E0:     '0  DIGOUT [00000001]
            DELAY  10
            HALT

EF:     'F  DIGOUT [......11]      ;Signals the fixation point on
            DELAY  10
            HALT   

ES:     'S  DIGOUT [.....1.1]      ;Signals the stimulus on
            DELAY  10
            HALT   

ET:     's  DIGOUT [.....0.1]      ;Signals the stimulus off
            DELAY  10
            HALT   

EX:     'X  DIGOUT [.....001]      ; all off
            DELAY  10
            HALT   

EA:     'a  DIGOUT [....i..1]      ;Advance stimulus
            DELAY  10
            HALT   

EQ:     'q  DIGOUT [...1...1]      ;exit program
            DELAY  10
            HALT   

; Reward
ER:     'R  DIGOUT [.......1]
            DIGOUT [.......0]           ; downward pulse delivers juice
            DELAY  5
            DIGOUT [.......1]
            DELAY  100
            DBNZ   V1,ER                ; decrement V1, more juice unless V1==0
            HALT                   ;End of this sequence section


EJ:     'J  DIGOUT [.......1]
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT                   ; End of this sequence section
