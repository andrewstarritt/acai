# $File: //depot/sw/epics/acai/documentation/Makefile $
# $Revision: #3 $
# $DateTime: 2015/07/26 01:56:54 $
# Last checked in by: $Author: andrew $
#

TOP=..

.PHONY: all install clean documentations

all: documentations

install:  all

documentations: acai.cfg
	@doxygen  acai.cfg || echo "doxygen not available"

clean:
	rm -rf $(TOP)/html/*

# end
