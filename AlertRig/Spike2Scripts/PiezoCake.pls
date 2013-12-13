                SET      0.010 2 0     ;Sequence speed, scale etc. note 10V system

                ;Sequencer variables are obligatory integers
                VAR    V1,OnStart      ;Time of onset ramp start, in ticks
                VAR    V2,OnEnd        ;Time of onset ramp end, in ticks
                VAR    V3,OffStart     ;Time of offset ramp start, in ticks
                VAR    V4,OffEnd       ;Time of offset ramp end, in ticks
                VAR    V5,Voltage1     ;Voltage target for ramp 1 (up) in DAC units
                VAR    V6,Voltage2     ;Voltage target for ramp 2 (down) in DAC units
                VAR    V7,RampSlp      ;Slope of ramp in DAC units per sequencer step
                VAR    V8,PlatDur      ;Duration of plateau in sequencer steps


;========== Sequence P: do positive voltage ramp ==========
0000         'P RAMP   0,Voltage1,RampSlp ;start positive ramp
0001            TICKS  OnStart         ;note time ramp to plateau starts
0002            JUMP   WAITUP          ;jump to common code

;========== Sequence N: do negative voltage ramp ==========
0003         'N RAMP   0,Voltage2,RampSlp ;start negative ramp
0004            TICKS  OnStart         ;note time ramp to plateau starts
0005            JUMP   WAITUP          ;jump to common code

;========== Voltage ramp common code ==========
0006 WAITUP:    WAITC  0,WAITUP        ;wait for end of ramp to plateau
0007            TICKS  OnEnd           ;note time ramp to plateau ends
0008            DELAY  PlatDur         ;wait until plateau time is elapsed
0009            RAMP   0,0,RampSlp     ;start ramp to 0, sign of slope is ignored
0010            TICKS  OffStart        ;note time ramp to 0 starts
0011 WAITDOWN:  WAITC  0,WAITDOWN      ;wait for end of ramp to 0
0012            TICKS  OffEnd          ;note time ramp to 0 ends
0013            JUMP   EXIT            ;not strictly necessary to jump as currently written

;========== Wait for next SampleKey ==========
0014 EXIT:      MOVI   OnStart,0       ;clear timing variables, idle loop will expect
0015            MOVI   OnEnd,0         ;them to be filled one by one
0016            MOVI   OffStart,0
0017            MOVI   OffEnd,0
