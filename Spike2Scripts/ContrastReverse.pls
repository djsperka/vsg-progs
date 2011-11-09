; $Id: ContrastReverse.pls,v 1.2 2011-11-09 17:58:19 jeff Exp $
; Revision 1.2  2011/11/08 jeff
; Added SafeSampleKey and multiple juicer functionality, SafeSampleKey means
; that individual calls do not have to close the juicer, also changed

                SET      1.000 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey

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

0012 ES:     'S MOVI   V2,1
0013            DIGOUT [...0..i.]      ;Signals the stimulus on
0014            MOVI   V2,0
0015            HALT   

0016 ET:     's MOVI   V2,1
0017            DIGOUT [...0.i..]      ;Signals the stimulus and fixation point off
0018            MOVI   V2,0
0019            HALT   

0020 EU:     'x MOVI   V2,1
0021            DIGOUT [...0.i..]      ;Signals the stimulus and fixation point off - same as s
0022            MOVI   V2,0
0023            HALT   

0024 ED:     'D MOVI   V2,1
0025            DIGOUT [...1....]      ;Signals the dot on
0026            MOVI   V2,0
0027            HALT   

; Reward
0028 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0029            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0030            DIGOUT [.......1]      ;Assert that juicer is closed
0031            DIGOUT [.......0]      ;Downward pulse delivers juice
0032            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0033            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0034            JUMP   RDONE           ;Jump over REWHI to a HALT
0035 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0036            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0037            DIGOUT [.......0]      ;Assert that juicer is closed
0038            DIGOUT [.......1]      ;Voltage High delivers juice
0039            DELAY  V1              ;Delay V1 ms, duration of reward
0040            DIGOUT [.......0]      ;Close juicer valve
0041 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0042            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0043 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0044            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0045            DIGOUT [.......1]      ;Assert that juicer is closed
0046            DIGOUT [.......0]      ;Downward pulse delivers juice
0047            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0048            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0049            JUMP   JDONE           ;Jump over JCRHI to a HALT
0050 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0051            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0052            DIGOUT [.......0]      ;Assert that juicer is closed
0053            DIGOUT [.......1]      ;Voltage High delivers juice
0054            DELAY  V1              ;Delay V1 ms, duration of reward
0055            DIGOUT [.......0]      ;Close juicer valve
0056 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0057            HALT                   ;End of this sequence section

0058 EQ:     'q MOVI   V2,1
0059            DIGOUT [1000000.]      ;Tells stim app to quit.
0060            MOVI   V2,0
0061            HALT   


