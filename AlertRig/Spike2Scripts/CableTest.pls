            SET    1,1,0           ; Get rate & scaling OK


E0:         DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E00:	'1	DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E2:     '2  DIGOUT [00000011]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

E3:     '3  DIGOUT [00000101]
            DELAY  s(0.996)-1
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


