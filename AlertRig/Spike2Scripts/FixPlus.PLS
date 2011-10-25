; $Id: FixPlus.PLS,v 1.3 2011-10-25 17:28:49 devel Exp $

            SET      0.010 1 0     ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]
            VAR    V2=0
            HALT   

; zero out all trigger lines
E0:     '0  MOVI   V2,1
            DIGOUT [00000001]
            MOVI   V2,0
            HALT   

EF:     'F  MOVI   V2,1
            DIGOUT [......11]      ;Signals the fixation point on
            MOVI   V2,0
            HALT   

ES:     'S  MOVI   V2,1
            DIGOUT [.....1.1]      ;Signals the stimulus on
            MOVI   V2,0
            HALT   

ET:     's  MOVI   V2,1
            DIGOUT [.....0.1]      ;Signals the stimulus off
            MOVI   V2,0
            HALT   

EX:     'X  MOVI   V2,1
            DIGOUT [.....001]      ;all off
            MOVI   V2,0
            HALT   

EA:     'a  MOVI   V2,1
            DIGOUT [....i..1]      ;Advance stimulus
            MOVI   V2,0
            HALT   

EQ:     'q  MOVI   V2,1
            DIGOUT [...1...1]      ;exit program
            MOVI   V2,0
            HALT   

; Reward
ER:     'R  DIGOUT [.......1]
            DIGOUT [.......0]      ;downward pulse delivers juice
            DELAY  5
            DIGOUT [.......1]
            DELAY  100
            DBNZ   V1,ER           ;decrement V1, more juice unless V1==0
            HALT                   ;End of this sequence section


EJ:     'J  MOVI   V2,1
            DIGOUT [.......1]
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            MOVI   V2,0
            HALT                   ;End of this sequence section
