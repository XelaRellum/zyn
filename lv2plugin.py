#! /usr/bin/env python
# encoding: utf-8
# Copyright (C) 2008 Nedko Arnaudov <nedko@arnaudov.name>

import Object
from Object import taskgen, after, before, feature
from Common import install_files
import os
import Params
import shutil

from Configure import g_maxlen
#g_maxlen = 40

def display_msg(msg, status = None, color = None):
    sr = msg
    global g_maxlen
    g_maxlen = max(g_maxlen, len(msg))
    if status:
        print "%s :" % msg.ljust(g_maxlen),
        Params.pprint(color, status)
    else:
        print "%s" % msg.ljust(g_maxlen)

def get_lv2_install_dir():
    envvar = 'LV2_PATH'

    has_lv2_path = os.environ.has_key(envvar)
    if has_lv2_path:
        display_msg("Checking LV2_PATH")
    else:
        display_msg("Checking LV2_PATH", "not set", 'YELLOW')
        return None

    if has_lv2_path:
        lv2paths = os.environ[envvar].split(':')
        for lv2path in lv2paths:
            if not os.path.isdir(lv2path):
                display_msg('  ' + lv2path, 'not directory!', 'YELLOW')
                continue

            if not os.access(lv2path, os.W_OK):
                display_msg('  ' + lv2path, 'not writable', 'YELLOW')
                continue

            display_msg('  ' + lv2path, 'looks good', 'GREEN')
            return lv2path

    return None

class lv2plugin_proxy(Object.task_gen):
    def __init__(self, tool, hook):
        Object.task_gen.__init__(self)
        self.tool = tool
        self.hook = hook

    def the_hook(self, obj, node):
        #print "-------------- '%s'" % node
        #print "tool '%s'" % self.tool
        #print "tool.target '%s'" % self.tool.target
        #print "hook '%s'" % self.hook
        #print "obj '%s'" % obj
        #print "self '%s'" % self
        self.hook(self.tool, node)

class lv2plugin(Object.task_gen):
    def __init__(self, type = 'cc', env=None):
        Object.task_gen.__init__(self)
        self.type = type
        self.tool = Object.g_allclasses[type]('shlib')

    def apply_core(self):
        #print "lv2plugin.apply_core() called."
        #print "sources: " + repr(self.source)
        #print "target: '%s'" % self.target
        #print "ttl: '%s'" % self.ttl
        self.tool.target = self.target
        self.tool.env['shlib_PATTERN'] = '%s.so'
        self.tool.uselib = self.uselib
        self.tool.ttl = self.ttl
        Object.task_gen.apply_core(self)

    def get_hook(self, ext):
        for cls in Object.g_allclasses.keys():
            if cls == 'lv2plugin':
                continue

            if cls != self.type:
                continue

            map = Object.g_allclasses[cls].mappings
            for x in map:
                if x == ext:
                    hook = map[x]
                    obj = lv2plugin_proxy(self.tool, hook)
                    return obj.the_hook

        return None

def install_target(self):
    if not Params.g_install:
        return

    self.env['LV2_INSTALL_DIR'] = get_lv2_install_dir()
    if not self.env['LV2_INSTALL_DIR']:
        Params.fatal('Cannot locate LV2 plugins directory')
    else:
        display_msg('LV2 installation directory', self.env['LV2_INSTALL_DIR'], 'GREEN')

    bundle_files = self.ttl
    bundle_files.append(self.target + '.so')
    install_files('LV2_INSTALL_DIR', self.target + '.lv2', bundle_files, self.env)

Object.register('lv2plugin', lv2plugin)

taskgen(install_target)
