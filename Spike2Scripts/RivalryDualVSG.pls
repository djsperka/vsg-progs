; $Id: RivalryDualVSG.pls,v 1.4 2011-11-16 00:48:00 jeff Exp $
; Revision 1.3  2011/11/08 jeff
; Added multiple juicer functionality

                SET      0.010 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use


; zero out all trigger lines, close juicer
0000 ZERO:   '0 MOVI   V2,1            ;Log that sequencer is in use
0001            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
0002            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0003            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
0004 ZOPENHI:   DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0005 ZERODONE:  MOVI   V2,0            ;Log that sequencer is not in use
0006            HALT                   ;End of this sequence section

; Stimulus 0/0
0007 E5:     '5 MOVI   V2,1
0008            DIGOUT [...0101.]
0009            MOVI   V2,0
0010            HALT   

; Stimulus 1/1
0011 EA:     'A MOVI   V2,1
0012            DIGOUT [...1010.]
0013            MOVI   V2,0
0014            HALT   

; Stimulus 0/1
0015 E6:     '6 MOVI   V2,1
0016            DIGOUT [...0110.]
0017            MOVI   V2,0
0018            HALT   

; Stimulus 1/0
0019 E9:     '9 MOVI   V2,1
0020            DIGOUT [...1001.]
0021            MOVI   V2,0
0022            HALT   


; Stimulus off request
0023 EX:     'X MOVI   V2,1
0024            DIGOUT [0000000.]
0025            MOVI   V2,0
0026            HALT   

; Turn on stim
0027 EG:     'g MOVI   V2,1
0028            DIGOUT [001.....]
0029            MOVI   V2,0
0030            HALT                   ;End of this sequence section

; turn off stim
0031 EY:     'x MOVI   V2,1
0032            DIGOUT [010.....]
0033            MOVI   V2,0
0034            HALT                   ;End of this sequence section


; Quit stimulus application
0035 EQ:     'q MOVI   V2,1
0036            DIGOUT [1000000.]
0037            MOVI   V2,0
0038            HALT                   ;End of this sequence section

; Reward
0039 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0040            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0041            DIGOUT [.......1]      ;Assert that juicer is closed
0042            DIGOUT [.......0]      ;Downward pulse delivers juice
0043            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0044            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0045            JUMP   RDONE           ;Jump over REWHI to a HALT
0046 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0047            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0048            DIGOUT [.......0]      ;Assert that juicer is closed
0049            DIGOUT [.......1]      ;Voltage High delivers juice
0050            DELAY  V1              ;Delay V1 ms, duration of reward
0051            DIGOUT [.......0]      ;Close juicer valve
0052 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0053            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0054 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0055            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0056            DIGOUT [.......1]      ;Assert that juicer is closed
0057            DIGOUT [.......0]      ;Downward pulse delivers juice
0058            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0059            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0060            JUMP   JDONE           ;Jump over JCRHI to a HALT
0061 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0062            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0063            DIGOUT [.......0]      ;Assert that juicer is closed
0064            DIGOUT [.......1]      ;Voltage High delivers juice
0065            DELAY  V1              ;Delay V1 ms, duration of reward
0066            DIGOUT [.......0]      ;Close juicer valve
0067 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0068            HALT                   ;End of this sequence section


