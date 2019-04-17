ifeq ($(findstring Rules.mk, $(wildcard *.mk)), Rules.mk)
   include Rules.mk
endif

CXX:=gcc -g -m32

ifeq ($(CCSMPHOME),)
	CCSMPHOME:=../../../..
endif

INCDIRS:=-I$(CCSMPHOME) \
	 -I$(CCSMPHOME)/inc \
	 -I.

SIXTY_FOUR_COMPAT:=
OS:=linux
ARCH:=x86
LINUXLIBDIR=$(CCSMPHOME)/lib/$(OS)/$(ARCH)
LIBDIRS:=-L$(CCSMPHOME)/lib -L$(LINUXLIBDIR)
LLSYS:=$(SIXTY_FOUR_COMPAT)
VPATH:=$(CCSMPHOME)/src/intro
OUTPUTDIR:=$(CCSMPHOME)/bin
COMPILEFLAG:= $(COMPILEFLAG) $(INCDIRS) $(ARCHFLAGS) -DPROVIDE_LOG_UTILITIES -g
LINKFLAGS:= $(LIBDIRS) -lsolclient $(LLSYS)

$(shell mkdir -p $(OUTPUTDIR))

%.o:	%.c
	$(CXX) $(COMPILEFLAG)  $(SIXTY_FOUR_COMPAT) -c $< -o $(OUTPUTDIR)/$@

EXECS:= TopicPublisher TopicSubscriber QueuePublisher QueueSubscriber BasicReplier BasicRequestor TopicToQueueMapping MessageReplay

all: $(EXECS)

clean:
	 rm -rf $(OUTPUTDIR)/*

install:
	cp setenv.sh $(OUTPUTDIR)
	cp $(CCSMPHOME)/lib/$(OS)/$(ARCH)/*.* $(OUTPUTDIR)/
	rm $(OUTPUTDIR)/*.a
	rm $(OUTPUTDIR)/*.o

TopicPublisher : TopicPublisher.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/$^ $(LINKFLAGS)

TopicSubscriber : TopicSubscriber.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/$^ $(LINKFLAGS)

QueuePublisher : QueuePublisher.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/$^ $(LINKFLAGS)

QueueSubscriber : QueueSubscriber.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/$^ $(LINKFLAGS)

MessageReplay : MessageReplay.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/$^ $(LINKFLAGS)

BasicReplier : common.o BasicReplier.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/common.o $(OUTPUTDIR)/BasicReplier.o $(LINKFLAGS)

BasicRequestor : common.o BasicRequestor.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/common.o $(OUTPUTDIR)/BasicRequestor.o $(LINKFLAGS)

TopicToQueueMapping : common.o TopicToQueueMapping.o $(DEPENDS)
	$(CXX) -o $(OUTPUTDIR)/$@ $(OUTPUTDIR)/common.o $(OUTPUTDIR)/TopicToQueueMapping.o $(LINKFLAGS)
