# check that we are on a 64-bit OS
ifneq (64,$(shell getconf LONG_BIT))
    $(error CSTools will only run on a 64-bit Linux operating system)
endif

# look for a suitable compiler to use if CS-COMPILER is not already set
# if CS-COMPILER is set, just trust it
ifeq (x$(CS-COMPILER),x)
    Compiler=$(shell bash bin/Compiler-check.bash)
    ifeq (,$(Compiler))
        $(info Please set environment variable CS-COMPILER to the name of your C++ compiler.)
        $(error Cannot find a suitable C++ compiler.)
    else
        export CXX=$(Compiler)
    endif
else
    $(info CS-COMPILER is set to $(CXX))
    export CXX=$(CS-COMPILER)
endif

# set CS_ARCH for the OS are we running on
System:=$(shell test -f /usr/bin/uname && /usr/bin/uname -s)

# MacOS is reported as Darwin
ifeq (Darwin,$(System))
    export CS_ARCH=macos
else
	THIS_OS:=$(shell test -f /etc/os-release && (grep "^ID=" /etc/os-release | sed 's/ID="\([^"]*\).*/\1/'))
	THIS_OSV:=$(shell test -f /etc/os-release && (grep "^VERSION_ID=" /etc/os-release | sed 's/VERSION_ID="\([^\."]*\).*/\1/'))

    # CATS is currently RHEL7.7
    ifeq (rhel7,$(THIS_OS)$(THIS_OSV))
        export CS_ARCH=cats

    # CentOS7 is close enough RHEL7
    else ifeq (centos7,$(THIS_OS)$(THIS_OSV))
        export CS_ARCH=cats

    # assume everything else is equiv to CentOS 8 for now
    else
        export CS_ARCH=centos8
    endif
endif


# must specify make target
all: test

notest: tokens tokens-context

# C++ 17
CXXFLAGS=--std=c++17 -I. -Iincludes -O3

.PHONY: tokens tokens-context

tokens:lib/$(CS_ARCH)/tokens

lib/$(CS_ARCH)/tokens: tokens.cpp tokeniser.cpp tokeniser-basics.cpp lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o lib/$(CS_ARCH)/tokens $^

tokens-context:lib/$(CS_ARCH)/tokens-context

lib/$(CS_ARCH)/tokens-context: tokens-context.cpp tokeniser.cpp tokeniser-basics.cpp lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o lib/$(CS_ARCH)/tokens-context $^

clean:
	-@rm -f lib/*/tokens lib/*/tokens-context

# regenerate all expected test outputs using working tokeniser
test-new:
	-@bash bin/run-tests new loud

# run tests using the working tokeniser - they should all pass!
test-working:
	-@bash bin/run-tests working loud

# generate any missing expected test outputs using working tokeniser
test-add:
	-@bash bin/run-tests add loud

# regenerate all expected test outputs using working tokeniser - quietly
test-new-quiet:
	-@bash bin/run-tests new quiet

# generate any missing expected test outputs using working tokeniser - quietly
test-add-quiet:
	-@bash bin/run-tests add quiet

# run tests using the working tokeniser - they should all pass! - quietly
test-working-quiet:
	-@bash bin/run-tests working quiet

# run tests using the student tokeniser
test-tokeniser: test
test: tokens tokens-context
	-@bash bin/run-tests tokeniser loud

# run tests using the student tokeniser - quietly
test-tokeniser-quiet: test-quiet
test-quiet: tokens tokens-context
	-@bash bin/run-tests tokeniser quiet
