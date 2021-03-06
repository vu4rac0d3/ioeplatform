# -*- makefile -*-
# 
#	makefile for http communication with PPC web server
#
# @author Yvan Castilloux

include ../../support/make/Makefile.include

LIB_NAME = libconfigio
SOURCE_NAME = libconfigio
SOURCES = $(SOURCE_NAME).c
RESULT_DIR = ./
CFLAGS += -I../../include

ifneq ($(HOST), mips-linux)
CFLAGS+= -g -pg
endif
ARFLAG = rcs

DYNLIB_EXTENSION = so
STATLIB_EXTENSION = a
CFLAGS += -Wall

# Link Flag
#LINK_FLAG = -L$(RESULT_DIR) -shared -Wl,-soname,$(LIB_NAME).$(DYNLIB_EXTENSION) -lc -lm -lpthread

LINK_FLAG = -shared -o $(RESULT_DIR)/$(LIB_NAME).so $(OBJECTS)

OBJECTS=$(SOURCES:.c=.o)
LDEXTRA+=$(PPCLIBPATH) $(LIBRT) $(LIBXML2) $(LIBCURL) -lpthread
LOCALINCLUDEPATH =

all: dynlib staticlib

clean:
	$(RM) -rf ./*.o ./*.d ./*.dll ./*.a ../*.a ./*.so ../*.so ../../include/$(SOURCE_NAME).h $(LIB_NAME)
	
$(LIB_NAME): $(OBJECTS)
	$(CC) $(PPCINCLUDEPATH) $(LOCALINCLUDEPATH) $(LDFLAGS) -o $@ $(OBJECTS) $(LDEXTRA)

.c.o:
	$(CC) $(PPCINCLUDEPATH) $(LOCALINCLUDEPATH) $(CFLAGS) -c -o $@ $< 
	
staticlib: $(OBJECTS)
	$(AR) $(ARFLAG) $(RESULT_DIR)/$(LIB_NAME).$(STATLIB_EXTENSION) ${OBJECTS}
	@cp ./$(LIB_NAME).a ../$(LIB_NAME).a
	@mkdir -p ../../include
	@cp ./$(SOURCE_NAME).h ../../include/.
	
dynlib: $(OBJECTS)
	$(CC) $(LINK_FLAG) $(LDEXTRA)
	@cp ./$(LIB_NAME).so ../
	@mkdir -p ../../include
	@cp ./$(SOURCE_NAME).h ../../include/.