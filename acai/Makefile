# $File: //depot/sw/epics/acai/Makefile $
# $Revision: #3 $
# $DateTime: 2015/06/22 20:49:29 $
# Last checked in by: $Author: andrew $
#
# Makefile at top of application tree
#
TOP = .
include $(TOP)/configure/CONFIG
DIRS := $(DIRS) $(filter-out $(DIRS), configure)
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *Sup))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard *App))
DIRS := $(DIRS) $(filter-out $(DIRS), $(wildcard iocBoot))
#DIRS := $(DIRS) $(filter-out $(DIRS), documentation)

define DIR_template
 $(1)_DEPEND_DIRS = configure
endef
$(foreach dir, $(filter-out configure,$(DIRS)),$(eval $(call DIR_template,$(dir))))

acaiTestApp_DEPEND_DIRS += acaiSup

include $(TOP)/configure/RULES_TOP

# end
