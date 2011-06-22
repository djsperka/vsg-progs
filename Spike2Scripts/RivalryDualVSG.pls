; $Id: RivalryDualVSG.pls,v 1.2 2011-06-22 19:19:34 devel Exp $

            SET      0.010 1 0     ;Get rate & scaling OK
            DIGOUT [00000000]
            VAR    V2=0            ;V2 logs whether the sequencer is in use
            HALT

; zero out all trigger lines
E0:     '0  MOVI V2,1
            DIGOUT [00000000]
            MOVI V2,0
            HALT                   ;End of this sequence section

; Stimulus 0/0
E5:     '5  MOVI V2,1
            DIGOUT [...0101.]
            MOVI V2,0
            HALT   

; Stimulus 1/1
EA:     'A  MOVI V2,1
            DIGOUT [...1010.]
            MOVI V2,0
            HALT   

; Stimulus 0/1
E6:      '6 MOVI V2,1
            DIGOUT [...0110.]
            MOVI V2,0
            HALT   

; Stimulus 1/0
E9:      '9 MOVI V2,1
            DIGOUT [...1001.]
            MOVI V2,0
            HALT   


; Stimulus off request
EX:     'X  MOVI V2,1
            DIGOUT [0000000.]
            MOVI V2,0
            HALT   

; Turn on stim
EG:     'g  MOVI V2,1
            DIGOUT [001.....]
            MOVI V2,0
            HALT                   ;End of this sequence section

; turn off stim
EY:     'x  MOVI V2,1
            DIGOUT [010.....]
            MOVI V2,0
            HALT                   ;End of this sequence section


; Quit stimulus application
EQ:     'q  MOVI V2,1
            DIGOUT [10000000]
            MOVI V2,0
            HALT                   ;End of this sequence section

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


