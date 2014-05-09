; $Id: Tuning.PLS,v 1.12 2014-05-09 17:53:21 jeff Exp $
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

0027 TOGGHOLE: 'u MOVI V2,1            ;Log that sequencer is in use
0028            DIGOUT [...i....]      ;Signals the tuned parameter to advance
0029            MOVI   V2,0            ;Log that sequencer is not in use
0030            HALT                   ;End of this sequence section

0031 TOGDONUT: 'v MOVI V2,1            ;Log that sequencer is in use
0032            DIGOUT [..i.....]      ;Signals the tuned parameter to advance
0033            MOVI   V2,0            ;Log that sequencer is not in use
0034            HALT                   ;End of this sequence section

0035 QUIT:   'Q MOVI   V2,1            ;Log that sequencer is in use
0036            DIGOUT [...1....]      ;tells stim we are finished. Program should exit.
0037            MOVI   V2,0            ;Log that sequencer is not in use
0038            HALT                   ;End of this sequence section

0039 CLEAR:  'X MOVI   V2,1            ;Log that sequencer is in use
0040            DIGOUT [.....00.]      ;Clears fixation and stim triggers
0041            MOVI   V2,0            ;Log that sequencer is not in use
0042            HALT                   ;End of this sequence section

; Reward
0043 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0044            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0045            DIGOUT [.......1]      ;Assert that juicer is closed
0046            DIGOUT [.......0]      ;Downward pulse delivers juice
0047            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0048            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0049            JUMP   RDONE           ;Jump over REWHI to a HALT
0050 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0051            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0052            DIGOUT [.......0]      ;Assert that juicer is closed
0053            DIGOUT [.......1]      ;Voltage High delivers juice
0054            DELAY  V1              ;Delay V1 ms, duration of reward
0055            DIGOUT [.......0]      ;Close juicer valve
0056 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0057            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0058 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0059            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0060            DIGOUT [.......1]      ;Assert that juicer is closed
0061            DIGOUT [.......0]      ;Downward pulse delivers juice
0062            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0063            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0064            JUMP   JDONE           ;Jump over JCRHI to a HALT
0065 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0066            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0067            DIGOUT [.......0]      ;Assert that juicer is closed
0068            DIGOUT [.......1]      ;Voltage High delivers juice
0069            DELAY  V1              ;Delay V1 ms, duration of reward
0070            DIGOUT [.......0]      ;Close juicer valve
0071 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0072            HALT                   ;End of this sequence section

0073 OPTOON: 'O MOVI   V2,1            ;Log that sequencer is in use
0074            DAC    0,V4            ;Set DAC0 to the value in V4
0075            MOVI   V2,0            ;Log that sequencer is not in use
0076            HALT                   ;End of this sequence section

0077 OPTOOFF: 'o MOVI  V2,1            ;Log that sequencer is in use
0078            DAC    0,0             ;Set DAC0 to the value in V4
0079            MOVI   V2,0            ;Log that sequencer is not in use
0080            HALT                   ;End of this sequence section
