SRC1     = Sender.c
SRC2     = Receiver.c
OBJ1     = $(SRC1:.c=.o)
OBJ2     = $(SRC2:.c=.o)
CFLAGS   = -Wall -pedantic -D_GNU_SOURCE -std=gnu99
INCLUDES = -I./
LIBS     =
LIBPATH  =
DEP_FILE = .depend
SNDR     = sndr
RCVR     = rcvr

.PHONY: all clean distclean


all: depend $(SNDR) $(RCVR)

$(SNDR): $(OBJ1)
	@echo Makefile - linking $@
	@$(CC) $(LIBPATH) $(LIBS) $^ -o $@ -lrt

$(RCVR): $(OBJ2)
	@echo Makefile - linking $@
	@$(CC) $(LIBPATH) $(LIBS) $^ -o $@ -lrt

.c.o:
	@echo Makefile - compiling $<
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ -lrt

clean:
	$(RM) $(OBJ1) $(OBJ2)

distclean: clean
	$(RM) $(SNDR) $(RCVR)
	$(RM) $(DEP_FILE)

depend: $(DEP_FILE)
	@touch $(DEP_FILE)

$(DEP_FILE):
	@echo Makefile - building dependencies in: $@
	@$(CC) -E -MM $(CFLAGS) $(INCLUDES) $(SRC1) >> $(DEP_FILE)
	@$(CC) -E -MM $(CFLAGS) $(INCLUDES) $(SRC2) >> $(DEP_FILE)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
ifeq (,$(findstring distclean,$(MAKECMDGOALS)))
-include $(DEP_FILE)
endif
endif
