            SET    1,1,0           ; Get rate & scaling OK


E0:     '0  DIGOUT [00000000]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E00:	'1	DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E2:     '2  DIGOUT [00000010]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E3:     '3  DIGOUT [00000100]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E4:     '4  DIGOUT [00001000]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E5:     '5  DIGOUT [00010000]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E6:     '6  DIGOUT [00100000]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E7:     '7  DIGOUT [01000000]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E8:     '8  DIGOUT [10000000]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section


