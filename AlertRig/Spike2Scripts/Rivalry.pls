; $Id: Rivalry.pls,v 1.3 2010-02-12 15:55:46 devel Exp $

            SET      1.000 1 0     ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]

; zero out all trigger lines
E0:     '0  DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ;End of this sequence section


; Left stimulus request
EH:     'l  DIGOUT [......11]
            DELAY  10
            HALT   

; Right stimulus request
EV:     'r  DIGOUT [.....1.1]
            DELAY  10
            HALT   

; toggle stimulus request
EZ:     'z  DIGOUT [....i..1]
            DELAY  10
            HALT   

; Stimulus off request
EX:     'X  DIGOUT [.....001]
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


