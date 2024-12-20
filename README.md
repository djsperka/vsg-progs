# vsg-progs
library and collection of visual stimulus programs written for the VSG and Visage from Cambridge Research Systems (http://crsltd.com)

This represents 20+ yrs of development of visual stimuli using the Visage system from CRS (and its prececessor the VSG 2/5). 
As such, there is quite a bit of old abandoned code which I will be clearing out and removing, or maybe not. As of this writing (12/20/2024), the only stuff actively used
is fixstim/alertlib, and the applications remote and CableTest. 

# setup
- install boost
  - installation is confusing. Need header-only libs as well as compiled libs. Be a hero and build it yourself, or else get [pre-built binary download.](https://boost.teeks99.com/)
- install visage stuff
- install asl sdk (unless no eyetracking)
- set env variables
  - BOOST_INCLUDE_PATH_64 - path to folder where you can find "boost/version.hpp". Address include files with <boost/xyz.hpp> in code
  - BOOST_LIB_PATH_64 - path to folder containing boost_*.lib (e.g. "boost_fileystem-vc141-mt-gd-x64-1_69.lib"
  - VSG_INCLUDE_PATH_64 - path to folder containing "VSGV8.H"
  - VSG_LIB_PATH_64 - path to folder containing vsgv8.lib
  - ASL_INCLUDE_PATH_64 - path to folder containing "ASLSerialOutLib2.h" - they put it in same folder as dll
  - ASL_LIB_PATH_64 - path to folder containing ASLSerialOutLib2.dll

# building
- MSVC 2022
- Get and prepare source code
  - git clone https://github.com/djsperka/vsg-progs.git
  - git submodule init
  - git submodule update
  - Run "Visual Studio/x64 Native Tools Command Prompt"
  - cd AlertRig\argp-standlone
  - cmake .
  - A VS2022 solution file is created. Open that solution, then build the lib. This should place lib in vsg-progs\AlertRig\argp-standalone\src\Release
  - The locations of include and lib files are already incorporated into the properties for the AlertRig solution.
- Re-open AlertRig solution
- Build alertlib first (if build entire solution, some parallel builds will fail because this isn't done yet, and I haven't bothered with the config to fix that)
- Build fixstim, cabletest, remote
- Build "aaaDeploy" to move exe files to bin folder used in spike2 scripts

# notes

Building these exe files and libs (with the exception of "aaaDeploy") is guaranteed NOT to affect production stuff (just make sure "vsg-progs" and "usrig" folders are not nested at all). 

Since we rely heavily on "fixstim", you can run it from any folder for testing. The scripts can communicate with its port regardless of where "fixstim.exe" resides. 

  
