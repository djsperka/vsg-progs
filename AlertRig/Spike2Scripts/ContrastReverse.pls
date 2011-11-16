; $Id: ContrastReverse.pls,v 1.3 2011-11-16 00:48:00 jeff Exp $
; Revision 1.2  2011/11/08 jeff
; Added SafeSampleKey and multiple juicer functionality, SafeSampleKey means
; that individual calls do not have to close the juicer, also changed

                SET      1.000 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey


; zero out all trigger lines, close juicer
0000 ZERO:   '0 MOVI   V2,1            ;Log that sequencer is in use
0001            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
0002            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0003            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
0004 ZOPENHI:   DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0005 ZERODONE:  MOVI   V2,0            ;Log that sequencer is not in use
0006            HALT                   ;End of this sequence section

0007 ES:     'S MOVI   V2,1
0008            DIGOUT [...0..i.]      ;Signals the stimulus on
0009            MOVI   V2,0
0010            HALT   

0011 ET:     's MOVI   V2,1
0012            DIGOUT [...0.i..]      ;Signals the stimulus and fixation point off
0013            MOVI   V2,0
0014            HALT   

0015 EU:     'x MOVI   V2,1
0016            DIGOUT [...0.i..]      ;Signals the stimulus and fixation point off - same as s
0017            MOVI   V2,0
0018            HALT   

0019 ED:     'D MOVI   V2,1
0020            DIGOUT [...1....]      ;Signals the dot on
0021            MOVI   V2,0
0022            HALT   

; Reward
0023 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0024            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0025            DIGOUT [.......1]      ;Assert that juicer is closed
0026            DIGOUT [.......0]      ;Downward pulse delivers juice
0027            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0028            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0029            JUMP   RDONE           ;Jump over REWHI to a HALT
0030 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0031            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0032            DIGOUT [.......0]      ;Assert that juicer is closed
0033            DIGOUT [.......1]      ;Voltage High delivers juice
0034            DELAY  V1              ;Delay V1 ms, duration of reward
0035            DIGOUT [.......0]      ;Close juicer valve
0036 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0037            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0038 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0039            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0040            DIGOUT [.......1]      ;Assert that juicer is closed
0041            DIGOUT [.......0]      ;Downward pulse delivers juice
0042            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0043            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0044            JUMP   JDONE           ;Jump over JCRHI to a HALT
0045 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0046            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0047            DIGOUT [.......0]      ;Assert that juicer is closed
0048            DIGOUT [.......1]      ;Voltage High delivers juice
0049            DELAY  V1              ;Delay V1 ms, duration of reward
0050            DIGOUT [.......0]      ;Close juicer valve
0051 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0052            HALT                   ;End of this sequence section

0053 EQ:     'q MOVI   V2,1
0054            DIGOUT [1000000.]      ;Tells stim app to quit.
0055            MOVI   V2,0
0056            HALT   


