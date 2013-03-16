; $Id: UseOpto.pls,v 1.1 2013-03-16 00:39:39 devel Exp $
;
;

;Simple script, useful for testing of TTL/DAC and for emergency kill
                SET      1.000 1 0     ;Get rate & scaling OK

0000 E0:     'O DIGOUT [00000001]
0001            HALT                   ;Turn dig 0 on

0002 ES:     'P DIGOUT [00000000]      ;Turn dig 0 off
0003            HALT   

0002 EU:     'U DAC 0,0
0003            HALT

0002 EV:     'V DAC 0,1
0003            HALT

0002 EW:     'W DAC 0,2
0003            HALT

0002 EX:     'X DAC 0,3
0003            HALT

0002 EY:     'Y DAC 0,4
0003            HALT

0002 EZ:     'Z DAC 0,5
0003            HALT

