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
T2 = $(TOP)/doc/acai_manual.pdf
I2 = $(TOP)/latex/refman.pdf

.PHONY: all install clean

all: $(T1) $(T2)

install:  all

# NOTE: a failure of any of these doco build commands does NOT cause an overall make failure.
#

$(T1): acai.cfg ../acaiSup/*h ../acaiSup/*cpp
	@doxygen  acai.cfg && cd ../latex/ && make || echo "doxygen not available"

$(I2) : $(T1)
	cd ../latex/        && \
	make                || echo "refman.pdf not generated"

$(T2) : $(I2)
	mkdir -p $(TOP)/doc && \
	cp $(I2) $(T2)      || echo "refman.pdf not available"

clean:
	rm -rf ../latex

# end
