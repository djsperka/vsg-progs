; $Id: StimFFT.pls,v 1.2 2015-05-13 18:24:51 devel Exp $
;
;Super simple script, because it's not really doing very much

                SET      0.010 1 0     ;Get rate & scaling OK


0000 STIM:   'S DIGOUT [......1.]      ;Turns the stimulator on with pulse
0001            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0002            DIGOUT [......0.]      ;End pulse
0003            HALT                   ;End of this sequence section









