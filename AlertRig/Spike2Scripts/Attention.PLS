; $Id: Attention.PLS,v 1.7 2012-12-20 20:05:07 devel Exp $

                SET      0.010 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey

                VAR    V45,LoopC=0     ;Define variable for section loops
                VAR    V46,RampC=0     ;Define variable for ramp loops
                VAR    V47,DelayC=0    ;Define variable for delay loops
                VAR    V48,Delay2=0    ;and another one
                VAR    V49,Delay3=0    ;and another one
                VAR    V50,Delay4=0    ;and another one
                VAR    V51,Delay5=0    ;and another one

;E0:     '0  DIGOUT [00000001]
;            DELAY  s(0.996)-1
;            HALT                   ;End of this sequence section

; zero out all trigger lines, close juicer
0000 ZERO:   '0 MOVI   V2,1            ;Log that sequencer is in use
0001            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
0002            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0003            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
0004 ZOPENHI:   DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0005 ZERODONE:  MOVI   V2,0            ;Log that sequencer is not in use
0006            HALT                   ;End of this sequence section

0007 EQ:     'q MOVI   V2,1            ;Log that sequencer is in use
0008            DIGOUT [1111....]      ;exit program
;0009            DELAY  s(0.996)-1
0010            MOVI   V2,0            ;Log that sequencer is not in use
0011            HALT   

0012 EF:     'F MOVI   V2,1            ;Log that sequencer is in use
0013            DIGOUT [...0.01.]      ;Signals the fixation point on
;0014            DELAY  s(0.996)-1
0015            MOVI   V2,0            ;Log that sequencer is not in use
0016            HALT   

0017 E1:     'f MOVI   V2,1            ;Log that sequencer is in use
0018            DIGOUT [...1.11.]      ;Signals the fixation point off
;0019            DELAY  s(0.996)-1
0020            MOVI   V2,0            ;Log that sequencer is not in use
0021            HALT   

0022 EG:     'G MOVI   V2,1            ;Log that sequencer is in use
0023            DIGOUT [...0.10.]      ;Signals the fixation point on
;0024            DELAY  s(0.996)-1
0025            MOVI   V2,0            ;Log that sequencer is not in use
0026            HALT   

0027 EH:     'g MOVI   V2,1            ;Log that sequencer is in use
0028            DIGOUT [...0.00.]      ;Signals the fixation point off
;0029            DELAY  s(0.996)-1
0030            MOVI   V2,0            ;Log that sequencer is not in use
0031            HALT   

0032 EW:     'W MOVI   V2,1            ;Log that sequencer is in use
0033            DIGOUT [...1.00.]      ;Signals the fixation point on
;0034            DELAY  s(0.996)-1
0035            MOVI   V2,0            ;Log that sequencer is not in use
0036            HALT   

0037 EY:     'w MOVI   V2,1            ;Log that sequencer is in use
0038            DIGOUT [...0.00.]      ;Signals the fixation point off
;0039            DELAY  s(0.996)-1
0040            MOVI   V2,0            ;Log that sequencer is not in use
0041            HALT   

0042 ES:     'S MOVI   V2,1            ;Log that sequencer is in use
0043            DIGOUT [....1...]      ;Signals the stimuli and answer pts on
;0044            DELAY  s(0.996)-1
0045            MOVI   V2,0            ;Log that sequencer is not in use
0046            HALT   

0047 ET:     's MOVI   V2,1            ;Log that sequencer is in use
0048            DIGOUT [....0...]      ;Signals the stimuli off (answer points remain on)
;0049            DELAY  s(0.996)-1
0050            MOVI   V2,0            ;Log that sequencer is not in use
0051            HALT   

0052 E2:     'X MOVI   V2,1            ;Log that sequencer is in use
0053            DIGOUT [0001011.]      ;Signals all off
;0054            DELAY  s(0.996)-1
0055            MOVI   V2,0            ;Log that sequencer is not in use
0056            HALT   

0057 EC:     'C MOVI   V2,1            ;Log that sequencer is in use
0058            DIGOUT [..1.....]      ;stimulus contrast UP
;0059            DELAY  s(0.996)-1
0060            MOVI   V2,0            ;Log that sequencer is not in use
0061            HALT   

0062 EN:     'c MOVI   V2,1            ;Log that sequencer is in use
0063            DIGOUT [.1......]      ;stimulus contrast DOWN
;0064            DELAY  s(0.996)-1
0065            MOVI   V2,0            ;Log that sequencer is not in use
0066            HALT   

0067 ED:     'D MOVI   V2,1            ;Log that sequencer is in use
0068            DIGOUT [.11.....]      ;distractor contrast UP
;0069            DELAY  s(0.996)-1
0070            MOVI   V2,0            ;Log that sequencer is not in use
0071            HALT   

0072 EP:     'd MOVI   V2,1            ;Log that sequencer is in use
0073            DIGOUT [1.......]      ;distractor contrast DOWN
;0074            DELAY  s(0.996)-1
0075            MOVI   V2,0            ;Log that sequencer is not in use
0076            HALT   

;0075 EJ:     'J MOVI   V2,1            ;Log that sequencer is in use
;0076            DIGOUT [.......1]      ;Administers juice
;0077            DIGOUT [.......0]
;0078            DELAY  s(0.005)-1
;0079            DIGOUT [.......1]
;0080            DELAY  s(0.989)-1
;0081            MOVI   V2,0            ;Log that sequencer is not in use
;0082            HALT   

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0077 JCR:    'R MOVI   V2,1            ;Log that sequencer is in use
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
