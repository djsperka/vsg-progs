; $Id: RivalryDualVSG.pls,v 1.3 2011-11-09 18:15:37 jeff Exp $
; Revision 1.3  2011/11/08 jeff
; Added multiple juicer functionality

                SET      0.010 1 0     ;Get rate & scaling OK
0000            DIGOUT [00000000]
                VAR    V2=0            ;V2 logs whether the sequencer is in use

0001            BLT    V3,1,OPENHI     ;Branch, if V3 is 0, go to OPENHI
0002            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0003            JUMP   INITDONE        ;Jump over OPENHI to a HALT
0004 OPENHI:    DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0005 INITDONE:  HALT                   ;End of setup, rest runs only on (Safe)SampleKey

; zero out all trigger lines, close juicer
0006 ZERO:   '0 MOVI   V2,1            ;Log that sequencer is in use
0007            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
0008            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0009            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
0010 ZOPENHI:   DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0011 ZERODONE:  MOVI   V2,0            ;Log that sequencer is not in use
0012            HALT                   ;End of this sequence section

; Stimulus 0/0
0013 E5:     '5 MOVI   V2,1
0014            DIGOUT [...0101.]
0015            MOVI   V2,0
0016            HALT   

; Stimulus 1/1
0017 EA:     'A MOVI   V2,1
0018            DIGOUT [...1010.]
0019            MOVI   V2,0
0020            HALT   

; Stimulus 0/1
0021 E6:     '6 MOVI   V2,1
0022            DIGOUT [...0110.]
0023            MOVI   V2,0
0024            HALT   

; Stimulus 1/0
0025 E9:     '9 MOVI   V2,1
0026            DIGOUT [...1001.]
0027            MOVI   V2,0
0028            HALT   


; Stimulus off request
0029 EX:     'X MOVI   V2,1
0030            DIGOUT [0000000.]
0031            MOVI   V2,0
0032            HALT   

; Turn on stim
0033 EG:     'g MOVI   V2,1
0034            DIGOUT [001.....]
0035            MOVI   V2,0
0036            HALT                   ;End of this sequence section

; turn off stim
0037 EY:     'x MOVI   V2,1
0038            DIGOUT [010.....]
0039            MOVI   V2,0
0040            HALT                   ;End of this sequence section


; Quit stimulus application
0041 EQ:     'q MOVI   V2,1
0042            DIGOUT [1000000.]
0043            MOVI   V2,0
0044            HALT                   ;End of this sequence section

; Reward
0045 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0046            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0047            DIGOUT [.......1]      ;Assert that juicer is closed
0048            DIGOUT [.......0]      ;Downward pulse delivers juice
0049            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0050            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0051            JUMP   RDONE           ;Jump over REWHI to a HALT
0052 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0053            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0054            DIGOUT [.......0]      ;Assert that juicer is closed
0055            DIGOUT [.......1]      ;Voltage High delivers juice
0056            DELAY  V1              ;Delay V1 ms, duration of reward
0057            DIGOUT [.......0]      ;Close juicer valve
0058 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0059            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0060 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0061            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0062            DIGOUT [.......1]      ;Assert that juicer is closed
0063            DIGOUT [.......0]      ;Downward pulse delivers juice
0064            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0065            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0066            JUMP   JDONE           ;Jump over JCRHI to a HALT
0067 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0068            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0069            DIGOUT [.......0]      ;Assert that juicer is closed
0070            DIGOUT [.......1]      ;Voltage High delivers juice
0071            DELAY  V1              ;Delay V1 ms, duration of reward
0072            DIGOUT [.......0]      ;Close juicer valve
0073 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0074            HALT                   ;End of this sequence section


