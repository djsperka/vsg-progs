; $Log: not supported by cvs2svn $

            SET    1,1,0           ; Get rate & scaling OK

            VAR    V45,LoopC=0     ; Define variable for section loops
            VAR    V46,RampC=0     ; Define variable for ramp loops
            VAR    V47,DelayC=0    ; Define variable for delay loops
            VAR    V48,Delay2=0    ;  and another one
            VAR    V49,Delay3=0    ;  and another one
            VAR    V50,Delay4=0    ;  and another one
            VAR    V51,Delay5=0    ;  and another one

E0:         DIGOUT [......01]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EH:     'H  DIGOUT [......11]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EJ:     'J  DIGOUT [.......1]
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT                   ; End of this sequence section

EL:     'L  DIGOUT [0000.001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

ES:     'S  DIGOUT [.....1.1]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section
			
EN:     'N  DIGOUT [....i..1]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EU:     'U  DIGOUT [...1...1]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section
			
ED:     'D  DIGOUT [...0...1]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section
EB:         HALT
EC:         HALT
EE:         HALT
EF:         HALT
EG:         HALT
EI:         HALT
EK:         HALT
EM:         HALT
EO:         HALT
EP:         HALT
EQ:         HALT
ER:         HALT
ET:         HALT
EV:         HALT
EW:         HALT
EX:         HALT
EY:         HALT
EZ:         HALT
