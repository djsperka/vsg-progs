; $Id: Tuning.PLS,v 1.11 2013-03-15 18:37:12 jeff Exp $
;

                SET      0.010 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey
                                       ;V3 determines whether juicer is open-high or open-low
                                       ;V4 is a voltage out variable for setting level on the opto


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

0015 STIMON: 'S MOVI   V2,1            ;Log that sequencer is in use
0016            DIGOUT [.....1..]      ;Signals the stimuli on
0017            MOVI   V2,0            ;Log that sequencer is not in use
0018            HALT                   ;End of this sequence section

0019 STIMOFF: 's MOVI  V2,1            ;Log that sequencer is in use
0020            DIGOUT [.....0..]      ;Signals the stimuli off
0021            MOVI   V2,0            ;Log that sequencer is not in use
0022            HALT                   ;End of this sequence section

0023 STIMADV: 'a MOVI  V2,1            ;Log that sequencer is in use
0024            DIGOUT [....i...]      ;Signals the tuned parameter to advance
0025            MOVI   V2,0            ;Log that sequencer is not in use
0026            HALT                   ;End of this sequence section

0027 QUIT:   'Q MOVI   V2,1            ;Log that sequencer is in use
0028            DIGOUT [...1....]      ;tells stim we are finished. Program should exit.
0029            MOVI   V2,0            ;Log that sequencer is not in use
0030            HALT                   ;End of this sequence section

0031 CLEAR:  'X MOVI   V2,1            ;Log that sequencer is in use
0032            DIGOUT [.....00.]      ;Clears fixation and stim triggers
0033            MOVI   V2,0            ;Log that sequencer is not in use
0034            HALT                   ;End of this sequence section

; Reward
0035 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0036            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0037            DIGOUT [.......1]      ;Assert that juicer is closed
0038            DIGOUT [.......0]      ;Downward pulse delivers juice
0039            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0040            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0041            JUMP   RDONE           ;Jump over REWHI to a HALT
0042 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0043            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0044            DIGOUT [.......0]      ;Assert that juicer is closed
0045            DIGOUT [.......1]      ;Voltage High delivers juice
0046            DELAY  V1              ;Delay V1 ms, duration of reward
0047            DIGOUT [.......0]      ;Close juicer valve
0048 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0049            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0050 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0051            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0052            DIGOUT [.......1]      ;Assert that juicer is closed
0053            DIGOUT [.......0]      ;Downward pulse delivers juice
0054            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0055            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0056            JUMP   JDONE           ;Jump over JCRHI to a HALT
0057 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0058            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0059            DIGOUT [.......0]      ;Assert that juicer is closed
0060            DIGOUT [.......1]      ;Voltage High delivers juice
0061            DELAY  V1              ;Delay V1 ms, duration of reward
0062            DIGOUT [.......0]      ;Close juicer valve
0063 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0064            HALT                   ;End of this sequence section

0065 OPTOON: 'O MOVI   V2,1            ;Log that sequencer is in use
0066            DAC    0,V4            ;Set DAC0 to the value in V4
0067            MOVI   V2,0            ;Log that sequencer is not in use
0068            HALT                   ;End of this sequence section

0069 OPTOOFF: 'o MOVI  V2,1            ;Log that sequencer is in use
0070            DAC    0,0             ;Set DAC0 to the value in V4
0071            MOVI   V2,0            ;Log that sequencer is not in use
0072            HALT                   ;End of this sequence section
