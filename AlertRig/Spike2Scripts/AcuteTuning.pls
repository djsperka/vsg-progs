; $Id: AcuteTuning.pls,v 1.2 2010-04-23 23:41:06 devel Exp $
;
;

            SET      1.000 1 0          ;1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000000]

E0:     '0  DIGOUT [00000000]           ; clear all lines
            DELAY  10
            HALT   

ES:     's  DIGOUT [......1.]           ; start stim
            DELAY  10
            HALT   

ET:     'X  DIGOUT [......0.]           ; Stop stim
            DELAY  10
            HALT   

EQ:     'q  DIGOUT [1.......]           ; quit program
            DELAY  10
            HALT   


