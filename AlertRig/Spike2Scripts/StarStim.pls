; $Id: StarStim.pls,v 1.3 2011-06-22 19:19:34 devel Exp $
;

            SET      0.010 1 0     ;Get rate & scaling OK
            DIGOUT [00000000]
            VAR    V2=0            ;V2 logs whether the sequencer is in use
            HALT

E0:     '0  MOVI V2,1
            DIGOUT [00000000]
            MOVI V2,0
            HALT                   ;End of this sequence section

EF:     'F  MOVI V2,1
            DIGOUT [......10]      ;Signals the fixation point on
            MOVI V2,0
            HALT   

E1:     'f  MOVI V2,1
            DIGOUT [......00]      ;Signals the fixation point off
            MOVI V2,0
            HALT   

ES:     'S  MOVI V2,1
            DIGOUT [.....1.0]      ;Signals the stimuli on
            MOVI V2,0
            HALT   

E2:     's  MOVI V2,1
            DIGOUT [.....0.0]      ;Signals the stimuli off
            MOVI V2,0
            HALT   

EA:     'a  MOVI V2,1
            DIGOUT [....i..0]      ;Signals the tuned parameter to advance
            MOVI V2,0
            HALT   

EQ:	'Q  MOVI V2,1
            DIGOUT [...1...0]		; tells stim we are finished. Program should exit.
            MOVI V2,0
            HALT   

EX:     'X  MOVI V2,1
            DIGOUT [.....000]      ;Clears all except for the toggled "advance" trigger
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
