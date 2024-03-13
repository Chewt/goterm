SRCDIR = src
OBJDIR = obj
INCLUDEDIR = inc
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)%.c=$(OBJDIR)%.o)
INCLUDES = $(SOURCES:$(SRCDIR)%.c=$(INCLUDEDIR)%.h)
UNIDEPS = 
CFLAGS = -I$(INCLUDEDIR) -O2 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -lncursesw
CC = gcc
TARGET = goterm
VERSION = \"v1.5.0\"

.PHONY: all 
all: CFLAGS += -DVERSION=$(VERSION)
all: $(TARGET)

.PHONY: debug
debug: CFLAGS += -g -DDEBUG
debug: clean all

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INCLUDEDIR)/%.h $(UNIDEPS)
	@mkdir -p $(OBJDIR)
	$(CC) -c $< $(CFLAGS) -o $@

$(INCLUDES):

$(UNIDEPS):
	@touch $@

.PHONY: clean
clean: 
	rm -rf $(OBJDIR)/*.o $(TARGET)

.PHONY: install
install: all
	@sudo cp $(TARGET) /usr/bin/$(TARGET)

.PHONY: uninstall
uninstall: 
	@sudo rm -f /usr/bin/$(TARGET)

