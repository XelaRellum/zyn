#! /usr/bin/env python
# encoding: utf-8

# TODO: check these flags and how to add them to waf
# LIBRARIES = -DPIC -Wall
# CFLAGS := -g -fPIC -DPIC -Wall -Werror

# the following two variables are used by the target "waf dist"
VERSION='2'
APPNAME='zyn'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.parser.remove_option('--prefix') # prefix as commonly used concept has no use here, so we remove it to not add confusion
    opt.tool_options('compiler_cc')
    opt.tool_options('compiler_cxx')
    opt.tool_options('lv2plugin', tooldir='.')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check_tool('compiler_cxx')
    conf.check_tool('lv2plugin', tooldir='.')

    conf.check_pkg('fftw3', mandatory=True)
    conf.check_pkg('lv2core', mandatory=True)
    conf.check_pkg('lv2dynparamplugin1', mandatory=True)

def build(bld):
    zynadd = bld.create_obj('lv2plugin')
    zynadd.uselib = 'LV2DYNPARAMPLUGIN1 LV2CORE FFTW3'
    zynadd.target = 'zynadd'
    zynadd.ttl = ['zynadd.ttl', 'manifest.ttl']
    zynadd.source = [
        'lv2plugin.c',
        'zynadd.c',
        'util.c',
        'zynadd_dynparam.c',
        'log.c',
        'fft.c',
        'zynadd_dynparam_value_changed_callbacks.c',
        'zynadd_dynparam_forest_map.c',
        'zynadd_dynparam_forest_map_top.c',
        'zynadd_dynparam_forest_map_voice.c',
        'portamento.c',
        'oscillator.c',
        'oscillator_access.c',
        'filter_sv.c',
        'addsynth.cpp',
        'addnote.cpp',
        'lfo.cpp',
        'filter_parameters.cpp',
        'envelope_parameters.cpp',
        'filter.cpp',
        'analog_filter.cpp',
        'formant_filter.cpp',
        'envelope.cpp',
        'sv_filter.cpp',
        'resonance.cpp',
        'addsynth_component_amp_globals.cpp',
        'addsynth_component_amp_envelope.cpp',
        'addsynth_component_lfo.cpp',
        'addsynth_component_filter_globals.cpp',
        'addsynth_component_filter_analog.cpp',
        'addsynth_component_filter_formant.cpp',
        'addsynth_component_filter_sv.cpp',
        'addsynth_component_filter_envelope.cpp',
        'addsynth_component_frequency_globals.cpp',
        'addsynth_component_frequency_envelope.cpp',
        'addsynth_component_voice_globals.cpp']
