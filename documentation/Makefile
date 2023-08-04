# Makefile
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
T2 = $(TOP)/doc/acai_manual.pdf

# An intermediate file
#
I2 = $(TOP)/latex/refman.pdf

.PHONY: all install clean

all: $(T1) $(T2)

install:  all

# NOTE: a failure of any of these doco build commands does NOT cause an overall make failure.
#

$(T1): acai.cfg ../acaiSup/*h ../acaiSup/*cpp
	@doxygen  acai.cfg  || echo "*** doxygen not available or failed."

# Notes:
# We do a dummy copy of the generated Makefile to force an immmediate error if doxygen failed.
# The Makefile in ../latex sometimes prompts the user for input, so
# ../documentation/empty provides a bunch of enters.
# We must ensure overall make does not stall awaiting input.
#
$(T2) : $(T1)
	@$(CP) $(TOP)/latex/Makefile z1    2>z2                     && \
	echo gnerating $(I2)                                        && \
	$(MAKE) -C $(TOP)/latex/ < $(TOP)/documentation/empty       && \
	echo installing acai_manual.pdf into ../doc/                && \
	$(MKDIR) $(TOP)/doc                                         && \
	$(CP) $(I2) $(T2)  || echo "*** acai_manual.pdf not generated"

clean:
	@$(RM) z1 z2 z3
	$(RM) -r ../latex

# end
