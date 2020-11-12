# check that we are on a 64-bit OS
ifneq (64,$(shell getconf LONG_BIT))
    $(error CSTools will only run on a 64-bit Linux operating system)
endif

# look for a suitable compiler to use if CSCOMPILER is not already set
# if CSCOMPILER is set, just trust it
ifeq (x$(CSCOMPILER),x)
    Compiler=$(shell bash bin/Compiler-check.bash)
    ifeq (,$(Compiler))
        $(info Please set environment variable CSCOMPILER to the name of your C++ compiler.)
        $(error Cannot find a suitable C++ compiler.)
    else
        export CXX=$(Compiler)
    endif
else
    $(info CSCOMPILER is set to $(CXX))
    export CXX=$(CSCOMPILER)
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


# all the executables
EXECS= parser pretty codegen optimiser-r lint
TESTS=$(patsubst %,test-%,${EXECS})
MYALL:=${wildcard shared-*.cpp}
MYPARSER:=parser.cpp ${wildcard parser-*.cpp}
MYCODEGEN:=codegen.cpp ${wildcard codegen-*.cpp}
MYPRETTY:=pretty.cpp ${wildcard pretty-*.cpp}
MYOPTIMISER_R:=optimiser-r.cpp ${wildcard optimiser-r-*.cpp}
MYLINT:=lint.cpp ${wildcard lint-*.cpp}

# C++ 17 just in case
CXXFLAGS=--std=c++17 -I. -Iincludes -O3 -Wall

all: test 

notest: ${EXECS}

test: ${TESTS}

help:
	-@echo make options are one of:
	-@echo " all, parser, codegen, pretty, optimiser-r, lint"
	-@echo testing options are one of:
	-@echo " test-all, test-parser, test-codegen, test-pretty, test-optimiser-r, lint"
	-@echo

.PHONY: parser codegen pretty optimiser-r lint

# to test student versions of the programs
test-new:
	-@(bash bin/run-tests new loud)

test-parser: parser
	-@(bash bin/run-tests parser loud)

test-codegen: codegen
	-@(bash bin/run-tests codegen loud)

test-pretty: pretty
	-@(bash bin/run-tests pretty loud)

test-optimiser-r: optimiser-r
	-@(bash bin/run-tests optimiser-r loud)

test-lint: lint
	-@(bash bin/run-tests lint loud)

# to run tests with working versions of the programs
test-working: test-working-parser test-working-codegen test-working-pretty test-working-optimiser-r test-working-lint

test-working-parser:
	-@(bash bin/run-tests working-parser loud)

test-working-codegen:
	-@(bash bin/run-tests working-codegen loud)

test-working-pretty:
	-@(bash bin/run-tests working-pretty loud)

test-working-optimiser-r:
	-@(bash bin/run-tests working-optimiser-r loud)

test-working-lint:
	-@(bash bin/run-tests working-lint loud)

clean:
	-rm -rf lib/*/parser lib/*/codegen lib/*/pretty lib/*/optimiser-r lib/*/lint

parser: lib/$(CS_ARCH)/parser

lib/$(CS_ARCH)/parser: ${MYPARSER} ${MYALL} lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

codegen: lib/$(CS_ARCH)/codegen

lib/$(CS_ARCH)/codegen: ${MYCODEGEN} ${MYALL} lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

pretty: lib/$(CS_ARCH)/pretty

lib/$(CS_ARCH)/pretty: ${MYPRETTY} ${MYALL} lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

optimiser-r: lib/$(CS_ARCH)/optimiser-r

lib/$(CS_ARCH)/optimiser-r: ${MYOPTIMISER_R} ${MYALL} lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

lint: lib/$(CS_ARCH)/lint

lib/$(CS_ARCH)/lint: ${MYLINT} ${MYALL} lib/$(CS_ARCH)/lib.a
	${CXX} ${CXXFLAGS} -o $@ $^

