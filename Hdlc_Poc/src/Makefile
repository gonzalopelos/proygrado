all: hdlc_test 
	
CC       ?= gcc
CFLAGS   ?= -Wall -c

CXX      ?= g++
CXXFLAGS ?= -Wall -c 

LDFLAGS  ?= -Wall -g


COBJS     = fcs16.o yahdlc.o usb_controller.o
CPPOBJS   = send_data_hdlc.o
OBJS      = $(COBJS) $(CPPOBJS)
INCLUDESFLAGS ?= -I../includes
INCLUDES  += ../includes/fcs16.h ../includes/yahdlc.h ../includes/usb_controller.h


# Console Test Program
hdlc_test: $(COBJS) $(CPPOBJS)
	$(CXX) $(LDFLAGS) $(OBJS) -o hdlc_test -ludev


fcs16.o: $(INCLUDES) fcs16.c
	$(CC) $(CFLAGS) $(INCLUDESFLAGS) fcs16.c

yahdlc.o: $(INCLUDES) yahdlc.c
	$(CC) $(CFLAGS) $(INCLUDESFLAGS) yahdlc.c

usb_controller.o: $(INCLUDES) usb_controller.c
	$(CC) $(CFLAGS) $(INCLUDESFLAGS) usb_controller.c

send_data_hdlc.o: $(INCLUDES)
	$(CXX) $(CXXFLAGS) $(INCLUDESFLAGS) $(INCLUDES) usb_controller.c send_data_hdlc.cpp

clean:
	rm -f $(OBJS) hdlc_test

.PHONY: clean