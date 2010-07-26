; $Id: StarStim.pls,v 1.2 2010-07-26 17:15:31 devel Exp $
;

            SET      1.000 1 0     ;Get rate & scaling OK
            DIGOUT [00000001]
            HALT

E0:     '0  DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ;End of this sequence section

EF:     'F  DIGOUT [......11]      ;Signals the fixation point on
            DELAY  s(0.996)-1
            HALT   

E1:     'f  DIGOUT [......01]      ;Signals the fixation point off
            DELAY  s(0.996)-1
            HALT   

ES:     'S  DIGOUT [.....1.1]      ;Signals the stimuli on
            DELAY  s(0.996)-1
            HALT   

E2:     's  DIGOUT [.....0.1]      ;Signals the stimuli off
            DELAY  s(0.996)-1
            HALT   

EA:     'a  DIGOUT [....i..1]      ;Signals the tuned parameter to advance
            DELAY  s(0.996)-1
            HALT   

EQ:		'Q	DIGOUT [...1...1]		; tells stim we are finished. Program should exit.
            DELAY  s(0.996)-1
            HALT   

EJ:     'J  DIGOUT [.......1]      ;Administers juice
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT   

EX:     'X  DIGOUT [.....001]      ;Clears all except for the toggled "advance" trigger
            DELAY  s(0.996)-1
            HALT                   ;End of this sequence section
