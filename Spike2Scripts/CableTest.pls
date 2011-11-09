; Revision 1.1  2011/11/07 jeff
; Added multiple juicer functionality

                SET      1.000 1 0     ;Get rate & scaling OK
0000            BLT    V3,1,OPENHI     ;Branch, if V3 is 0, go to OPENHI
0001            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0002            JUMP   INITDONE        ;Jump over OPENHI to a HALT
0003 OPENHI:    DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0004 INITDONE:  HALT                   ;End of setup, rest runs only on (Safe)SampleKey

0005 E0:     '0 DIGOUT [0000000.]
0006            DELAY  s(0.996)-1
0007            HALT                   ;End of this sequence section

0008 E1:     '1 DIGOUT [0000000.]
0009            DELAY  s(0.996)-1
0010            HALT                   ;End of this sequence section

0011 E2:     '2 DIGOUT [0000001.]
            ;DELAY  s(0.996)-1
0012            HALT                   ;End of this sequence section

0013 E3:     '3 DIGOUT [0000010.]
            ;DELAY  s(0.996)-1
0014            HALT                   ;End of this sequence section

0015 E4:     '4 DIGOUT [0000100.]
0016            DELAY  s(0.996)-1
0017            HALT                   ;End of this sequence section

0018 E5:     '5 DIGOUT [0001000.]
0019            DELAY  s(0.996)-1
0020            HALT                   ;End of this sequence section

0021 E6:     '6 DIGOUT [0010000.]
0022            DELAY  s(0.996)-1
0023            HALT                   ;End of this sequence section

0024 E7:     '7 DIGOUT [0100000.]
0025            DELAY  s(0.996)-1
0026            HALT                   ;End of this sequence section

0027 E8:     '8 DIGOUT [1000000.]
0028            DELAY  s(0.996)-1
0029            HALT                   ;End of this sequence section

0030 JCR:    'J BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0031            DIGOUT [.......1]      ;Assert that juicer is closed
0032            DIGOUT [.......0]      ;Downward pulse delivers juice
0033            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0034            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0035            JUMP   JDONE           ;Jump over JCRHI to a HALT
0036 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0037            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0038            DIGOUT [.......0]      ;Assert that juicer is closed
0039            DIGOUT [.......1]      ;Voltage High delivers juice
0040            DELAY  V1              ;Delay V1 ms, duration of reward
0041            DIGOUT [.......0]      ;Close juicer valve
0042 JDONE:     HALT                   ;End of this sequence section

0043 EQ:     'q DIGOUT [1111000.]
0044            DELAY  10
0045            HALT   

0046 EZ:     'z DIGOUT [1111111.]
0047            HALT                   ;End of this sequence section

