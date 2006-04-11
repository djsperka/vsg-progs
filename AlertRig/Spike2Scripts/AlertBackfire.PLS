; $Log: not supported by cvs2svn $
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

            SET    1,1,0           ; Get rate & scaling OK

E0:     '0  DIGOUT [00000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section


ES:		'S	DIGOUT V1
			DELAY  s(0.996)-1
			HALT
			
EH:     'H  DIGOUT [....0011]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EL:     'L  DIGOUT [....0001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EQ:     'Q  DIGOUT [01000001]
            DELAY  s(0.996)-1
            HALT                   ; End of this sequence section

EJ:     'J  DIGOUT [.......1]
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT                   ; End of this sequence section


