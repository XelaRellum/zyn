#! /usr/bin/env python
# encoding: utf-8

# the following two variables are used by the target "waf dist"
VERSION='2'
APPNAME='zyn'

# these variables are mandatory ('/' are converted automatically)
srcdir = '.'
blddir = 'build'

def set_options(opt):
    opt.tool_options('compiler_cc')
    opt.tool_options('compiler_cxx')

def configure(conf):
    conf.check_tool('compiler_cc')
    conf.check_tool('compiler_cxx')
    conf.check_tool('lv2plugin', tooldir='.')

    pkgconfig_lv2dynparam = conf.create_pkgconfig_configurator()
    pkgconfig_lv2dynparam.name = 'lv2dynparamplugin1'
    pkgconfig_lv2dynparam.run()

def build(bld):
    zynadd = bld.create_obj('lv2plugin')
    zynadd.uselib = 'LV2DYNPARAMPLUGIN1'
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
