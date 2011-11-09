; $Id: AttaJoy.pls,v 1.5 2011-11-09 17:56:23 jeff Exp $
; $Id: AttaJoy.pls,v 1.5 2011-11-09 17:56:23 jeff Exp $
; Revision 1.5  2011/11/07 jeff
; Added SafeSampleKey and multiple juicer functionality, SafeSampleKey means
; that individual calls do not have to close the juicer, also changed


                SET      1.000 1 0     ;1ms per seq step, DAC scale=1, DAC offset=0
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

0012 EQ:     'q MOVI   V2,1
0013            DIGOUT [1111....]      ;exit program
0014            MOVI   V2,0
0015            HALT   

0016 EF:     'F MOVI   V2,1
0017            DIGOUT [.....01.]      ;Signals the fixation point on
0018            MOVI   V2,0
0019            HALT   

0020 EG:     'G MOVI   V2,1
0021            DIGOUT [.....10.]      ;Signals the fixation point on
0022            MOVI   V2,0
0023            HALT   

0024 EW:     'W MOVI   V2,1
0025            DIGOUT [...1....]      ;cue circle on
0026            MOVI   V2,0
0027            HALT   

0028 EY:     'w MOVI   V2,1
0029            DIGOUT [...0....]      ;cue circle off
0030            MOVI   V2,0
0031            HALT   

0032 ES:     'S MOVI   V2,1
0033            DIGOUT [....1...]      ;stim on
0034            MOVI   V2,0
0035            HALT   

0036 EU:     's MOVI   V2,1
0037            DIGOUT [....0...]      ;stim off
0038            MOVI   V2,0
0039            HALT   

0040 E2:     'X MOVI   V2,1
0041            DIGOUT [0000011.]      ;all off
0042            MOVI   V2,0
0043            HALT   

0044 EC:     'C MOVI   V2,1
0045            DIGOUT [..1.....]      ;stimulus contrast UP
0046            MOVI   V2,0
0047            HALT   

0048 EN:     'c MOVI   V2,1
0049            DIGOUT [.1......]      ;stimulus contrast DOWN
0050            MOVI   V2,0
0051            HALT   

0052 ED:     'D MOVI   V2,1
0053            DIGOUT [.11.....]      ;distractor contrast UP
0054            MOVI   V2,0
0055            HALT   

0056 EP:     'd MOVI   V2,1
0057            DIGOUT [1.......]      ;distractor contrast DOWN
0058            MOVI   V2,0
0059            HALT   

; Reward
0060 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0061            DELAY  V4              ;Delay, used in ATTAJOY
0062            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0063            DIGOUT [.......1]      ;Assert that juicer is closed
0064            DIGOUT [.......0]      ;Downward pulse delivers juice
0065            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0066            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0067            JUMP   RDONE           ;Jump over REWHI to a HALT
0068 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0069            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0070            DIGOUT [.......0]      ;Assert that juicer is closed
0071            DIGOUT [.......1]      ;Voltage High delivers juice
0072            DELAY  V1              ;Delay V1 ms, duration of reward
0073            DIGOUT [.......0]      ;Close juicer valve
0074 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0075            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0076 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0077            DELAY  V4              ;Delay, used in ATTAJOY
0078            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0079            DIGOUT [.......1]      ;Assert that juicer is closed
0080            DIGOUT [.......0]      ;Downward pulse delivers juice
0081            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0082            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0083            JUMP   JDONE           ;Jump over JCRHI to a HALT
0084 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0085            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0086            DIGOUT [.......0]      ;Assert that juicer is closed
0087            DIGOUT [.......1]      ;Voltage High delivers juice
0088            DELAY  V1              ;Delay V1 ms, duration of reward
0089            DIGOUT [.......0]      ;Close juicer valve
0090 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0091            HALT                   ;End of this sequence section



