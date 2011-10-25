            SET    1,1,0           ; Get rate & scaling OK
            DIGOUT [00000000]
            HALT

E0:     '0  DIGOUT [00000000]
            HALT                   ; End of this sequence section

E1:	'1  DIGOUT [00000001]
            HALT                   ; End of this sequence section

E2:     '2  DIGOUT [00000010]
            HALT                   ; End of this sequence section

E3:     '3  DIGOUT [00000100]
            HALT                   ; End of this sequence section

E4:     '4  DIGOUT [00001000]
            HALT                   ; End of this sequence section

E5:     '5  DIGOUT [00010000]
            HALT                   ; End of this sequence section

E6:     '6  DIGOUT [00100000]
            HALT                   ; End of this sequence section

E7:     '7  DIGOUT [01000000]
            HALT                   ; End of this sequence section

E8:     '8  DIGOUT [10000000]
            HALT                   ; End of this sequence section

Ez:     'z  DIGOUT [11111111]
            HALT                   ; End of this sequence section

EQ:     'q  DIGOUT [11110000]
            DELAY 10
            HALT


