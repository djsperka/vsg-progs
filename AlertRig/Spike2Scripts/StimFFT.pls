; $Id: StimFFT.pls,v 1.3 2016-02-04 23:49:19 devel Exp $
;
;Super simple script, because it's not really doing very much

                SET      0.010 1 0     ;Get rate & scaling OK


0000 STIM:   'S DIGOUT [......1.]      ;Turns the stimulator on with pulse
;0001            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0001            DELAY V1               ;Delay for desired time
0002            DIGOUT [......0.]      ;End pulse
0003            HALT                   ;End of this sequence section









