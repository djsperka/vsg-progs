            SET    1,1,0           ; Get rate & scaling OK

E0:         DIGOUT [......01]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section


EJ:     'J  DIGOUT [.......1]
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT                   ; End of this sequence section

