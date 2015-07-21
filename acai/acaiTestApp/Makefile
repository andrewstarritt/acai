# $File: //depot/sw/epics/acai/acaiTestApp/Makefile $
# $Revision: #2 $
# $DateTime: 2015/01/19 19:58:00 $
# Last checked in by: $Author: andrew $
#
TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the test application

PROD_HOST += acai_monitor
acai_monitor_SRCS += acai_monitor.cpp

# Client only, we don't need all the EPICS_BASE_IOC_LIBS.
#
acai_monitor_LIBS += ca
acai_monitor_LIBS += Com
acai_monitor_LIBS += acai

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
