                SET    1,1,0           ; Get rate & scaling OK
0000            BLT    V3,1,OPENHI     ;Branch, if V3 is 0, go to OPENHI
0001            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0002            JUMP   INITDONE        ;Jump over OPENHI to a HALT
0003 OPENHI:    DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0004 INITDONE:  HALT                   ;End of setup, rest runs only on (Safe)SampleKey

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


