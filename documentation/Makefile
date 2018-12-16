# Makefile
#

TOP=..

# Pick up configure/RELEASE defitions.
#
include $(TOP)/configure/CONFIG
#
# Note: we do not include the rule $(TOP)/configure/RULES

# Define target
#
T1 = $(TOP)/html/index.html

.PHONY: all install clean

all: $(T1)

install:  all

# NOTE: a failure of any of these doco build commands does NOT cause an overall make failure.

$(T1): acai.cfg ../acaiSup/*h ../acaiSup/*cpp
	@doxygen  acai.cfg || echo "doxygen not available"


clean:

# end
