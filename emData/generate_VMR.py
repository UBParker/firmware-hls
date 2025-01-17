#!/usr/bin/env python3

## python 2/3 inter-compatibility
from __future__ import absolute_import, print_function

# This script generates VMRouter_parameters.h, VMRouterTop.h,
# and VMRouterTop.cc in the TopFunctions/ directory.
# Supports all VMRs, but creates separate top function files each
# VMR specified. VMRouter_parameters.h contains magic numbers
# for all "default" VMRs plus additonals VMR if specified.

import argparse
import collections

# Constants
nLayers = 6
nDisks = 5

# The VMRs specified in download.sh
default_vmr_list = ["VMR_L1PHID", "VMR_L2PHIA", "VMR_L2PHIB", "VMR_L3PHIA" ,"VMR_L4PHIA", "VMR_L5PHIA", "VMR_L6PHIA", "VMR_D1PHIA", "VMR_D2PHIA", "VMR_D3PHIA", "VMR_D4PHIA", "VMR_D5PHIA"]

# Lists of which layer/disk has VMSTE memories
has_vmste_inner = [True, True, True, False, True, False, True, False, True, False, False]
has_vmste_overlap = [True, True, False, False, False, False, False, False, False, False, False]
has_vmste_outer = [False, True, True, True, False, True, True, True, False, True, False]

# Number of entries in bendcut table
bendcuttable_size = [8, 8, 8, 16, 16, 16, 8, 8, 8, 8, 8]


#############################################
# Returns a dictionary of memories

# One key for every memory type in each layer/disk
# Value is a list of all memory names for that key

def getDictOfMemories(wireconfig, vmr_list):

    # Dictionary of all the input and output memories
    mem_dict = {}

    # Open wiring file
    with open(wireconfig, "r") as wires_file:

        # Loop over each line in the wiring
        for line in wires_file:
            # Check if any of the VMRs exist in the line
            for vmr in vmr_list:
                if vmr in line:
                    mem_name = line.split()[0]
                    mem_type = mem_name.split("_")[0]
                    # Check if memory type is VMSTE or IL DISK2S
                    if "IL_D" in mem_name and "2S" in mem_name:
                        mem_type = mem_type + "_DISK2S_" + vmr
                    elif "TEI" in line:
                        phi_region = mem_name.split("PHI")[1]
                        if phi_region <= "L":
                            mem_type = mem_type + "I_" + vmr
                        else: # Overlap
                            mem_type = mem_type + "OL_" + vmr
                    elif "TEO" in line:
                        mem_type = mem_type + "O_" + vmr
                    else:
                        mem_type = mem_type + "_" + vmr
                    # Add memory and memory type to dictionary
                    if mem_type not in mem_dict:
                        mem_dict[mem_type] = [mem_name]
                    else:
                        mem_dict[mem_type].append(mem_name)
                    break

        # Loop over all memories and add an empty IL DISK2S and VMSTE memory lists if missing in dictionary
        for vmr in vmr_list:
            if "IL_DISK2S_" + vmr not in mem_dict:
                mem_dict["IL_DISK2S_" + vmr] = []
            if "VMSTEI_" + vmr not in mem_dict:
                mem_dict["VMSTEI_" + vmr] = []
            if "VMSTEOL_" + vmr not in mem_dict:
                mem_dict["VMSTEOL_" + vmr] = []
            if "VMSTEO_" + vmr not in mem_dict:
                mem_dict["VMSTEO_" + vmr] = []

    return mem_dict


###################################
# Returns a dictionary of memory copies

# Count the number of memories and copies, as the number of copies can differ

def getDictOfCopies(mem_list):

    mem_copy_dict = collections.OrderedDict() # Dictionary that counts the number of copies

    for mem in mem_list:
        mem_name = mem.split("n")[0] # Memory name without the copy number

        # Count the number of copies
        if mem_name not in mem_copy_dict:
            mem_copy_dict[mem_name] = 1
        else:
            mem_copy_dict[mem_name] += 1

    return mem_copy_dict


###################################
# Returns a list of all VMRs in the given wiring

def getAllVMRs(wireconfig):

    vmr_list = []

    # Open wiring file
    with open(wireconfig, "r") as wires_file:

        # Loop over each line in the wiring
        for line in wires_file:
            module_name = line.split(" ")[-1].split(".")[0]
            # Add module name if not already in vmr_list
            if "VMR" in module_name and module_name not in vmr_list:
                vmr_list.append(module_name)

    return vmr_list


#########################################
# Bendcut Table Helper Function

# Helper function that returns a string for writing the bendcut table
# functions in VMRouter_parameters.h file.

def getBendCutTable(mem_region, layer_disk_char, layer_disk_num, phi_region, max_copy_count, mem_copy_dict, mem_list):

    table_string = "template<> inline const ap_uint<getBendCutTableSize<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">()>* getBendCut" + mem_region + "Table<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){\n" +\
                   "  const int bendCutTableSize = getBendCutTableSize<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">();\n"

    # Sort memory list
    mem_list.sort(key=lambda mem: int("".join([i for i in mem[:-2] if i.isdigit()]))) # Sort by number (excluding the "copy" number nX)
    mem_list.sort(key=lambda mem: mem[:mem.index('PHI')]) # Sort alphabetically

    # Create temporary bendcut tables
    mem_index, mem_list_index = 0, 0

    for key in mem_copy_dict:
        mem_index += 1
        table_string += "  // TE " + mem_region + " Memory " + str(mem_index) + "\n"
        # Loop over the maximum number of copies, if the memory has less copies than that, add tables of zeros
        for i in range(max_copy_count):
            table_string += "  ap_uint<1> tmpBendTable" + str(mem_index) + "n" + str(i+1) + "[bendCutTableSize] ="
            if i < mem_copy_dict[key]:
                table_string += "\n#if __has_include(\"../emData/VMR/tables/" + mem_list[mem_list_index] + "_vmbendcut.tab\")\n#  include \"../emData/VMR/tables/" + mem_list[mem_list_index] + "_vmbendcut.tab\"\n#else\n  {};\n#endif\n"
                mem_list_index += 1
            else:
                table_string += " {0};\n"

    # Combine temporary bendcut tables and return it
    table_string += "\n  // Combine all the temporary tables into one big table\n" +\
                    "  static ap_uint<bendCutTableSize> bendCutTable[] = {"
    for i in range(len(mem_copy_dict)):
        table_string += "\n    "
        for j in range(max_copy_count):
            table_string += "arrayToInt<bendCutTableSize>(tmpBendTable" + str(i+1) + "n" + str(j+1) + "), "

    table_string = table_string[:-2] # Remove the last comma and space
    table_string += "\n  };\n\n" +\
                    "  return bendCutTable;\n" +\
                    "}\n"
    return table_string


##########################################
# Writes the VMRouter_parameters.h file

# Contains magic numbers for all default VMRs specified in download.sh
# and the specified units under test if non-default. Make sure to add
# non-default VMRs to download.sh and run it before running Vivado HLS

def writeParameterFile(vmr_list, mem_dict, output_dir):

    with open(output_dir + "/VMRouter_parameters.h", "w") as parameter_file:

        # Write preamble
        parameter_file.write(
"""#ifndef TopFunctions_VMRouter_parameters_h
#define TopFunctions_VMRouter_parameters_h

// Hardcoded number of memories and masks from the wiring.
// Generated by generate_VMR.py
""")

        parameter_file.write("// Contains number for the following VMRs: " + " ".join(vmr for vmr in vmr_list) + "\n")

        # Declare functions
        parameter_file.write("""
// Enums used to get the correct parameters
enum class phiRegions : char {A = 'A', B = 'B', C = 'C', D = 'D', E = 'E', F = 'F', G = 'G', H = 'H'};

// The functions that returns the LUTs and parameters
template<TF::layerDisk LayerDisk> const int* getPhiCorrTable();
template<TF::layerDisk LayerDisk> const int* getRzBitsInnerTable();
template<TF::layerDisk LayerDisk> const int* getRzBitsOverlapTable();
template<TF::layerDisk LayerDisk> const int* getRzBitsOuterTable();
template<TF::layerDisk LayerDisk, phiRegions Phi> const int* getFineBinTable();
template<TF::layerDisk LayerDisk, phiRegions Phi, int size> const ap_uint<size>* getBendCutInnerTable();
template<TF::layerDisk LayerDisk, phiRegions Phi, int size> const ap_uint<size>* getBendCutOverlapTable();
template<TF::layerDisk LayerDisk, phiRegions Phi, int size> const ap_uint<size>* getBendCutOuterTable();

template<TF::layerDisk LayerDisk, phiRegions Phi> constexpr int getNumInputs();
template<TF::layerDisk LayerDisk, phiRegions Phi> constexpr int getNumInputsDisk2S();
template<TF::layerDisk LayerDisk, phiRegions Phi> constexpr int getNumASCopies();
template<TF::layerDisk LayerDisk, phiRegions Phi> constexpr int getNumVMSTEICopies();
template<TF::layerDisk LayerDisk, phiRegions Phi> constexpr int getNumVMSTEOLCopies();
template<TF::layerDisk LayerDisk, phiRegions Phi> constexpr int getNumVMSTEOCopies();

template<TF::layerDisk LayerDisk, phiRegions Phi> constexpr int getBendCutTableSize();

// Help function that converts an array of 0s and 1s to an ap_uint
template<int arraySize>
inline ap_uint<arraySize> arrayToInt(ap_uint<1> array[arraySize]) {
  ap_uint<arraySize> number;
  for(int i = 0; i < arraySize; i++) {
    #pragma HLS unroll
    number[i] = array[i];
  }
  return number;
}

// VMPhiCorr LUTs
"""
        )

        # Write phi correction LUTs functions
        for i in range(nLayers):
            parameter_file.write(
                "template<> inline const int* getPhiCorrTable<TF::L" + str(i+1) + ">(){\n"
                "  static int lut[] = \n"
                "#if __has_include(\"../emData/VMR/tables/VMPhiCorrL" + str(i+1) + ".tab\")\n#  include \"../emData/VMR/tables/VMPhiCorrL" + str(i+1) + ".tab\"\n#else\n  {};\n#endif\n"
                "  return lut;\n"
                "}\n"
            )
        for i in range(nDisks):
            parameter_file.write(
                "template<> inline const int* getPhiCorrTable<TF::D" + str(i+1) + ">(){\n"
                "  return nullptr;\n"
                "}\n"
            )

        # Write VMTableInner
        parameter_file.write("\n// VMTableInner\n")
        for i in range(nLayers):
            parameter_file.write(
                "template<> inline const int* getRzBitsInnerTable<TF::L" + str(i+1) + ">(){\n"
                +("  static int lut[] =\n#if __has_include(\"../emData/VMR/tables/VMTableInnerL" + str(i+1) + "L" + str(i+2) + ".tab\")\n#  include \"../emData/VMR/tables/VMTableInnerL" + str(i+1) + "L" + str(i+2) + ".tab\"\n#else\n  {};\n#endif\n  return lut;\n" if has_vmste_inner[i] else "  return nullptr;\n")+
                "}\n"
            )
        for i in range(nDisks):
            parameter_file.write(
                "template<> inline const int* getRzBitsInnerTable<TF::D" + str(i+1) + ">(){\n"
                +("  static int lut[] =\n#if __has_include(\"../emData/VMR/tables/VMTableInnerD" + str(i+1) + "D" + str(i+2) + ".tab\")\n#  include \"../emData/VMR/tables/VMTableInnerD" + str(i+1) + "D" + str(i+2) + ".tab\"\n#else\n  {};\n#endif\n  return lut;\n" if has_vmste_inner[i+nLayers] else "  return nullptr;\n")+
                "}\n"
            )

        # Write VMRTableInner - Overlap
        parameter_file.write("\n// VMTableInner - Overlap\n")
        for i in range(nLayers):
            parameter_file.write(
                "template<> inline const int* getRzBitsOverlapTable<TF::L" + str(i+1) + ">(){\n"
                +("  static int lut[] =\n#if __has_include(\"../emData/VMR/tables/VMTableInnerL" + str(i+1) + "D1.tab\")\n#  include \"../emData/VMR/tables/VMTableInnerL" + str(i+1) + "D1.tab\"\n#else\n  {};\n#endif\n  return lut;\n" if has_vmste_overlap[i] else "  return nullptr;\n")+
                "}\n"
            )
        for i in range(nDisks):
            parameter_file.write(
                "template<> inline const int* getRzBitsOverlapTable<TF::D" + str(i+1) + ">(){\n"
                "  return nullptr;\n"
                "}\n"
            )

        # Write VMTableOuter
        parameter_file.write("\n// VMTableOuter\n")
        for i in range(nLayers):
            parameter_file.write(
                "template<> inline const int* getRzBitsOuterTable<TF::L" + str(i+1) + ">(){\n"
                +("  static int lut[] =\n#if __has_include(\"../emData/VMR/tables/VMTableOuterL" + str(i+1) + ".tab\")\n#  include \"../emData/VMR/tables/VMTableOuterL" + str(i+1) + ".tab\"\n#else\n  {};\n#endif\n  return lut;\n" if has_vmste_outer[i] else "  return nullptr;\n")+
                "}\n"
            )
        for i in range(nDisks):
            parameter_file.write(
                "template<> inline const int* getRzBitsOuterTable<TF::D" + str(i+1) + ">(){\n"
                +("  static int lut[] =\n#if __has_include(\"../emData/VMR/tables/VMTableOuterD" + str(i+1) + ".tab\")\n#  include \"../emData/VMR/tables/VMTableOuterD" + str(i+1) + ".tab\"\n#else\n  {};\n#endif\n  return lut;\n" if has_vmste_outer[i+nLayers] else "  return nullptr;\n")+
                "}\n"
            )

        # Write VMR specific functions
        for vmr in vmr_list:

            layer_disk_char = vmr.split("_")[1][0] # 'L' or 'D'
            layer_disk_num = int(vmr.split("_")[1][1])
            phi_region= vmr.split("PHI")[1]

            parameter_file.write(
                "\n////////////////\n// " + vmr + " //\n////////////////\n"
                "template<> constexpr int getNumInputs<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){ // Number of input memories, EXCLUDING DISK2S\n"
                "  return " + str(len(mem_dict["IL_"+vmr])) + ";\n"
                "}\n"
                "template<> constexpr int getNumInputsDisk2S<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){ // Number of DISK2S input memories\n"
                "  return " + str(len(mem_dict["IL_DISK2S_"+vmr])) + ";\n"
                "}\n"
                "template<> constexpr int getNumASCopies<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){ // Allstub memory\n"
                "  return " + str(len(mem_dict["AS_"+vmr])) + ";\n"
                "}\n"
                "template<> constexpr int getBendCutTableSize<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){\n"
                "  return " + str(bendcuttable_size[layer_disk_num-1]) + ";\n"
                "}\n"
                )

            for te_mem_type in ["VMSTEI", "VMSTEOL", "VMSTEO"]:

                if te_mem_type == "VMSTEI":
                    te_mem_region = "Inner"
                elif te_mem_type == "VMSTEOL":
                    te_mem_region = "Overlap"
                elif te_mem_type == "VMSTEO":
                    te_mem_region = "Outer"

                # Get the number of copies for the specified TE memory type
                mem_copy_dict = getDictOfCopies(mem_dict[te_mem_type + "_" + vmr])
                max_copy_count = max(mem_copy_dict.values()) if mem_copy_dict else 1

                parameter_file.write(
                    "template<> constexpr int getNum" + te_mem_type + "Copies<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){ // TE" + te_mem_region + "memory. NOTE: can't use 0 if we don't have any memories of a certain type. Use 1.\n"
                    "  return " + str(max_copy_count) + ";\n"
                    "}\n"
                )

                # Bendcut tables
                if mem_dict[te_mem_type + "_" + vmr]:
                    parameter_file.write(getBendCutTable(te_mem_region, layer_disk_char, layer_disk_num, phi_region, max_copy_count, mem_copy_dict, mem_dict[te_mem_type + "_" + vmr]))
                else:
                    parameter_file.write(
                    "template<> inline const ap_uint<getBendCutTableSize<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">()>* getBendCut" + te_mem_region + "Table<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){\n"
                    "  return nullptr;\n"
                    "}\n"
                    )

            parameter_file.write(
                "template<> inline const int* getFineBinTable<TF::" + layer_disk_char + str(layer_disk_num) + ", phiRegions::" + phi_region + ">(){\n"
                "  static int lut[] =\n"
                "#if __has_include(\"../emData/VMR/tables/" + vmr + "_finebin.tab\")\n#  include \"../emData/VMR/tables/" + vmr + "_finebin.tab\"\n#else\n  {};\n#endif\n"
                "  return lut;\n"
                "}\n"
            )

        # End parameter file
        parameter_file.write("\n#endif // TopFunctions_VMRouter_parameters_h\n")

#################################
# Writes the VMRouterTop.h file

def writeTopHeader(vmr_specific_name, vmr, output_dir):

    # Get layer/disk number and phi region
    layer = vmr.split("_")[1][1] if vmr.split("_")[1][0] == "L" else 0
    disk = 0 if layer else vmr.split("_")[1][1]
    phi_region= vmr.split("PHI")[1]

    # Top file name
    file_name = "VMRouterTop" + ("_" + vmr.split("_")[1] if vmr_specific_name else "")

    with open(output_dir + "/" + file_name  + ".h", "w") as header_file:

        # Write preamble
        header_file.write(
"#ifndef TopFunctions_" + file_name + "_h\n" + \
"#define TopFunctions_" + file_name + "_h\n" + \
"""
#include "VMRouter.h"
#include "VMRouter_parameters.h"

// VMRouter Top Function
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).

// To run a different phi region, change the following:
//          - add the phi region in emData/download.sh, make sure to also run clean
//
//          - kLAYER, kDISK, and phiRegion in VMRouterTop.h
//          - add corresponding magic numbers to VMRouter_parameters.h if not already defined
//          - add/remove pragmas depending on inputStubs in VMRouterTop.cc (not necessary to run simulation)
//          OR
//          - run emData/generate_VMR.py to generate new top and parameters files

////////////////////////////////////////////
// Variables for that are specified with regards to the VMR region

"""
        )

        # Write the configuration
        header_file.write(
"#define kLAYER " + str(layer) + " // Which barrel layer number the data is coming from\n"
"#define kDISK " + str(disk) + " // Which disk number the data is coming from, 0 if not disk\n\n"

"constexpr phiRegions phiRegion = phiRegions::" + phi_region+ "; // Which AllStub/PhiRegion\n"
        )

        # Write the rest...
        header_file.write("""

///////////////////////////////////////////////
// Variables that don't need changing

constexpr TF::layerDisk layerdisk = static_cast<TF::layerDisk>((kLAYER) ? kLAYER-1 : trklet::N_LAYER+kDISK-1);

// Number of inputs
constexpr int numInputs = getNumInputs<layerdisk, phiRegion>(); // Number of input memories, EXCLUDING DISK2S
constexpr int numInputsDisk2S = getNumInputsDisk2S<layerdisk, phiRegion>(); // Number of DISK2S input memories

// Maximum number of memory "copies" for this Phi region
constexpr int maxASCopies = getNumASCopies<layerdisk, phiRegion>(); // Allstub memory
constexpr int maxTEICopies = getNumVMSTEICopies<layerdisk, phiRegion>(); // TE Inner memories
constexpr int maxOLCopies = getNumVMSTEOLCopies<layerdisk, phiRegion>(); // TE Inner memories
constexpr int maxTEOCopies = getNumVMSTEOCopies<layerdisk, phiRegion>(); // TE Outer memories

constexpr int bendCutTableSize = getBendCutTableSize<layerdisk, phiRegion>(); // Number of entries in each bendcut table

#if kLAYER == kDISK
#error kLAYER and kDISK cannot be the same

#elif kLAYER > 0
	// Number of VMs
	constexpr int nvmME = nvmmelayers[kLAYER-1]; // ME memories
	constexpr int nvmTEI = (kLAYER != 2) ? nvmtelayers[kLAYER-1] : nvmteextralayers[1]; // TE Inner memories
	constexpr int nvmOL = kLAYER == 1 ? nvmollayers[0] : (kLAYER == 2 ? nvmollayers[1] : 1); // TE Inner Overlap memories, can't use 0 when we don't have any OL memories
	constexpr int nvmTEO = (kLAYER != 3) ? nvmtelayers[kLAYER-1] : nvmteextralayers[2]; // TE Outer memories

	// Number of bits used to address the stubs
	constexpr int nbitsmemaddr = kNBits_MemAddr;

	// Number of bits used for the bins in VMStubME memories
	constexpr int nbitsbin = 3;

	// What regionType the input/output is
	constexpr regionType inputType = (kLAYER > 3) ? BARREL2S : BARRELPS;
	constexpr regionType outputType = (kLAYER > 3) ? BARREL2S : BARRELPS;

#elif kDISK > 0
	// Number of VMs
	constexpr int nvmME = nvmmedisks[kDISK-1]; // ME memories
	constexpr int nvmTEI = nvmtedisks[kDISK-1]; // TE Inner memories
	constexpr int nvmOL = 1; // TE Inner Overlap memories, can't use 0 when we don't have any OL memories
	constexpr int nvmTEO = nvmtedisks[kDISK-1]; // TE Outer memories

	// Number of bits used to address the stubs
	constexpr int nbitsmemaddr = kNBits_MemAddr + 1;

	// Number of bits used for the bins in VMStubME memories
	constexpr int nbitsbin = 4;

	// What regionType the input/output is
	constexpr regionType inputType = DISKPS;
	constexpr regionType outputType = DISK;

#else
#error Need to have either kLAYER or kDISK larger than 0.
#endif


/////////////////////////////////////////////////////
// VMRouter Top Function

void %s(const BXType bx, BXType& bx_o,
	// Input memories
	const InputStubMemory<inputType> inputStubs[numInputs]
#if kDISK > 0
	, const InputStubMemory<DISK2S> inputStubsDisk2S[numInputsDisk2S]
#endif
	// Output memories
	, AllStubMemory<outputType> memoriesAS[maxASCopies]
	, VMStubMEMemory<outputType, nbitsmemaddr, nbitsbin> memoriesME[nvmME]
#if kLAYER == 1 || kLAYER == 2 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3
	, VMStubTEInnerMemory<outputType> memoriesTEI[nvmTEI][maxTEICopies]
#endif
#if kLAYER == 1 || kLAYER == 2
	, VMStubTEInnerMemory<BARRELOL> memoriesOL[nvmOL][maxOLCopies]
#endif
#if kLAYER == 2 || kLAYER == 3 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4
	, VMStubTEOuterMemory<outputType> memoriesTEO[nvmTEO][maxTEOCopies]
#endif
	);
#endif // TopFunctions_VMRouterTop_h
""" % file_name
        )

# Writes the VMRouterTop.cc file
def writeTopFile(vmr_specific_name, vmr, num_inputs, num_inputs_disk2s, output_dir):

    # Top file name
    file_name = "VMRouterTop" + ("_" + vmr.split("_")[1] if vmr_specific_name else "")

    with open(output_dir + "/" + file_name  + ".cc", "w") as top_file:

        # Write preamble
        top_file.write("#include \"" + file_name + ".h\"" + \
"""

// VMRouter Top Function
// Sort stubs into smaller regions in phi, i.e. Virtual Modules (VMs).
// To run a different phi region, change the following:
//          - add the phi region in emData/download.sh, make sure to also run clean
//
//          - kLAYER, kDISK, and phiRegion in VMRouterTop.h
//          - add corresponding magic numbers to VMRouter_parameters.h if not already defined
//          - add/remove pragmas depending on inputStubs in VMRouterTop.cc (not necessary to run simulation)
//          OR
//          - run emData/generate_VMR.py to generate new top and parameters files

void %s(const BXType bx, BXType& bx_o,
	// Input memories
	const InputStubMemory<inputType> inputStubs[numInputs]
#if kDISK > 0
	, const InputStubMemory<DISK2S> inputStubsDisk2S[numInputsDisk2S]
#endif
	// Output memories
	, AllStubMemory<outputType> memoriesAS[maxASCopies]
	, VMStubMEMemory<outputType, nbitsmemaddr, nbitsbin> memoriesME[nvmME]
#if kLAYER == 1 || kLAYER == 2 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3
	, VMStubTEInnerMemory<outputType> memoriesTEI[nvmTEI][maxTEICopies]
#endif
#if kLAYER == 1 || kLAYER == 2
	, VMStubTEInnerMemory<BARRELOL> memoriesOL[nvmOL][maxOLCopies]
#endif
#if kLAYER == 2 || kLAYER == 3 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4
	, VMStubTEOuterMemory<outputType> memoriesTEO[nvmTEO][maxTEOCopies]
#endif
) {

// Takes 2 clock cycles before on gets data, used at high frequencies
""" % file_name
        )

        for i in range(num_inputs):
            top_file.write("#pragma HLS resource variable=inputStubs[" + str(i) + "].get_mem() latency=2\n")
        for i in range(num_inputs_disk2s):
            top_file.write("#pragma HLS resource variable=inputStubsDisk2S[" + str(i) + "].get_mem() latency=2\n")

        top_file.write("""
#pragma HLS interface register port=bx_o

	///////////////////////////
	// Open Lookup tables

	// LUT with the corrected r/z. It is corrected for the average r (z) of the barrel (disk).
	// Includes both coarse r/z position (bin), and finer region each r/z bin is divided into.
	// Indexed using r and z position bits
	static const int* fineBinTable = getFineBinTable<layerdisk, phiRegion>();

	// LUT with phi corrections to project the stub to the average radius in a layer.
	// Only used by layers.
	// Indexed using phi and bend bits
	static const int* phiCorrTable = getPhiCorrTable<layerdisk>();

	// LUT with the Z/R bits for TE memories
	// Contain information about where in z to look for valid stub pairs
	// Indexed using z and r position bits
	static const int* rzBitsInnerTable = getRzBitsInnerTable<layerdisk>();
	static const int* rzBitsOverlapTable = getRzBitsOverlapTable<layerdisk>();
	static const int* rzBitsOuterTable = getRzBitsOuterTable<layerdisk>();

	// LUT with bend-cuts for the TE memories
	// The cuts are different depending on the memory version (nX)
	// Indexed using bend bits
	// Note: use an array of zeros for "missing" memories in the first and last Phi Region
	static const ap_uint<bendCutTableSize>* bendCutInnerTable = getBendCutInnerTable<layerdisk, phiRegion, bendCutTableSize>();
	static const ap_uint<bendCutTableSize>* bendCutOverlapTable = getBendCutOverlapTable<layerdisk, phiRegion, bendCutTableSize>();
	static const ap_uint<bendCutTableSize>* bendCutOuterTable = getBendCutOuterTable<layerdisk, phiRegion, bendCutTableSize>();

	//////////////////////////////////
	// Create memory masks
	// Masks of which memories that are being used. The first memory is represented by the LSB
	// and a "1" implies that the specified memory is used for this phi region
	// Create "nvm" 1s, e.g. "1111", shift the mask until it corresponds to the correct phi region

	static const ap_uint<maskMEsize> maskME = ((1 << nvmME) - 1) << (nvmME * (static_cast<char>(phiRegion) - 'A')); // ME memories
	static const ap_uint<maskTEIsize> maskTEI =
		(kLAYER == 1 || kLAYER == 2 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3) ?
				((1 << nvmTEI) - 1) << (nvmTEI * (static_cast<char>(phiRegion) - 'A')) : 0x0; // TE Inner memories, only used for odd layers/disk and layer 2
	static const ap_uint<maskOLsize> maskOL =
		((kLAYER == 1) || (kLAYER == 2)) ?
				((1 << nvmOL) - 1) << (nvmOL * (static_cast<char>(phiRegion) - 'A')) : 0x0; // TE Inner Overlap memories, only used for layer 1 and 2
	static const ap_uint<maskTEOsize> maskTEO =
		(kLAYER == 2 || kLAYER == 3 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4) ?
				((1 << nvmTEO) - 1) << (nvmTEO * (static_cast<char>(phiRegion) - 'A')) : 0x0; // TE Outer memories, only for even layers/disks, and layer and disk 1

	/////////////////////////
	// Main function

	VMRouter<inputType, outputType, kLAYER, kDISK, numInputs, numInputsDisk2S, maxASCopies, maxTEICopies, maxOLCopies, maxTEOCopies, nbitsmemaddr, nbitsbin, bendCutTableSize>
	(bx, bx_o, fineBinTable, phiCorrTable,
		rzBitsInnerTable, rzBitsOverlapTable, rzBitsOuterTable,
		bendCutInnerTable, bendCutOverlapTable, bendCutOuterTable,
		// Input memories
		inputStubs,
#if kDISK > 0
		inputStubsDisk2S,
#else
		nullptr,
#endif
		// AllStub memories
		memoriesAS,
		// ME memories
		maskME, memoriesME,
		// TEInner memories
		maskTEI,
#if kLAYER == 1 || kLAYER == 2 || kLAYER == 3 || kLAYER == 5 || kDISK == 1 || kDISK == 3
		memoriesTEI,
#else
		nullptr,
#endif
		// TEInner Overlap memories
		maskOL, 
#if kLAYER == 1 || kLAYER == 2
		memoriesOL,
#else
		nullptr,
#endif
		// TEOuter memories
		maskTEO, 
#if kLAYER == 2 || kLAYER == 3 || kLAYER == 4 || kLAYER == 6 || kDISK == 1 || kDISK == 2 || kDISK == 4
		memoriesTEO
#else
		nullptr
#endif
		);

	return;
}
"""
        )

###############################
# Main execution

if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description = """
Generates top function and parameter files for the VMRouter.
VMRouterTop*.h and VMRouterTop*.cc contain the top function for the units under test specified (default VMR_L2PHIA).
VMRouter_parameters.h contains the magic numbers for the specified units under test.

Examples:
python3 generate_VMR.py
python3 generate_VMR.py --uut VMR_L1PHIE -o
python3 generate_VMR.py --uut VMR_L1PHIE VMR_L1PHID
python3 generate_VMR.py -a
""",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument("-a", "--all", default=False, action="store_true", help="Create files for all VMRouters in a nonant.")
    parser.add_argument("-d", "--default", default=False, action="store_true", help="Create files for VMRs: " + " ".join(vmr for vmr in default_vmr_list) + "(default = %(default)s)")
    parser.add_argument("-O", "--overwrite", default=False, action="store_true", help="Overwrite the default VMRouterTop.h/cc files (instead of creating files e.g. VMRouterTop_L1PHIE.h/cc). Only works if a single VMR has been specified (default = %(default)s)")
    parser.add_argument("--uut", default=["VMR_L2PHIA"], nargs="+", help="Unit Under Test (default = %(default)s)")
    parser.add_argument("-o", "--outputdir", type=str, default="../TopFunctions/", help="The directory in which to write the output files (default=%(default)s)")
    parser.add_argument("-w", "--wireconfig", type=str, default="LUTs/wires.dat",
                        help="Name and directory of the configuration file for wiring (default = %(default)s)")

    args = parser.parse_args()

    # Include the VMR name in the names of VMRouterCMTop files
    vmr_specific_name = bool(len(args.uut) > 1 or not args.overwrite or args.all)

    # Get a list of the Units Under Test
    if args.all:
        vmr_list = getAllVMRs(args.wireconfig)
    elif args.default:
        vmr_list = default_vmr_list.copy()
        # Add explicitly defined UUTs if any
        for vmr in args.uut:
            if vmr not in vmr_list:
                vmr_list.append(vmr)
    else:
        vmr_list = args.uut
    vmr_list.sort()

    # Dictionary of all memories sorted by type and Unit Under Test
    mem_dict = getDictOfMemories(args.wireconfig, vmr_list)

    # Loop over all Units Under Test
    for vmr in vmr_list:
        # Check that the Unit Under Test is a VMR
        if "VMR" not in vmr:
            raise IndexError("Unit under test has to be a VMR.")

        # Check if one of the default VMRs
        if vmr not in default_vmr_list:
            print("Make sure to add " + vmr + " to download.sh and run it before running Vivado HLS.")

        # Create and write the files
        writeTopHeader(vmr_specific_name, vmr, args.outputdir)
        writeTopFile(vmr_specific_name, vmr, len(mem_dict["IL_"+vmr]), len(mem_dict["IL_DISK2S_"+vmr]), args.outputdir)

    # Write parameters file
    writeParameterFile(vmr_list, mem_dict, args.outputdir)
