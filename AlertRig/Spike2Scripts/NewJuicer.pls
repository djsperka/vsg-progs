; $Id: NewJuicer.pls,v 1.1 2011-02-03 00:42:46 devel Exp $
;
; Sequencer file to test Ben's new juicer. 
; The sequence EJ below (start with a keyboard "J") sends a positive-going 
; ttl pulse that is held high for a specified number of ms. 
;
; Two things to know about the time the pulse is held high. First, the DELAY 
; statement stops processing of the EJ sequence for V1 clock steps. The DIGOUT
; statement prior to that also takes a clock tick, so the total time the pulse 
; is held high is actually V1+1 ticks. Second, the length of a clock tick is 
; set by the SET statement below. The first arg is the number of ms that a tick
; really takes. Lets just always set it to 1.000, OK? 
                                        
            SET      1.000 1 0          ; 1ms per seq step, DAC scale=1, DAC offset=0
            DIGOUT [00000000]           ; set all digout lines low
            HALT

; zero out all trigger lines
E0:     '0  DIGOUT [00000000]
            HALT                   ;End of this sequence section


; Reward
ER:     'R  DIGOUT [.......0]
            BLE    V1,0,RDONE           ; Skip if V1 is <= 0
            DIGOUT [.......1]           ; upward pulse delivers juice
            DELAY  V1                   ; Delay V1 ms
RDONE:      DIGOUT [.......0]           ; close valve
            HALT


