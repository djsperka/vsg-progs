            SET    1,1,0           ; Get rate & scaling OK
            DIGOUT [00000001]
            HALT

E0:     '0  DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E1:	'1  DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E2:     '2  DIGOUT [00000011]
            ;DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E3:     '3  DIGOUT [00000101]
            ;DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E4:     '4  DIGOUT [00001001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E5:     '5  DIGOUT [00010001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E6:     '6  DIGOUT [00100001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E7:     '7  DIGOUT [01000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E8:     '8  DIGOUT [10000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EJ:     'J  DIGOUT [.......1]      ;Administers juice
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT   

EQ:     'q  DIGOUT [11110000]
            DELAY 10
            HALT


