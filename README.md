# acai

ACAI Channel Access Interface

## Introduction

ACAI is a thin-ish C++ wrapper around the low level Channel Access API
that provides an Asynchronus Channel Access Interface.

It is aimed at long term applications that need to connect to a number of
channels and stay connected, such as a gui back end or an orbit feedback
program.  It is not reaaly suitable, altough not impossible, for a "script" 
like program that does a number of one off "caget"/"caput" operations.

## Features

Each channel client is implemented as an ACAI client class object type.
These can be used "as is" or the class type can be used as the base class of 
one or more application specific class types.

The data for each channel is cached locally, so as up-to-date as possible data
is always available to the application.

An ACAI application must be implemented as an event loop with a regular call
to the ACAI poll function:
* Pros: All callbacks are in the same thread as the main program.
* Cons: Must implement an event loop style program.

Callbacks for channel connection and data update events may be implemented by
overriding the virtual notification functions within an application's derived
client class types and/or as a traditional callback handlers.

Comprehensive suite of channel data interogation/configuration functions 
including the capability to:
* access channel connection state.
* access data availability state.
* access values as integer/float/string.
* array access for multi value PVs.
* understands long strings ("PV.VAL$" type access).
* can override default native field type read/subscripition kind.
* can limit number of elements requested.

Stable API.

Fully doxygenated code. Doxygen invoked as part of make if available.

Apart from EPICS itself there are no external library dependencies.

