; $Id: Rivalry.pls,v 1.7 2011-11-09 18:11:18 jeff Exp $
; Revision 1.7  2011/11/08 jeff
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



; Left stimulus request
0012 EH:     'l MOVI   V2,1
0013            DIGOUT [......1.]
0014            MOVI   V2,0
0015            HALT   

; Right stimulus request
0016 EV:     'r MOVI   V2,1
0017            DIGOUT [.....1..]
0018            MOVI   V2,0
0019            HALT   

; Fixation point request
0020 EF:     'F MOVI   V2,1
0021            DIGOUT [...1....]
0022            MOVI   V2,0
0023            HALT   

; toggle stimulus request
0024 EZ:     'z MOVI   V2,1
0025            DIGOUT [....i...]
0026            MOVI   V2,0
0027            HALT   

; Stimulus off request
0028 EX:     'X MOVI   V2,1
0029            DIGOUT [...0.00.]
0030            MOVI   V2,0
0031            HALT   

; Quit stimulus application
0032 EQ:     'q MOVI   V2,1
0033            DIGOUT [1000000.]
0034            MOVI   V2,0
0035            HALT                   ;End of this sequence section

; Reward
0036 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0037            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0038            DIGOUT [.......1]      ;Assert that juicer is closed
0039            DIGOUT [.......0]      ;Downward pulse delivers juice
0040            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0041            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0042            JUMP   RDONE           ;Jump over REWHI to a HALT
0043 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0044            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0045            DIGOUT [.......0]      ;Assert that juicer is closed
0046            DIGOUT [.......1]      ;Voltage High delivers juice
0047            DELAY  V1              ;Delay V1 ms, duration of reward
0048            DIGOUT [.......0]      ;Close juicer valve
0049 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0050            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0051 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0052            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0053            DIGOUT [.......1]      ;Assert that juicer is closed
0054            DIGOUT [.......0]      ;Downward pulse delivers juice
0055            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0056            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0057            JUMP   JDONE           ;Jump over JCRHI to a HALT
0058 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0059            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0060            DIGOUT [.......0]      ;Assert that juicer is closed
0061            DIGOUT [.......1]      ;Voltage High delivers juice
0062            DELAY  V1              ;Delay V1 ms, duration of reward
0063            DIGOUT [.......0]      ;Close juicer valve
0064 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0065            HALT                   ;End of this sequence section


