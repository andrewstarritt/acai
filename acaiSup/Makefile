# Makefile for the acai linrary.
#
# SPDX-FileCopyrightText: 2013-2025  Andrew C. Starritt
# SPDX-License-Identifier: LGPL-3.0-only
#

TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#========================================

#========================================
# build a support library.

LIBRARY_IOC += acai
SHRLIB_VERSION = 1.8

# indicate that we are building the library.
#
USR_CPPFLAGS += -DBUILDING_ACAI_LIBRARY

# specify header files to be visible to library users.
#
INC += acai_client.h
INC += acai_abstract_client_user.h
INC += acai_client_set.h
INC += acai_client_types.h
INC += acai_shared.h
INC += acai_version.h

# specify all source files to be compiled and added to the library.
#
acai_SRCS += buffered_callbacks.c
acai_SRCS += acai_client.cpp
acai_SRCS += acai_abstract_client_user.cpp
acai_SRCS += acai_client_set.cpp
acai_SRCS += acai_client_types.cpp
acai_SRCS += acai_version.cpp

# Required libraries.
#
acai_LIBS += ca
acai_LIBS += Com

#========================================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
