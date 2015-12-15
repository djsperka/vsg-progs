; $Id: Tuning.PLS,v 1.16 2015-12-15 00:24:46 devel Exp $
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
0010            BEQ    V5,1,VWAIT
0009            MOVI   V2,0            ;Log that sequencer is not in use
0011            HALT                   ;End of this sequence section

0012 FIXOFF: 'f MOVI   V2,1            ;Log that sequencer is in use
0013            DIGOUT [......0.]      ;Signals the fixation point off
0014            MOVI   V2,0            ;Log that sequencer is not in use
0015            HALT                   ;End of this sequence section

0016 STIMON: 'S MOVI   V2,1            ;Log that sequencer is in use
0017            DIGOUT [.....1..]      ;Signals the stimuli on
0019            BEQ    V5,1,VWAIT
0018            MOVI   V2,0            ;Log that sequencer is not in use
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
0029            HALT                   ;End of this sequence section

0030 TOGGHOLE: 'v MOVI V2,1            ;Log that sequencer is in use
0031            DIGOUT [..i.....]      ;Signals the tuned parameter to advance
0032            MOVI   V2,0            ;Log that sequencer is not in use
0033            HALT                   ;End of this sequence section

0034 TOGDONUT: 'u MOVI V2,1            ;Log that sequencer is in use
0035            DIGOUT [.i......]      ;Signals the tuned parameter to advance, this doesn't work
0036            MOVI   V2,0            ;Log that sequencer is not in use
0037            HALT                   ;End of this sequence section

0038 QUIT:   'Q MOVI   V2,1            ;Log that sequencer is in use
0039            DIGOUT [...1....]      ;tells stim we are finished. Program should exit.
0040            MOVI   V2,0            ;Log that sequencer is not in use
0041            HALT                   ;End of this sequence section

0042 CLEAR:  'X MOVI   V2,1            ;Log that sequencer is in use
0043            DIGOUT [.....00.]      ;Clears fixation and stim triggers
;0045            BEQ    V5,1,VWAIT      ; no branch for triggering, cmdline to fixstim should 
;                       ;omit 'X' from V arg, e.g. "-V 128,FS" , not "-V 128,FSX" or "-V 128"
0044            MOVI   V2,0            ;Log that sequencer is not in use
0046            HALT                   ;End of this sequence section

0047 VWAIT:     WAIT   [......11]
0048 TRIGGER: 'V MOVI  V2,1            ;Log that sequencer is in use
0049            DIGOUT [1.......]      ;Trigger to present stim chg.
0050            DELAY  ms(50)
;0050            DELAY  ms(100)
0051            DIGOUT [0.......]
0052            MOVI   V2,0            ;Log that sequencer is not in use
0053            HALT                   ;End of this sequence section


; Reward
0054 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0055            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0056            DIGOUT [.......1]      ;Assert that juicer is closed
0057            DIGOUT [.......0]      ;Downward pulse delivers juice
0058            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0059            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0060            JUMP   RDONE           ;Jump over REWHI to a HALT
0061 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0062            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0063            DIGOUT [.......0]      ;Assert that juicer is closed
0064            DIGOUT [.......1]      ;Voltage High delivers juice
0065            DELAY  V1              ;Delay V1 ms, duration of reward
0066            DIGOUT [.......0]      ;Close juicer valve
0067 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0068            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0069 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0070            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0071            DIGOUT [.......1]      ;Assert that juicer is closed
0072            DIGOUT [.......0]      ;Downward pulse delivers juice
0073            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0074            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0075            JUMP   JDONE           ;Jump over JCRHI to a HALT
0076 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0077            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0078            DIGOUT [.......0]      ;Assert that juicer is closed
0079            DIGOUT [.......1]      ;Voltage High delivers juice
0080            DELAY  V1              ;Delay V1 ms, duration of reward
0081            DIGOUT [.......0]      ;Close juicer valve
0082 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0083            HALT                   ;End of this sequence section

; Simple optogenetics, just blanketing the stimulus 
0084 OPTOON: 'O MOVI   V2,1            ;Log that sequencer is in use
0085            DAC    0,V4            ;Set DAC0 to the value in V4
0086            MOVI   V2,0            ;Log that sequencer is not in use
0087            HALT                   ;End of this sequence section

0088 OPTOOFF: 'o MOVI  V2,1            ;Log that sequencer is in use
0089            DAC    0,0             ;Set DAC0 to 0
0090            MOVI   V2,0            ;Log that sequencer is not in use
0091            HALT                   ;End of this sequence section

; Complicated optogenetics, pulsing
0092 PULSOPTO: 'P MOVI V2,1            ;Log that sequencer is in use
0093              DIGOUT [.....1..]    ;Signals the stimuli on
0094              DELAY  V9            ;Wait until first pulse on, subtract 1 tick
0095 PULSEON:     DAC    0,V4          ;Set DAC0 to the value in V4
0096              DELAY  V7            ;Wait until pulse off time, subtract 1 tick
0097              DAC    0,0           ;Set DAC0 to 0
0098              DBNZ   V6,PULSEIPI   ;Decrement #pulses, go to IPI if more
0099              JUMP   PULSEXIT      ;Jump to wait/exit from pulse/stim
0100 PULSEIPI:    DELAY  V8            ;Wait through IPI, subtract 3 ticks
0101              JUMP   PULSEON       ;Start next pulse
0102 PULSEXIT:    DELAY  V10           ;Wait until stim should go off, subtract 2 ticks
0103              DIGOUT [.....0..]    ;Signals the stimuli off
0023              MOVI   V2,0          ;Log that sequencer is not in use
0024              HALT                 ;End of this sequence section