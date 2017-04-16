# $File: //depot/sw/epics/acai/documentation/Makefile $
# $Revision: #5 $
# $DateTime: 2017/04/16 15:31:20 $
# Last checked in by: $Author: andrew $
#

TOP=..

# Pick up configure/RELEASE defitions.
#
include $(TOP)/configure/CONFIG
#
# Note: we do not include the rule $(TOP)/configure/RULES

# Define targets
#
T1 = $(TOP)/html/index.html
T2 = $(TOP)/doc/acai.pdf

.PHONY: all install clean

all: $(T1)  $(T2)

install:  all

# NOTE: a failure of any of these doco build commands does NOT cause an overal make failure.

$(T1): acai.cfg ../acaiSup/*h ../acaiSup/*cpp
	@doxygen  acai.cfg || echo "doxygen not available"


$(T2): latex/refman.pdf
	@$(MKDIR) -p $(TOP)/doc &&  cp -f latex/refman.pdf  $(T2) || echo "latex/refman.pdf does not exist"


latex/refman.pdf : $(T1)
	@make -C latex  || echo "latex data not generated"

clean:
	rm -rf latex

# end
