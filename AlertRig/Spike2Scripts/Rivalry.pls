; $Id: Rivalry.pls,v 1.8 2011-11-16 00:48:00 jeff Exp $
; Revision 1.7  2011/11/08 jeff
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



; Left stimulus request
0007 EH:     'l MOVI   V2,1
0008            DIGOUT [......1.]
0009            MOVI   V2,0
0010            HALT   

; Right stimulus request
0011 EV:     'r MOVI   V2,1
0012            DIGOUT [.....1..]
0013            MOVI   V2,0
0014            HALT   

; Fixation point request
0015 EF:     'F MOVI   V2,1
0016            DIGOUT [...1....]
0017            MOVI   V2,0
0018            HALT   

; toggle stimulus request
0019 EZ:     'z MOVI   V2,1
0020            DIGOUT [....i...]
0021            MOVI   V2,0
0022            HALT   

; Stimulus off request
0023 EX:     'X MOVI   V2,1
0024            DIGOUT [...0.00.]
0025            MOVI   V2,0
0026            HALT   

; Quit stimulus application
0027 EQ:     'q MOVI   V2,1
0028            DIGOUT [1000000.]
0029            MOVI   V2,0
0030            HALT                   ;End of this sequence section

; Reward
0031 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0032            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0033            DIGOUT [.......1]      ;Assert that juicer is closed
0034            DIGOUT [.......0]      ;Downward pulse delivers juice
0035            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0036            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0037            JUMP   RDONE           ;Jump over REWHI to a HALT
0038 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0039            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0040            DIGOUT [.......0]      ;Assert that juicer is closed
0041            DIGOUT [.......1]      ;Voltage High delivers juice
0042            DELAY  V1              ;Delay V1 ms, duration of reward
0043            DIGOUT [.......0]      ;Close juicer valve
0044 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0045            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0046 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0047            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0048            DIGOUT [.......1]      ;Assert that juicer is closed
0049            DIGOUT [.......0]      ;Downward pulse delivers juice
0050            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0051            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0052            JUMP   JDONE           ;Jump over JCRHI to a HALT
0053 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0054            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0055            DIGOUT [.......0]      ;Assert that juicer is closed
0056            DIGOUT [.......1]      ;Voltage High delivers juice
0057            DELAY  V1              ;Delay V1 ms, duration of reward
0058            DIGOUT [.......0]      ;Close juicer valve
0059 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0060            HALT                   ;End of this sequence section


