; $Log: not supported by cvs2svn $
; Revision 1.1  2004-10-01 19:41:01  dan
; From production
;
;
;
;
;
;
;

            SET      1.000 1 0
            VAR    V2=127

E0:         DIGOUT [00000001]
            HALT   

ET:     'T  DIGOUT [...0100.]
            HALT   

EM:     'M  DIGOUT [...1100.]
            HALT   

EB:     'B  DIGOUT [...1000.]
            HALT   

EL:     'L  DIGOUT [...0010.]
            HALT   

EC:     'C  DIGOUT [...0011.]
            HALT   

ER:     'R  DIGOUT [...0001.]
            HALT   

ES:     'S  DIGOUT V1
            HALT   

EQ:     'Q  DIGOUT [.10.....]
            HALT   

EJ:     'J  DIGOUT [.......1]
            DIGOUT [.......0]
            DELAY  s(0.005)-1
            DIGOUT [.......1]
            DELAY  s(0.989)-1
            HALT                   ; End of this sequence section


