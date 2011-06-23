rem WORKS ..\..\bin\msequence -d 8 -p 0,0 -t 5 -v -a -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem WORKS ..\..\bin\msequence -d 16 -p 100,100 -t 200 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem DOES NOT WORK ..\..\bin\msequence -d 16 -r 12 -c 8 -p 100,100 -t 200 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem ..\..\bin\msequence -d 28 -p 0,0 -t 200 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem WORKS (will not fit into memory warning)..\..\bin\msequence -d 28 -r 8 -c 8 -p 0,0 -t 200 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem		works ..\..\bin\msequence -d 24 -r 8 -c 8 -O -K -z 8 -p 0,0 -t 200 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem WORKS..\..\bin\msequence -d 24 -r 8 -c 12 -O -K -z 8 -p 0,0 -t 200 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
REM works ..\..\bin\msequence -d 24 -r 16 -c 12 -O -K -z 8 -p 0,0 -t 200 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem ..\..\bin\msequence -d 16 -r 8 -c 8 -z 8 -p 0,0 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem Y OFFSET ..\..\bin\msequence -d 28 -r 1 -c 16 -p 0,0 -z 4 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
REM d/z=7 ..\..\bin\msequence -d 28 -r 1 -c 16 -p 0,2 -z 4 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
REM OK ..\..\bin\msequence -d 24 -r 1 -c 16 -p 0,0 -z 4 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
REM ok ..\..\bin\msequence -H 64 -W 16 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem ?? ..\..\bin\msequence -H 256 -W 32 -N -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK ..\..\bin\msequence -H 32 -W 32 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
REM OK ..\..\bin\msequence -H 32 -W 32 -r 2 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK ..\..\bin\msequence -H 64 -W 32 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK ..\..\bin\msequence -H 96 -W 32 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK ..\..\bin\msequence -H 128 -W 32 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem NOT OK MUST ADD -1 TO Y OFFSETS IN WINDOW MOVE ..\..\bin\msequence -H 128 -W 32 -N -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK ..\..\bin\msequence -H 128 -W 32 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK ..\..\bin\msequence -H 128 -W 48 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK..\..\bin\msequence -H 128 -W 50 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
rem OK too tall ..\..\bin\msequence -H 256 -W 50 -r 1 -c 16 -p 0,0 -z 16 -t 200 -O -K -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms
..\..\bin\msequence -H 240 -W 50 -r 1 -c 16 -p 0,0 -z 16 -t 2 -a -v -R 3 -T 1,100 -f ../../stim/MSequenceTerms