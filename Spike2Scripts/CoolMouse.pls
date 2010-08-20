; $Id: CoolMouse.pls,v 1.2 2010-08-20 01:13:55 devel Exp $
;
;

            SET      1.000 1 0          ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]
            HALT

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


EJ:     'J  DIGOUT [.......1]
            DIGOUT [.......0]           ; downward pulse delivers juice
            DELAY  10
            DIGOUT [.......1]
            HALT
