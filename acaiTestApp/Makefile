# Makefile
#
TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the test applications

PROD_HOST += acai_monitor
acai_monitor_SRCS += acai_monitor.cpp

# Client only, we don't need all the EPICS_BASE_IOC_LIBS.
#
acai_monitor_LIBS += ca
acai_monitor_LIBS += Com
acai_monitor_LIBS += acai


PROD_HOST += test_client_set
test_client_set_SRCS += test_client_set.cpp

# Client only, we don't need all the EPICS_BASE_IOC_LIBS.
#
test_client_set_LIBS += ca
test_client_set_LIBS += Com
test_client_set_LIBS += acai


PROD_HOST += test_abstract_user
test_abstract_user_SRCS += test_abstract_user.cpp

# Client only, we don't need all the EPICS_BASE_IOC_LIBS.
#
test_abstract_user_LIBS += ca
test_abstract_user_LIBS += Com
test_abstract_user_LIBS += acai


PROD_HOST += test_csnprintf
test_csnprintf_SRCS += test_csnprintf.cpp

# Client only, we don't need all the EPICS_BASE_IOC_LIBS.
#
test_csnprintf_LIBS += ca
test_csnprintf_LIBS += Com
test_csnprintf_LIBS += acai


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
