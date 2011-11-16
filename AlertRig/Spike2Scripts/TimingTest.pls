                SET      1.000 1 0     ;Get rate & scaling OK


0000 E0:     '0 DIGOUT [00000000]
0001            DELAY  s(0.996)-1
0002            HALT                   ;End of this sequence section

0003 E00:    '1 DIGOUT [00000001]
0004            DELAY  s(0.996)-1
0005            HALT                   ;End of this sequence section

0006 E2:     '2 DIGOUT [00000010]
0007            DELAY  s(0.996)-1
0008            HALT                   ;End of this sequence section

0009 E3:     '3 DIGOUT [00000100]
0010            DELAY  s(0.996)-1
0011            HALT                   ;End of this sequence section

0012 E4:     '4 DIGOUT [00001000]
0013            DELAY  s(0.996)-1
0014            HALT                   ;End of this sequence section

0015 E5:     '5 DIGOUT [00010000]
0016            DELAY  s(0.996)-1
0017            HALT                   ;End of this sequence section

0018 E6:     '6 DIGOUT [00100000]
0019            DELAY  s(0.996)-1
0020            HALT                   ;End of this sequence section

0021 E7:     '7 DIGOUT [01000000]
0022            DELAY  s(0.996)-1
0023            HALT                   ;End of this sequence section

0024 E8:     '8 DIGOUT [10000000]
0025            DELAY  s(0.996)-1
0026            HALT                   ;End of this sequence section

0027 JCR:    'J BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0028            DIGOUT [.......1]      ;Assert that juicer is closed
0029            DIGOUT [.......0]      ;Downward pulse delivers juice
0030            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0031            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0032            JUMP   JDONE           ;Jump over JCRHI to a HALT
0033 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0034            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0035            DIGOUT [.......0]      ;Assert that juicer is closed
0036            DIGOUT [.......1]      ;Voltage High delivers juice
0037            DELAY  V1              ;Delay V1 ms, duration of reward
0038            DIGOUT [.......0]      ;Close juicer valve
0039 JDONE:     HALT                   ;End of this sequence section


