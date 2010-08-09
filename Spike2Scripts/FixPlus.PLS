; $Id: FixPlus.PLS,v 1.1 2010-08-09 23:23:36 devel Exp $

            SET      1.000 1 0     ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]
            HALT

; zero out all trigger lines
E0:     '0  DIGOUT [00000001]
            DELAY  10
            HALT

EQ:     'q  DIGOUT [1111...1]      ;exit program
            DELAY  10
            HALT   

EF:     'F  DIGOUT [.....011]      ;Signals the fixation point on
            DELAY  10
            HALT   

EG:     'G  DIGOUT [.....101]      ;Signals the fixation point on
            DELAY  10
            HALT   

EW:     'W  DIGOUT [...1...1]      ; cue circle on
            DELAY  10
            HALT   

EY:     'w  DIGOUT [...0...1]      ; cue circle off
            DELAY  10
            HALT   

ES:     'S  DIGOUT [....1..1]      ; stim on
            DELAY  10
            HALT   

ET:     's  DIGOUT [....0..1]      ; stim off
            DELAY  10
            HALT   

E2:     'X  DIGOUT [00000111]      ; all off
            DELAY  10
            HALT   

EC:     'C  DIGOUT [..1....1]      ; stimulus contrast UP
            DELAY  10
            HALT   

EN:     'c  DIGOUT [.1.....1]      ;stimulus contrast DOWN
            DELAY  10
            HALT   

ED:     'D  DIGOUT [.11....1]      ;distractor contrast UP
            DELAY  10
            HALT   

EP:     'd  DIGOUT [1......1]      ;distractor contrast DOWN
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
