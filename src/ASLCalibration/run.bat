REM This bat script will run the calibration helper app. 
REM Any lines that start with the letters "rem" or "REM" are remarks and are ignored. 
REM Same is true of blank lines. 

REM There are 4 possible args to the application. They can come in any order. 
REM
REM Fixation point properties. Note the space between the "-f" and the comma-delimited values. There are
REM no spaces between the comma-separated values. 
REM The 4 values are x-position,y-position,diameter(degrees),color
REM Color can be red, blue or green.
REM
REM -f 0,0,0.2,red
REM
REM Background color - can be black, gray, white. 
REM
REM -b gray
REM
REM Screen distance and calibration offset. Don't change these! (Unless of course the actual screen distance 
REM changes!!!)
REM
REM -d 800 -C 8
REM
REM 
REM Calibration  
REM
REM
REM Here are some examples: Note that they are commented out. The original command line is at the bottom of the file.
REM
REM  Black background, big blue fixation point (good for Betty)
d:\work\alertrig\bin\ASLCalibration -f 0,0,0.4,blue -b black -d 800 -C 8
REM
REM  White background, blue fixation point
REM  d:\work\alertrig\bin\ASLCalibration -f 0,0,0.2,blue -b white -d 800 -C 8
REM
REM Below is the original command line

REM d:\work\alertrig\bin\ASLCalibration -f 0,0,0.3,blue -b gray -d 800 -C 8

REM This is large red spot on grey background (Beetle's favorite)
REM d:\work\alertrig\bin\ASLCalibration -f 0,0,0.2,red -b gray -d 800 -C 8

