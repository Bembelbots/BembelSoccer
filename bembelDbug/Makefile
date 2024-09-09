# if you need to change any of the variables below,
# create a new file Makefile.local and add them there
-include Makefile.local

## CONFIG
# python interpreter
PYTHON ?= python3

#pyuic and pyrcc binaries
PYUIC ?= pyuic5
PYRCC ?= pyrcc5
## END CONFIG

# python main script
START_SCRIPT = src/main.py

FRAMEWORK ?= ../../soccer

rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

# filter out resources/widgets/remote_keydialog because pyuic5 bugs compiling UI with QKeySequenceEdit
UI_SRC    = $(call rwildcard,resources/widgets/,*.ui, filter-out resources/widgets/remote_keydialog.ui)
QRC_SRC   = $(call rwildcard,resources/,*.qrc)

UI_PY     = $(subst resources/widgets/,src/ui/, $(UI_SRC:.ui=_ui.py))
QRC_PY    = $(subst resources/,src/ui/, $(QRC_SRC:.qrc=_rc.py))

all: $(UI_PY) $(QRC_PY)

%_ui.py: $(UI_SRC)
	mkdir -p $(dir $@)
	touch $(dir $@)/__init__.py
	$(PYUIC) --import-from=ui $(subst src/ui/, resources/widgets/, $(@:_ui.py=.ui)) -o $@

%_rc.py: $(QRC_SRC)
	mkdir -p $(dir $@)
	$(PYRCC) $(subst src/ui/, resources/, $(@:_rc.py=.qrc)) -o $@

clean:
	rm -rf src/ui/*
