                SET    0.010,2,0       ;Sequence speed, scale etc. note 10V system

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
0000        'P  MOVI   OnStart,0       ;clear timing variables, idle loop will expect
0001            MOVI   OnEnd,0         ;them to be filled one by one
0002            MOVI   OffStart,0      ;reset cannot be done at EXIT or Spike2 misses values
0003            MOVI   OffEnd,0        ;so you lose 4 clock ticks after 'P - cope with it
0004            RAMP   0,Voltage1,RampSlp ;start positive ramp
0005            TICKS  OnStart         ;note time ramp to plateau starts
0006            JUMP   WAITUP          ;jump to common code

;========== Sequence N: do negative voltage ramp ==========
0007        'n  MOVI   OnStart,0       ;clear timing variables, idle loop will expect
0008            MOVI   OnEnd,0         ;them to be filled one by one
0009            MOVI   OffStart,0      ;reset cannot be done at EXIT or Spike2 misses values
0010            MOVI   OffEnd,0        ;so you lose 4 clock ticks after 'n - cope with it
0011            RAMP   0,Voltage2,RampSlp ;start negative ramp
0012            TICKS  OnStart         ;note time ramp to plateau starts
0013            JUMP   WAITUP          ;jump to common code

;========== Voltage ramp common code ==========
0014 WAITUP:    WAITC  0,WAITUP        ;wait for end of ramp to plateau
0015            TICKS  OnEnd           ;note time ramp to plateau ends
0016            DELAY  PlatDur         ;wait until plateau time is elapsed
0017            RAMP   0,0,RampSlp     ;start ramp to 0, sign of slope is ignored
0018            TICKS  OffStart        ;note time ramp to 0 starts
0019 WAITDOWN:  WAITC  0,WAITDOWN      ;wait for end of ramp to 0
0020            TICKS  OffEnd          ;note time ramp to 0 ends
0021            JUMP   EXIT            ;not strictly necessary to jump as currently written

;========== Wait for next SampleKey ==========
0022 EXIT:      NOP