# -*- coding: utf-8 -*-
# --------------------------------------------------------------------------------
# Copyright (c) 2012, Francisco de Sande González. 
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this 
#   list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice, 
#   this list of conditions and the following disclaimer in the documentation 
#   and/or other materials provided with the distribution.
# * Neither the name of Francisco de Sande nor the names of its contributors may 
#   be used to endorse or promote products derived from this software without 
#   specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE 
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
# OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
# --------------------------------------------------------------------------------

#! /usr/bin/python

# ==================================================================
from sys import argv, exit

from Backends.Frangollo.Mutators.Common import FrangolloMutatorError
from Backends.Frangollo.Runner import FrangolloTransformer
from Frontend.SymbolTable import *

from sys import argv
from Tools import SourceStorage
from Backends.C.Files.FileTypes import C_FileType
from Tools.DescriptionFile.XMLParser import XMLManager

import optparse
import argparse
# ==================================================================

class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'

    def disable(self):
        self.HEADER = ''
        self.OKBLUE = ''
        self.OKGREEN = ''
        self.WARNING = ''
        self.FAIL = ''
        self.ENDC = ''

def colored(string, color='blue'):
	if (color == 'blue'):
		return bcolors.OKBLUE + string + bcolors.ENDC
	elif (color == 'green'):
		return bcolors.OKGREEN + string + bcolors.ENDC
	elif (color == 'fail'):
		return bcolors.FAIL + string + bcolors.ENDC

import sys
sys.setcheckinterval(1000000000)

profile = False

aparser = argparse.ArgumentParser(description='OpenAcc to Frangollo Driver')
aparser.add_argument('-I', action="append", dest="IPATH", help = "Path to additional includes")
aparser.add_argument('-L', action="store", dest="LPATH", help = "Path to additional libraries (not used)")
aparser.add_argument('-c', action="store", dest="CNAME", help = "Name of the cluster which we are generating code")
aparser.add_argument('-x', action="store", dest="XMLFILE", help = "Path and filename of the XML description file")
aparser.add_argument('source_name', action="store")
aparser.add_argument('destination_path', action="store")

options = aparser.parse_args()

import os
if os.environ.has_key('YACF_CFLAGS'):
    print os.environ['YACF_CFLAGS']

# -----------------------------------
# Includes
dirs=['.']
if options.IPATH:
    for f in options.IPATH:
	    for i in f.split(','):
        	if i != '':
	            dirs.append(i)
# -----------------------------------

filename = options.source_name
output_file = options.destination_path

xmlm = None
if options.XMLFILE:
    xmlm = XMLManager(options.XMLFILE)
# print " XML Option " + str(xmlm)


###################### First Layer  : File parsing
# Parse file
#===============================================================================
print "Parsing " + filename + " .... " 
original_source = " ".join(open(filename, 'r').readlines())
#===============================================================================

import config
config.CURRENT_LANGUAGES = ['C99','Omp',]

from Frontend.FrontendFactory import FrontendFactory
[ast_, prepro_class, lexer_class, parser_class,] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES,dirs)
#ast_.show()
ast = ast_
print " OK "


print " Migrating to Internal Representation ....", 
from Frontend.C99.C99InternalRepr import C99AstToIR
from Backends.C99.Writers.OmpWriter import OmpWriter

# Transform the C99 ast into the internal representation
migrator = C99AstToIR(Writer = OmpWriter, ast = ast_)

print 'ok'
print 'Annotate ....',
migrator.annotate()
new_ast = migrator.ast
print "OK "


# ===========================================================
print colored(" Building SymbolTable ")
st = migrator.symbolTable
print st
print " ==========================="
print colored(" OK")

##print st.lookUpName('tutu')
print " ==========================="

# ===========================================================


# Call pretty printer over the file
if output_file:
    v = OmpWriter(filename = output_file)
    v.visit(new_ast)
    del v  # Ensure file closing
    import os
    if os.system("indent -kr " + output_file) != 0:
        print " You need to install the indent tool to pretty print ouput files "
else:
    new_ast.show(attrnames = True)
    print "************"
    print new_ast


stripped_filename = filename.split('/')[-1].split('.')[0]

st = SourceStorage.Storage('/', output_file + "C/")
st.addFile(stripped_filename, C_FileType())
st.append(stripped_filename, C_FileType(), open(filename,'r').read())

end_ast = None
# Delete SourceStorage ensures file saving
del st

if end_ast:
    print colored("OK", 'green')
else:
    print colored(" No mutation applied ", 'red')
    import sys
    sys.exit(-1)

