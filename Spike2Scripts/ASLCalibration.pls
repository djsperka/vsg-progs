; $Id: ASLCalibration.pls,v 1.3 2011-06-22 19:19:34 devel Exp $

            SET      0.010 1 0     ;Get rate & scaling OK
            DIGOUT [00000000]
            VAR    V2=0            ;V2 logs whether the sequencer is in use
            HALT

; zero out all trigger lines
E0:     '0  MOVI V2,1
            DIGOUT [00000000]
            MOVI V2,0
            HALT                   ;End of this sequence section

EQ:     'q  MOVI V2,1
            DIGOUT [.1......]      ;exit program
            MOVI V2,0
            HALT   

EM:     'M  MOVI V2,1
            DIGOUT [......1.]      ;Signals master fixation point on
            MOVI V2,0
            HALT   

EMM:    'm  MOVI V2,1
            DIGOUT [......0.]      ;Signals master fixation point off
            MOVI V2,0
            HALT   

ES:     'S  MOVI V2,1
            DIGOUT [.....1..]      ;Signals master fixation point on
            MOVI V2,0
            HALT   

ESS:    's  MOVI V2,1
            DIGOUT [.....0..]      ;Signals master fixation point off
            MOVI V2,0
            HALT   


; Reward
ER:     'R  MOVI V2,1
            BLE    V1,0,RDONE           ; Skip if V1 is <= 0
            MULI V1,ms(1)               ; convert V1 from ms to clock ticks
            DIGOUT [.......0]
            DIGOUT [.......1]           ; upward pulse delivers juice
            DELAY  V1                   ; Delay V1 ms
RDONE:      DIGOUT [.......0]           ; close valve
            MOVI V2,0
            HALT



