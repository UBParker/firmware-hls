#!/bin/sh

# Create HLS IP for IR, VMR, TE, TC, PR, ME, MC & TB
# Run this in IntegrationTests/xyz/script/ 

pushd ../../../project

vivado_hls -f script_IR.tcl
vivado_hls -f script_VMR.tcl
vivado_hls -f script_TE.tcl
vivado_hls -f script_TC.tcl
vivado_hls -f script_PR.tcl
vivado_hls -f script_ME.tcl
vivado_hls -f script_MC.tcl

popd