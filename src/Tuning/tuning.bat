rem  run tuning area
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning  -a -f 0,0,0.25,blue -b gray -d 500 -g -10.8,-5.7,5.0,5.0,-100,0.50,4.00,90.0,b,s,e -A 0.15,0.208299,0.289258,0.401682,0.5578,0.774597,1.07565,1.49372,2.07427,2.88047,4
rem same thing without ascii triggers - use spike2, run sampling config sequencer file...
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning  -f 0,0,0.25,blue -b gray -d 500 -g -10.8,-5.7,5.0,5.0,-100,0.50,4.00,90.0,b,s,e -A 0.15,0.208299,0.289258,0.401682,0.5578,0.774597,1.07565,1.49372,2.07427,2.88047,4

rem orientation tuning here , ascii triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,0.00,90.0,b,s,e -O 0,90,180,270,360

rem orientation tuning here , use sampling sequencer
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,0.00,90.0,b,s,e -O 0,90,180,270,360


rem SANDBOX - orientation tuning here , ascii triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,0.00,90.0,b,s,e -O 0,90,180,270,360

rem sandbox, orientation, spike2 triggers
C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,2.00,90.0,b,s,e -O 0,90,180,270,360

rem sandbox, area, spike2 triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox  -f 0,0,0.25,blue -b gray -d 500 -g -10.8,-5.7,5.0,5.0,-100,0.50,4.00,90.0,b,s,e -A 0.15,0.208299,0.289258,0.401682,0.5578,0.774597,1.07565,1.49372,2.07427,2.88047,4
