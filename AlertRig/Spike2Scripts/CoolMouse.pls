; $Id: CoolMouse.pls,v 1.4 2011-11-09 17:59:02 jeff Exp $
; Revision 1.4  2011/11/08 jeff
; Added multiple juicer functionality, SafeSampleKey means
; that individual calls do not have to close the juicer, also changed

                SET      0.010 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use
0000            BLT    V3,1,OPENHI     ;Branch, if V3 is 0, go to OPENHI
0001            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0002            JUMP   INITDONE        ;Jump over OPENHI to a HALT
0003 OPENHI:    DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0004 INITDONE:  HALT                   ;End of setup, rest runs only on (Safe)SampleKey

; zero out all trigger lines, close juicer
0005 ZERO:   '0 MOVI   V2,1            ;Log that sequencer is in use
0006            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
0007            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0008            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
0009 ZOPENHI:   DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0010 ZERODONE:  MOVI   V2,0            ;Log that sequencer is not in use
0011            HALT                   ;End of this sequence section

0012 FIXON:  'F MOVI   V2,1            ;Log that sequencer is in use
0013            DIGOUT [......1.]      ;Signals the fixation point on
0014            MOVI   V2,0            ;Log that sequencer is not in use
0015            HALT                   ;End of this sequence section

0016 FIXOFF: 'f MOVI   V2,1            ;Log that sequencer is in use
0017            DIGOUT [......0.]      ;Signals the fixation point off
0018            MOVI   V2,0            ;Log that sequencer is not in use
0019            HALT                   ;End of this sequence section

; Quit stimulus application
0020 EQ:     'q MOVI   V2,1
0021            DIGOUT [1000000.]
0022            MOVI   V2,0
0023            HALT                   ;End of this sequence section


; Reward
0024 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0025            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0026            DIGOUT [.......1]      ;Assert that juicer is closed
0027            DIGOUT [.......0]      ;Downward pulse delivers juice
0028            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0029            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0030            JUMP   RDONE           ;Jump over REWHI to a HALT
0031 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0032            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0033            DIGOUT [.......0]      ;Assert that juicer is closed
0034            DIGOUT [.......1]      ;Voltage High delivers juice
0035            DELAY  V1              ;Delay V1 ms, duration of reward
0036            DIGOUT [.......0]      ;Close juicer valve
0037 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0038            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0039 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0040            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0041            DIGOUT [.......1]      ;Assert that juicer is closed
0042            DIGOUT [.......0]      ;Downward pulse delivers juice
0043            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0044            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0045            JUMP   JDONE           ;Jump over JCRHI to a HALT
0046 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0047            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0048            DIGOUT [.......0]      ;Assert that juicer is closed
0049            DIGOUT [.......1]      ;Voltage High delivers juice
0050            DELAY  V1              ;Delay V1 ms, duration of reward
0051            DIGOUT [.......0]      ;Close juicer valve
0052 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0053            HALT                   ;End of this sequence section
