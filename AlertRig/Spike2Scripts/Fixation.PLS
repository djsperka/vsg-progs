; $Log: not supported by cvs2svn $
; Revision 1.1  2004/10/01 20:06:10  dan
; Modified directory locations for executables (../bin) and sequencer files (script dir)
;

            SET    1,1,0           ; Get rate & scaling OK

E0:         DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EH:     'H  DIGOUT [....0011]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EL:     'L  DIGOUT [....0001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EQ:     'Q  DIGOUT [....1001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EJ:     'J  DIGOUT [.......1]
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT                   ; End of this sequence section


ED:		'D	DIGOUT [....0101]
            DELAY  s(0.996)-1
			HALT
