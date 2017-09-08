; $Id: CoolMouse.pls,v 1.5 2011/11/16 00:48:00 jeff Exp $
; Revision 1.4  2011/11/08 jeff
; Added multiple juicer functionality, SafeSampleKey means
; that individual calls do not have to close the juicer, also changed

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

0007 FIXON:  'F MOVI   V2,1            ;Log that sequencer is in use
0008            DIGOUT [......1.]      ;Signals the fixation point on
0009            MOVI   V2,0            ;Log that sequencer is not in use
0010            HALT                   ;End of this sequence section

0011 FIXOFF: 'f MOVI   V2,1            ;Log that sequencer is in use
0012            DIGOUT [......0.]      ;Signals the fixation point off
0013            MOVI   V2,0            ;Log that sequencer is not in use
0014            HALT                   ;End of this sequence section

; Quit stimulus application
0015 EQ:     'q MOVI   V2,1
0016            DIGOUT [1000000.]
0017            MOVI   V2,0
0018            HALT                   ;End of this sequence section


; Reward
0019 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0020            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0021            DIGOUT [.......1]      ;Assert that juicer is closed
0022            DIGOUT [.......0]      ;Downward pulse delivers juice
0023            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0024            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0025            JUMP   RDONE           ;Jump over REWHI to a HALT
0026 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0027            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0028            DIGOUT [.......0]      ;Assert that juicer is closed
0029            DIGOUT [.......1]      ;Voltage High delivers juice
0030            DELAY  V1              ;Delay V1 ms, duration of reward
0031            DIGOUT [.......0]      ;Close juicer valve
0032 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0033            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0034 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0035            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0036            DIGOUT [.......1]      ;Assert that juicer is closed
0037            DIGOUT [.......0]      ;Downward pulse delivers juice
0038            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0039            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0040            JUMP   JDONE           ;Jump over JCRHI to a HALT
0041 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0042            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0043            DIGOUT [.......0]      ;Assert that juicer is closed
0044            DIGOUT [.......1]      ;Voltage High delivers juice
0045            DELAY  V1              ;Delay V1 ms, duration of reward
0046            DIGOUT [.......0]      ;Close juicer valve
0047 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0048            HALT                   ;End of this sequence section
