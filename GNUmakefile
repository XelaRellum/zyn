# Copyright (C) 2006,2007 Nedko Arnaudov <nedko@arnaudov.name>
#  
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.

PLUGIN_NAME = zynadd
VERSION = 0
LIBRARIES = -DPIC -Wall $(strip $(shell pkg-config --libs fftw3 lv2dynparamplugin-1))

CC = gcc -c
CXX = g++ -c
CFLAGS := -g -fPIC -DPIC -Wall -Werror $(strip $(shell pkg-config --cflags fftw3 lv2dynparamplugin-1))

GENDEP_SED_EXPR = "s/^\\(.*\\)\\.o *: /$(subst /,\/,$(@:.d=.o)) $(subst /,\/,$@) : /g"
GENDEP_C = set -e; $(GCC_PREFIX)gcc -MM $(CFLAGS) $< | sed $(GENDEP_SED_EXPR) > $@; [ -s $@ ] || rm -f $@
GENDEP_CXX = set -e; $(GCC_PREFIX)g++ -MM $(CXXFLAGS) $< | sed $(GENDEP_SED_EXPR) > $@; [ -s $@ ] || rm -f $@

# These files go into the bundle
BUNDLE_FILES = manifest.ttl zynadd.ttl zynadd.so

# These are the source files for the plugin
PLUGIN_SOURCES_CXX = addsynth.cpp

PLUGIN_SOURCES_CXX += addnote.cpp
PLUGIN_SOURCES_CXX += lfo.cpp
PLUGIN_SOURCES_CXX += filter_parameters.cpp
PLUGIN_SOURCES_CXX += envelope_parameters.cpp
PLUGIN_SOURCES_CXX += filter.cpp
PLUGIN_SOURCES_CXX += analog_filter.cpp
PLUGIN_SOURCES_CXX += formant_filter.cpp
PLUGIN_SOURCES_CXX += envelope.cpp
PLUGIN_SOURCES_CXX += sv_filter.cpp
PLUGIN_SOURCES_CXX += resonance.cpp
PLUGIN_SOURCES_CXX += addsynth_component_amp_globals.cpp
PLUGIN_SOURCES_CXX += addsynth_component_amp_envelope.cpp
PLUGIN_SOURCES_CXX += addsynth_component_lfo.cpp
PLUGIN_SOURCES_CXX += addsynth_component_filter_globals.cpp
PLUGIN_SOURCES_CXX += addsynth_component_filter_analog.cpp
PLUGIN_SOURCES_CXX += addsynth_component_filter_formant.cpp
PLUGIN_SOURCES_CXX += addsynth_component_filter_sv.cpp
PLUGIN_SOURCES_CXX += addsynth_component_filter_envelope.cpp
PLUGIN_SOURCES_CXX += addsynth_component_frequency_globals.cpp
PLUGIN_SOURCES_CXX += addsynth_component_frequency_envelope.cpp
PLUGIN_SOURCES_CXX += addsynth_component_voice_globals.cpp

PLUGIN_SOURCES_C = lv2plugin.c zynadd.c util.c zynadd_dynparam.c log.c
PLUGIN_SOURCES_C += fft.c
PLUGIN_SOURCES_C += zynadd_dynparam_value_changed_callbacks.c
PLUGIN_SOURCES_C += zynadd_dynparam_forest_map.c
PLUGIN_SOURCES_C += zynadd_dynparam_forest_map_top.c
PLUGIN_SOURCES_C += zynadd_dynparam_forest_map_voice.c
PLUGIN_SOURCES_C += portamento.c
PLUGIN_SOURCES_C += oscillator.c
PLUGIN_SOURCES_C += oscillator_access.c
PLUGIN_SOURCES_C += filter_sv.c

# Derived variables - do not edit
PLUGIN_OBJECTS = $(subst .cpp,.o,$(PLUGIN_SOURCES_CXX)) $(subst .c,.o,$(PLUGIN_SOURCES_C))
ALL_SOURCES_CXX = $(sort $(PLUGIN_SOURCES_CXX))
ALL_SOURCES_C = $(sort $(PLUGIN_SOURCES_C))

default: $(PLUGIN_NAME).lv2

all:  $(PLUGIN_NAME).lv2 zyn

zyn: addnote_cpp.o main.o util.o
	@echo "Creating standalone zyn ..."
	@g++ -g $^ -o $@ $(LIBRARIES)

%.o:%.c
	@echo "Compiling $< to $@ ..."
	@$(CC) $(CFLAGS) $< -o $@

%.o:%.cpp
	@echo "Compiling $< to $@ ..."
	@$(CXX) $(CFLAGS) $< -o $@

# Remove all generated files
clean:
	-rm -rf $(PLUGIN_NAME).lv2 zynadd.so zynadd_gtk *.d *.o zyn

rebuild: clean zyn

# The main target - the LV2 plugin bundle
$(PLUGIN_NAME).lv2: $(BUNDLE_FILES)
	@echo "Creating LV2 bundle $@ ..."
	@rm -rf $(PLUGIN_NAME).lv2
	@mkdir $(PLUGIN_NAME).lv2
	@cp $(BUNDLE_FILES) $(PLUGIN_NAME).lv2

# The plugin module
zynadd.so: $(PLUGIN_OBJECTS)
	@echo "Creating LV2 shared library $@ ..."
	g++ -shared -fPIC $(LDFLAGS) $(PLUGIN_OBJECTS) $(LIBRARIES) -o $@
	@echo "Checking for undefined zyn symbols"
	@nm $@ |grep ' U zyn' ; RESULT=$$? ; if test $${RESULT} -eq 0; then rm zynadd.so; fi ; test ! $${RESULT} -eq 0

DEPFILES = $(ALL_SOURCES_CXX:.cpp=.d) $(ALL_SOURCES_C:.c=.d)

# All header dependencies need to be generated and included
-include  $(DEPFILES)

$(DEPFILES):

# This is how to generate the dependency files
%.d: %.cpp
	@echo "Generating dependency for $< to $@ ..."
	@$(GENDEP_CXX)

%.d: %.c
	@echo "Generating dependency for $< to $@ ..."
	@$(GENDEP_C)

install: $(PLUGIN_NAME).lv2
	@./install_lv2.sh zynadd.lv2

.install_timestamp: $(PLUGIN_NAME).lv2
	@echo "Installing..."
	@sudo ./install_lv2.sh zynadd.lv2
	@touch .install_timestamp

# compile/link if needs to and install using sudo, if compile/link was not a nop
maybe_sudo_install: .install_timestamp

# run it through zynjacku and use jack_connect to connect it
test_run: maybe_sudo_install
	@(sleep 2 ; jack_snapshot restore ~/zynadd.jack_snapshot)&
	@zynjacku http://home.gna.org/zyn/zynadd/0
