; $Id: Fixation.PLS,v 1.11 2016-03-04 22:55:20 devel Exp $

            SET      1.000 1 0     ;Get rate & scaling OK
            VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey


; zero out all trigger lines, close juicer
ZERO:   '0  MOVI   V2,1            ;Log that sequencer is in use
            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
            DIGLOW [00000000]      ;Also zero low bits
            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
ZOPENHI:    DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
            DIGLOW [00000000]      ;Also zero low bits
ZERODONE:   MOVI   V2,0            ;Log that sequencer is not in use
            HALT                   ;End of this sequence section


ES:     'S  MOVI   V2,1
            DIGOUT V1
            MOVI   V2,0
            HALT   

EF:     'F  MOVI   V2,1
            DIGOUT [......1.]
            MOVI   V2,0
            HALT                   ;End of this sequence section

EX:     'X  MOVI   V2,1
            DIGOUT [......0.]
            MOVI   V2,0
            HALT                   ;End of this sequence section

EA:     'a  MOVI   V2,1
            DIGOUT [....i...]
            MOVI   V2,0
            HALT                   ;End of this sequence section

EV:     'V  MOVI   V2,1
            DIGOUT [.....0.1]
            MOVI   V2,0
            HALT                   ;End of this sequence section

EH:     'H  MOVI   V2,1
            DIGOUT [.....1.0]
            MOVI   V2,0
            HALT                   ;End of this sequence section

EC:     'C  MOVI   V2,1
            DIGOUT [.....1.1]
            MOVI   V2,0
            HALT                   ;End of this sequence section

EN:     'N  MOVI   V2,1
            DIGOUT [.....0.0]
            MOVI   V2,0
            HALT                   ;End of this sequence section


EQ:     'Q  MOVI   V2,1
            DIGOUT [0001000.]
            MOVI   V2,0
            HALT                   ;End of this sequence section

; Reward
REW:    'R  MOVI   V2,1            ;Log that sequencer is in use
            BLT    V3,1,REWHI      ;Branch, if V3 is 0, go to REWHI
            DIGOUT [.......1]      ;Assert that juicer is closed
            DIGOUT [.......0]      ;Downward pulse delivers juice
            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
            JUMP   RDONE           ;Jump over REWHI to a HALT
REWHI:      BLE    V1,0,RDONE      ;Skip if V1 is <= 0
            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
            DIGOUT [.......0]      ;Assert that juicer is closed
            DIGOUT [.......1]      ;Voltage High delivers juice
            DELAY  V1              ;Delay V1 ms, duration of reward
            DIGOUT [.......0]      ;Close juicer valve
RDONE:      MOVI   V2,0            ;Log that sequencer is not in use
            HALT                   ;End of this sequence section

; Reward, compatible with 'J'uicer command from Farran's rig, just do same thing as 'R'
JCR:    'J  MOVI   V2,1            ;Log that sequencer is in use
            BLT    V3,1,JCRHI      ;Branch, if V3 is 0, go to JCRHI
            DIGOUT [.......1]      ;Assert that juicer is closed
            DIGOUT [.......0]      ;Downward pulse delivers juice
            DELAY  s(0.005)-1      ;Delay for 5 ms for adequate pulse width
            DIGOUT [.......1]      ;End downward pulse, juicer will close on its own
            JUMP   JDONE           ;Jump over JCRHI to a HALT
JCRHI:      BLE    V1,0,JDONE      ;Skip if V1 is <= 0
            MULI   V1,ms(1)        ;convert V1 from ms to clock ticks
            DIGOUT [.......0]      ;Assert that juicer is closed
            DIGOUT [.......1]      ;Voltage High delivers juice
            DELAY  V1              ;Delay V1 ms, duration of reward
            DIGOUT [.......0]      ;Close juicer valve
JDONE:      MOVI   V2,0            ;Log that sequencer is not in use
            HALT                   ;End of this sequence section

; LED commands for fixation LED
LEDON:  'L  MOVI   V2,1            ;Log that sequencer is in use
            DIGLOW [1.......]      ;Turn on LED - don't know which digout yet
            JUMP   LDONE           ;Jump over LEDOFF to a HALT
LEDOFF: 'M  MOVI   V2,1            ;Log that sequencer is in use
            DIGLOW [0.......]      ;Turn off LED - don't know which digout yet
LDONE:      MOVI   V2,0            ;Log that sequencer is not in use
            HALT                   ;End of this sequence section