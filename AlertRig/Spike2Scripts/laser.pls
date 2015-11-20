            SET      0.010 1 0     ;Get rate & scaling OK
            VAR    V2=0            ;V2 logs whether the sequencer is in use for SafeSampleKey
                                       ;V3 determines whether juicer is open-high or open-low
                                       ;V4 is a voltage out variable for setting level on the opto


; zero out all trigger lines, close juicer
ZERO:   '0  MOVI   V2,1            ;Log that sequencer is in use
            BLT    V3,1,ZOPENHI    ;Branch, if V3 is 0, go to ZOPENHI
            DIGOUT [00000001]      ;Set juicer bit to closed (1 is closed)
            JUMP   ZERODONE        ;Jump over ZOPENHI to MOVI
ZOPENHI:    DIGOUT [00000000]      ;Set juicer bit to closed (0 is closed)
ZERODONE:   MOVI   V2,0            ;Log that sequencer is not in use
            HALT                   ;End of this sequence section


; set dac1 to +2.5V for 2s

EW:     'W  DAC    1,2.5           ;ttl on
            DELAY  s(2)-1          ;Delay 1S
            DAC    1,0             ;ttl off
            HALT   

; set dac1 to +2.5V for 1s

EX:     'X  DAC    1,2.5           ;ttl on
            DELAY  s(1)-1          ;Delay 1S
            DAC    1,0             ;ttl off
            HALT   

; set dac1 to +5V for 2s

EY:     'Y  DAC    1,5             ;ttl on
            DELAY  s(2)-1          ;Delay 1S
            DAC    1,0             ;ttl off
            HALT   

; set dac1 to +5V for 1s

EZ:     'Z  DAC    1,5             ;ttl on
            DELAY  s(1)-1          ;Delay 1S
            DAC    1,0             ;ttl off
            HALT   

; set dac1 to 0V

EU:     'U  DAC    1,0
            HALT   

