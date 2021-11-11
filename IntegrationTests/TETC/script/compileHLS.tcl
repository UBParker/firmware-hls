# Common settings
source ../../../project/env_hls.tcl
source ../../common/script/tools.tcl
source ../../common/script/build_ip.tcl
linkCreate ../../common common
linkCreate ../../../emData/MemPrints MemPrints
linkCreate ../../../emData/LUTs LUTs
set CFLAGS {-std=c++11 -I../../../TrackletAlgorithm -I../../../TopFunctions}
set cwd [pwd]

# Build TE_L1L2
open_project -reset trackletengine
add_files ../../../TopFunctions/TrackletEngineTop.cc -cflags "$CFLAGS"
build_ip TrackletEngine_PS_PS ../../../project

# Build TC_L1L2E
open_project -reset trackletCalculator
add_files ../../../TopFunctions/TrackletCalculatorTop.cc -cflags "$CFLAGS"
build_ip TrackletCalculator_L1L2E ../../../project

exit
