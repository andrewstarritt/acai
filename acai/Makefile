# $File: //depot/sw/epics/acai/Makefile $
# $Revision: #4 $
# $DateTime: 2016/07/17 15:58:22 $
# Last checked in by: $Author: andrew $
#
# Makefile at top of application tree
#
TOP = .
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), configure)
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Sup))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *App))
DIRS := $(DIRS) $(filter-out $(DIRS), documentation)

# The build order is controlled by these dependency rules:

# All dirs except configure depend on configure
#
$(foreach dir, $(filter-out configure, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += configure))

# Any *App dirs depend on all *Sup dirs
#
$(foreach dir, $(filter %App, $(DIRS)), \
    $(eval $(dir)_DEPEND_DIRS += $(filter %Sup, $(DIRS))))

include $(TOP)/configure/RULES_TOP

# end
