; $Id: AttaJoy.pls,v 1.6 2011-11-16 00:48:00 jeff Exp $
; $Id: AttaJoy.pls,v 1.6 2011-11-16 00:48:00 jeff Exp $
; Revision 1.5  2011/11/07 jeff
; Added SafeSampleKey and multiple juicer functionality, SafeSampleKey means
; that individual calls do not have to close the juicer, also changed


                SET      1.000 1 0     ;1ms per seq step, DAC scale=1, DAC offset=0
                VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey


; zero out all trigger lines, close juicer
0000 ZERO:   '0 MOVI   V2,1            ;Log that sequencer is in use
0001            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
0002            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0003            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
0004 ZOPENHI:   DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0005 ZERODONE:  MOVI   V2,0            ;Log that sequencer is not in use
0006            HALT                   ;End of this sequence section

0007 EQ:     'q MOVI   V2,1
0008            DIGOUT [1111....]      ;exit program
0009            MOVI   V2,0
0010            HALT   

0011 EF:     'F MOVI   V2,1
0012            DIGOUT [.....01.]      ;Signals the fixation point on
0013            MOVI   V2,0
0014            HALT   

0015 EG:     'G MOVI   V2,1
0016            DIGOUT [.....10.]      ;Signals the fixation point on
0017            MOVI   V2,0
0018            HALT   

0019 EW:     'W MOVI   V2,1
0020            DIGOUT [...1....]      ;cue circle on
0021            MOVI   V2,0
0022            HALT   

0023 EY:     'w MOVI   V2,1
0024            DIGOUT [...0....]      ;cue circle off
0025            MOVI   V2,0
0026            HALT   

0027 ES:     'S MOVI   V2,1
0028            DIGOUT [....1...]      ;stim on
0029            MOVI   V2,0
0030            HALT   

0031 EU:     's MOVI   V2,1
0032            DIGOUT [....0...]      ;stim off
0033            MOVI   V2,0
0034            HALT   

0035 E2:     'X MOVI   V2,1
0036            DIGOUT [0000011.]      ;all off
0037            MOVI   V2,0
0038            HALT   

0039 EC:     'C MOVI   V2,1
0040            DIGOUT [..1.....]      ;stimulus contrast UP
0041            MOVI   V2,0
0042            HALT   

0043 EN:     'c MOVI   V2,1
0044            DIGOUT [.1......]      ;stimulus contrast DOWN
0045            MOVI   V2,0
0046            HALT   

0047 ED:     'D MOVI   V2,1
0048            DIGOUT [.11.....]      ;distractor contrast UP
0049            MOVI   V2,0
0050            HALT   

0051 EP:     'd MOVI   V2,1
0052            DIGOUT [1.......]      ;distractor contrast DOWN
0053            MOVI   V2,0
0054            HALT   

; Reward
0055 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0056            DELAY  V4              ;Delay, used in ATTAJOY
0057            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0058            DIGOUT [.......1]      ;Assert that juicer is closed
0059            DIGOUT [.......0]      ;Downward pulse delivers juice
0060            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0061            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0062            JUMP   RDONE           ;Jump over REWHI to a HALT
0063 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0064            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0065            DIGOUT [.......0]      ;Assert that juicer is closed
0066            DIGOUT [.......1]      ;Voltage High delivers juice
0067            DELAY  V1              ;Delay V1 ms, duration of reward
0068            DIGOUT [.......0]      ;Close juicer valve
0069 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0070            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0071 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0072            DELAY  V4              ;Delay, used in ATTAJOY
0073            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0074            DIGOUT [.......1]      ;Assert that juicer is closed
0075            DIGOUT [.......0]      ;Downward pulse delivers juice
0076            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0077            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0078            JUMP   JDONE           ;Jump over JCRHI to a HALT
0079 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0080            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0081            DIGOUT [.......0]      ;Assert that juicer is closed
0082            DIGOUT [.......1]      ;Voltage High delivers juice
0083            DELAY  V1              ;Delay V1 ms, duration of reward
0084            DIGOUT [.......0]      ;Close juicer valve
0085 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0086            HALT                   ;End of this sequence section



