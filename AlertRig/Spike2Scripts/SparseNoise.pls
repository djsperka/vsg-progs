; $Id: SparseNoise.pls,v 1.1 2010-03-18 23:58:29 devel Exp $
;
;

            SET      1.000 1 0          ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000001]

E0:     '0  DIGOUT [00000000]           ; clear all lines
            DELAY  10
            HALT   

ES:     's  DIGOUT [......1.]           ; start stim
            DELAY  10
            HALT   

ET:     'X  DIGOUT [......0.]           ; Stop stim
            DELAY  10
            HALT   

EQ:     'q  DIGOUT [10000000]           ; quit program
            DELAY  10
            HALT   


