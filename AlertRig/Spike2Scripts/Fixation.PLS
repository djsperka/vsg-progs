; $Log: not supported by cvs2svn $
; Revision 1.8  2012/07/02 21:27:38  devel
; Amidst fixing fixation update
;
; Revision 1.7  2012/06/29 22:44:06  jeff
; Update for Ben's LED fixation, includes lines to operate LED
; should be on correct DIGOUT port but that's easy enough to change if not.
;
; Revision 1.6  2011/11/16 00:48:00  jeff
; Update to dual juicer capability, moving majority of work to UsreyDAQ when possible
;
; Revision 1.5  2011/11/09 17:59:49  jeff
; Update to allow use of open-high (variable open dur) OR open-low (constant open dur) juicers
;
; Revision 1.5  2011/11/08 jeff
; Added SafeSampleKey and multiple juicer functionality, SafeSampleKey means
; that individual calls do not have to close the juicer, also changed
;
; Revision 1.4  2005/03/18 19:38:51  dan
; Merged calibration functionality in to this script.
;
; Revision 1.3  2005/03/11 01:13:12  dan
; Almost there!
;
; Revision 1.2  2005/02/19 01:42:45  dan
; Changes for new Fixation script.
;
; Revision 1.1  2004/10/01 20:06:10  dan
; Modified directory locations for executables (../bin) and sequencer files (script dir)
;

                SET      1.000 1 0     ;Get rate & scaling OK
                VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey


; zero out all trigger lines, close juicer
0000 ZERO:   '0 MOVI   V2,1            ;Log that sequencer is in use
0001            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
0002            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
0003            DIGLOW [00000000]      ;Also zero low bits
0004            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
0005 ZOPENHI:   DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
0006            DIGLOW [00000000]      ;Also zero low bits
0007 ZERODONE:  MOVI   V2,0            ;Log that sequencer is not in use
0008            HALT                   ;End of this sequence section


0009 ES:     'S MOVI   V2,1
0010            DIGOUT V1
0011            MOVI   V2,0
0012            HALT   

0013 EH:     'H MOVI   V2,1
0014            DIGOUT [.....01.]
0015            MOVI   V2,0
0016            HALT                   ;End of this sequence section

0017 EL:     'L MOVI   V2,1
0018            DIGOUT [.....00.]
0019            MOVI   V2,0
0020            HALT                   ;End of this sequence section

0021 EQ:     'Q MOVI   V2,1
0022            DIGOUT [0100000.]
0023            MOVI   V2,0
0024            HALT                   ;End of this sequence section

; Reward
0025 REW:    'R MOVI   V2,1            ;Log that sequencer is in use
0026            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
0027            DIGOUT [.......1]      ;Assert that juicer is closed
0028            DIGOUT [.......0]      ;Downward pulse delivers juice
0029            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0030            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0031            JUMP   RDONE           ;Jump over REWHI to a HALT
0032 REWHI:     BLE    V1,0,RDONE      ;Skip if V1 is <= 0
0033            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0034            DIGOUT [.......0]      ;Assert that juicer is closed
0035            DIGOUT [.......1]      ;Voltage High delivers juice
0036            DELAY  V1              ;Delay V1 ms, duration of reward
0037            DIGOUT [.......0]      ;Close juicer valve
0038 RDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0039            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
0040 JCR:    'J MOVI   V2,1            ;Log that sequencer is in use
0041            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
0042            DIGOUT [.......1]      ;Assert that juicer is closed
0043            DIGOUT [.......0]      ;Downward pulse delivers juice
0044            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
0045            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
0046            JUMP   JDONE           ;Jump over JCRHI to a HALT
0047 JCRHI:     BLE    V1,0,JDONE      ;Skip if V1 is <= 0
0048            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
0049            DIGOUT [.......0]      ;Assert that juicer is closed
0050            DIGOUT [.......1]      ;Voltage High delivers juice
0051            DELAY  V1              ;Delay V1 ms, duration of reward
0052            DIGOUT [.......0]      ;Close juicer valve
0053 JDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0054            HALT                   ;End of this sequence section

; LED commands for fixation LED
0055 LEDON:  'F MOVI   V2,1            ;Log that sequencer is in use
0056            DIGLOW [1.......]      ;Turn on LED - don't know which digout yet
0057            JUMP   LDONE           ;Jump over LEDOFF to a HALT
0058 LEDOFF: 'G MOVI   V2,1            ;Log that sequencer is in use
0059            DIGLOW [0.......]      ;Turn off LED - don't know which digout yet
0060 LDONE:     MOVI   V2,0            ;Log that sequencer is not in use
0061            HALT                   ;End of this sequence section