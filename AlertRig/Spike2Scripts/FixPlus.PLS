; $Id: FixPlus.PLS,v 1.4 2011-11-09 18:36:32 jeff Exp $

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

0012 FIXON:  'F MOVI   V2,1            ;Log that sequencer is in use
0013            DIGOUT [......1.]      ;Signals the fixation point on
0014            MOVI   V2,0            ;Log that sequencer is not in use
0015            HALT                   ;End of this sequence section

0016 STIMON: 'S MOVI   V2,1            ;Log that sequencer is in use
0017            DIGOUT [.....1..]      ;Signals the stimuli on
0018            MOVI   V2,0            ;Log that sequencer is not in use
0019            HALT                   ;End of this sequence section

0020 STIMOFF: 's MOVI  V2,1            ;Log that sequencer is in use
0021            DIGOUT [.....0..]      ;Signals the stimuli off
0022            MOVI   V2,0            ;Log that sequencer is not in use
0023            HALT                   ;End of this sequence section

0024 CLEAR:  'X MOVI   V2,1            ;Log that sequencer is in use
0025            DIGOUT [.....00.]      ;Clears fixation and stim triggers
0026            MOVI   V2,0            ;Log that sequencer is not in use
0027            HALT                   ;End of this sequence section

0028 STIMADV: 'a MOVI  V2,1            ;Log that sequencer is in use
0029            DIGOUT [....i...]      ;Signals the tuned parameter to advance
0030            MOVI   V2,0            ;Log that sequencer is not in use
0031            HALT                   ;End of this sequence section

0032 QUIT:   'Q MOVI   V2,1            ;Log that sequencer is in use
0033            DIGOUT [...1....]      ;tells stim we are finished. Program should exit.
0034            MOVI   V2,0            ;Log that sequencer is not in use
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
