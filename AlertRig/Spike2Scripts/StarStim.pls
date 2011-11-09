; $Id: StarStim.pls,v 1.4 2011-11-09 18:16:34 jeff Exp $
; Revision 1.4  2011/11/08 jeff
; Added multiple juicer functionality

                SET      0.010 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use

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

0016 FIXOFF: 'f MOVI   V2,1            ;Log that sequencer is in use
0017            DIGOUT [......0.]      ;Signals the fixation point off
0018            MOVI   V2,0            ;Log that sequencer is not in use
0019            HALT                   ;End of this sequence section

0020 STIMON: 'S MOVI   V2,1            ;Log that sequencer is in use
0021            DIGOUT [.....1..]      ;Signals the stimuli on
0022            MOVI   V2,0            ;Log that sequencer is not in use
0023            HALT                   ;End of this sequence section

0024 STIMOFF: 's MOVI  V2,1            ;Log that sequencer is in use
0025            DIGOUT [.....0..]      ;Signals the stimuli off
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

0036 CLEAR:  'X MOVI   V2,1            ;Log that sequencer is in use
0037            DIGOUT [.....00.]      ;Clears fixation and stim triggers
0038            MOVI   V2,0            ;Log that sequencer is not in use
0039            HALT                   ;End of this sequence section

; Reward
0040 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0041            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0042            DIGOUT [.......1]      ;Assert that juicer is closed
0043            DIGOUT [.......0]      ;Downward pulse delivers juice
0044            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0045            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0046            JUMP   RDONE           ;Jump over REWHI to a HALT
0047 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0048            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0049            DIGOUT [.......0]      ;Assert that juicer is closed
0050            DIGOUT [.......1]      ;Voltage High delivers juice
0051            DELAY  V1              ;Delay V1 ms, duration of reward
0052            DIGOUT [.......0]      ;Close juicer valve
0053 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0054            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0055 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0056            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0057            DIGOUT [.......1]      ;Assert that juicer is closed
0058            DIGOUT [.......0]      ;Downward pulse delivers juice
0059            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0060            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0061            JUMP   JDONE           ;Jump over JCRHI to a HALT
0062 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0063            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0064            DIGOUT [.......0]      ;Assert that juicer is closed
0065            DIGOUT [.......1]      ;Voltage High delivers juice
0066            DELAY  V1              ;Delay V1 ms, duration of reward
0067            DIGOUT [.......0]      ;Close juicer valve
0068 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0069            HALT                   ;End of this sequence section
