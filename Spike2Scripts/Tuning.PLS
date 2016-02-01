; $Id: Tuning.PLS,v 1.18 2016-02-01 19:56:54 devel Exp $
;

                SET      0.010 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey
                                       ;V3 determines whether juicer is open-high or open-low
                                       ;V4 is a voltage out variable for setting level on the opto
                                       ;V5 is used for rivalry, indicates time of stimulus change
                                       ;V6 is the number of pulses, for pulsed optogenetics
                                       ;V7 is the duration of the pulse, for pulsed optogenetics
                                       ;V8 is the interpulse interval, for pulsed optogenetics
                                       ;V9 is the pre-pulse stimulus duration, for pulsed opto
                                       ;V10 is the post-pulse stimulus duration, for pulsed opto

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
0009            BEQ    V5,1,VWAIT
0010            MOVI   V2,0            ;Log that sequencer is not in use
0011            HALT                   ;End of this sequence section

0012 FIXOFF: 'f MOVI   V2,1            ;Log that sequencer is in use
0013            DIGOUT [......0.]      ;Signals the fixation point off
0014            MOVI   V2,0            ;Log that sequencer is not in use
0015            HALT                   ;End of this sequence section

0016 STIMON: 'S MOVI   V2,1            ;Log that sequencer is in use
0017            DIGOUT [.....1..]      ;Signals the stimuli on
0018            BEQ    V5,1,VWAIT
0019            MOVI   V2,0            ;Log that sequencer is not in use
0020            HALT                   ;End of this sequence section

0021 STIMOFF: 's MOVI  V2,1            ;Log that sequencer is in use
0022            DIGOUT [.....0..]      ;Signals the stimuli off
0023            MOVI   V2,0            ;Log that sequencer is not in use
0024            HALT                   ;End of this sequence section

0025 STIMADV: 'a MOVI  V2,1            ;Log that sequencer is in use
0026            DIGOUT [....i...]      ;Signals the tuned parameter to advance
;0028            BEQ    V5,1,VWAIT     ;no branch for triggering, cmdline to fixstim should 
;                     ;omit 'a' from V arg, e.g. "-V 128,FS" , not "-V 128,FSa" or "-V 128"
0027            MOVI   V2,0            ;Log that sequencer is not in use
0028            HALT                   ;End of this sequence section

0029 TOGGHOLE: 'v MOVI V2,1            ;Log that sequencer is in use
0030            DIGOUT [..i.....]      ;Signals the tuned parameter to advance
0031            MOVI   V2,0            ;Log that sequencer is not in use
0032            HALT                   ;End of this sequence section

0033 TOGDONUT: 'u MOVI V2,1            ;Log that sequencer is in use
0034            DIGOUT [.i......]      ;Signals the tuned parameter to advance, this doesn't work
0035            MOVI   V2,0            ;Log that sequencer is not in use
0036            HALT                   ;End of this sequence section

0037 QUIT:   'Q MOVI   V2,1            ;Log that sequencer is in use
0038            DIGOUT [...1....]      ;tells stim we are finished. Program should exit.
0039            MOVI   V2,0            ;Log that sequencer is not in use
0040            HALT                   ;End of this sequence section

0041 CLEAR:  'X MOVI   V2,1            ;Log that sequencer is in use
0042            DIGOUT [.....00.]      ;Clears fixation and stim triggers
;0045            BEQ    V5,1,VWAIT      ; no branch for triggering, cmdline to fixstim should 
;                       ;omit 'X' from V arg, e.g. "-V 128,FS" , not "-V 128,FSX" or "-V 128"
0043            MOVI   V2,0            ;Log that sequencer is not in use
0044            HALT                   ;End of this sequence section

0045 VWAIT:     WAIT   [......11]
0046 TRIGGER: 'V MOVI  V2,1            ;Log that sequencer is in use
0047            DIGOUT [1.......]      ;Trigger to present stim chg.
0048            DELAY  ms(50)
;0050            DELAY  ms(100)
0049            DIGOUT [0.......]
0050            MOVI   V2,0            ;Log that sequencer is not in use
0051            HALT                   ;End of this sequence section


; Reward
0052 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0053            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0054            DIGOUT [.......1]      ;Assert that juicer is closed
0055            DIGOUT [.......0]      ;Downward pulse delivers juice
0056            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0057            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0058            JUMP   RDONE           ;Jump over REWHI to a HALT
0059 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0060            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0061            DIGOUT [.......0]      ;Assert that juicer is closed
0062            DIGOUT [.......1]      ;Voltage High delivers juice
0063            DELAY  V1              ;Delay V1 ms, duration of reward
0064            DIGOUT [.......0]      ;Close juicer valve
0065 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0066            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0067 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0068            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0069            DIGOUT [.......1]      ;Assert that juicer is closed
0070            DIGOUT [.......0]      ;Downward pulse delivers juice
0071            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0072            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0073            JUMP   JDONE           ;Jump over JCRHI to a HALT
0074 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0075            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0076            DIGOUT [.......0]      ;Assert that juicer is closed
0077            DIGOUT [.......1]      ;Voltage High delivers juice
0078            DELAY  V1              ;Delay V1 ms, duration of reward
0079            DIGOUT [.......0]      ;Close juicer valve
0080 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0081            HALT                   ;End of this sequence section

; Simple optogenetics, just blanketing the stimulus 
0082 OPTOON: 'O MOVI   V2,1            ;Log that sequencer is in use
0083            DAC    0,V4            ;Set DAC0 to the value in V4
0084            MOVI   V2,0            ;Log that sequencer is not in use
0085            HALT                   ;End of this sequence section

0086 OPTOOFF: 'o MOVI  V2,1            ;Log that sequencer is in use
0087            DAC    0,0             ;Set DAC0 to 0
0088            MOVI   V2,0            ;Log that sequencer is not in use
0089            HALT                   ;End of this sequence section

0090 NOOPTO: 'N HALT                   ;Simply reserving this sequence to do nothing

0091 NOOPTOFF: 'n HALT                 ;Simply reserving this sequence to do nothing

; Complicated optogenetics, pulsing
0092 PULSOPTO: 'P MOVI V2,1            ;Log that sequencer is in use
0093            DIGOUT [.....1..]      ;Signals the stimuli on
0094            DELAY  V9              ;Wait until first pulse on, subtract 1 tick
0095 PULSEON:   DAC    0,V4            ;Set DAC0 to the value in V4
0096            MARK   1               ;Mark onset of laser on digital marker channel
0097            DELAY  V7              ;Wait until pulse off time, subtract 2 ticks
0098            DAC    0,0             ;Set DAC0 to 0
0099            MARK   0               ;Mark offset of laser on digital marker channel
0100            DBNZ   V6,PULSEIPI     ;Decrement #pulses, go to IPI if more
0101            JUMP   PULSEXIT        ;Jump to wait/exit from pulse/stim
0102 PULSEIPI:  DELAY  V8              ;Wait through IPI, subtract 3 ticks
0103            JUMP   PULSEON         ;Start next pulse
0104 PULSEXIT:  DELAY  V10             ;Wait until stim should go off, subtract 3 ticks
0105            DIGOUT [.....0..]      ;Signals the stimuli off
0106            MOVI   V2,0            ;Log that sequencer is not in use
0107            HALT                   ;End of this sequence section