#include "base_types.h"

function OperatingSystem
operating_system_from_context(void) {
    OperatingSystem result = OperatingSystem_Null;
#if OS_WINDOWS
    result = OperatingSystem_Windows;
#elif OS_LINUX
    result = OperatingSystem_Linux;
#elif OS_MAC
    result = OperatingSystem_Mac;
#endif
    return result;
}

function Architecture
architecture_from_context(void) {
    Architecture result = Architecture_Null;
#if ARCH_X64
    result = Architecture_X64;
#elif ARCH_X86
    result = Architecture_X86;
#elif ARCH_ARM
    result = Architecture_Arm;
#elif ARCH_ARM64
    result = Architecture_Arm64;
#endif
    return result;
}

function char*
string_from_operating_system(OperatingSystem os) {
    char *result = "(null)";
    switch (os){
        case OperatingSystem_Windows: result = "windows"; break;
        case OperatingSystem_Linux:   result = "linux";   break;
        case OperatingSystem_Mac:     result = "mac";     break;
    }
    return result;
}

function char*
string_from_architecture(Architecture arch) {
    char *result = "(null)";
    switch (arch){
        case Architecture_X64:   result = "x64";    break;
        case Architecture_X86:   result = "x86";    break;
        case Architecture_Arm:   result = "arm";    break;
        case Architecture_Arm64: result = "arm64";  break;
    }
    return(result);
}

