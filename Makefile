include $(PVM_ROOT)/conf/$(PVM_ARCH).def

.SUFFIXES: .c

CC = egcs	# should b updated to gcc with new build


LIB = $(PVM_ROOT)/lib/$(PVM_ARCH)/libpvm3.a -lgmp
INCL = $(PVM_ROOT)/include

.c:	
	$(CC) $< -I$(INCL) $(LIB) -o $@

