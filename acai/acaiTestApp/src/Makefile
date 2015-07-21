# $File: //depot/sw/epics/acai/acaiTestApp/src/Makefile $
# $Revision: #1 $
# $DateTime: 2014/11/18 22:35:35 $
# Last checked in by: $Author: andrew $
#
TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = acaiTest
# acaiTest.dbd will be created and installed
DBD += acaiTest.dbd

# acaiTest.dbd will be made up from these files:
acaiTest_DBD += base.dbd

# Include dbd files from all support applications:
#acaiTest_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#
acaiTest_LIBS += acai

# acaiTest_registerRecordDeviceDriver.cpp derives from acaiTest.dbd
acaiTest_SRCS += acaiTest_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
acaiTest_SRCS_DEFAULT += acaiTestMain.cpp
acaiTest_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#acaiTest_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
acaiTest_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
