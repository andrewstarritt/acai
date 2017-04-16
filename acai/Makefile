# $File: //depot/sw/epics/acai/Makefile $
# $Revision: #6 $
# $DateTime: 2017/04/16 15:36:43 $
# Last checked in by: $Author: andrew $
#
# Makefile at top of application tree
#
TOP = .
include $(TOP)/configure/CONFIG

# Directories to build, any order
DIRS += configure
DIRS += $(wildcard *Sup)
DIRS += $(wildcard *App)
DIRS += $(wildcard documentation)

# The build order is controlled by these dependency rules:

# All dirs except configure depend on configure
$(foreach dir, $(filter-out configure, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += configure))

# Any *App dirs depend on all *Sup dirs
$(foreach dir, $(filter %App, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += $(filter %Sup, $(DIRS))))

# Add any additional dependency rules here:

include $(TOP)/configure/RULES_TOP

# end
