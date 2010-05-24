; $Id: CoolMouse.pls,v 1.1 2010-05-24 22:08:03 devel Exp $
;
;

            SET      1.000 1 0          ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]

E0:     '0  DIGOUT [00000001]           ; clear all lines except for juice
            DELAY  10
            HALT   

EF:     'F  DIGOUT [......1.]           ; fixpt on
            DELAY  10
            HALT   

EG:     'f  DIGOUT [......0.]           ; fixpt off
            DELAY  10
            HALT   

; Quit stimulus application
EQ:     'q  DIGOUT [10000001]
            DELAY  10
            HALT                   ;End of this sequence section

; Reward
ER:     'R  DIGOUT [.......1]
            DIGOUT [.......0]           ; downward pulse delivers juice
            DELAY  5
            DIGOUT [.......1]
            DELAY  200
            DBNZ   V1,ER                ; decrement V1, more juice unless V1==0
            HALT                   ;End of this sequence section
