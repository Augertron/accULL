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

#!/usr/bin/python

"""
	Author     : Juan Jose Fumero Alfonso
	Descripcion: YaCF Driver to show lexer, ast or final code from a source file.
	Examples   :  
				1. Show the OpenACC lexer 
				$ python bin/check.py examples/acc/pi.acc.c --showlexer --frontend Acc
				2. Show the OpenACC AST
				$ python bin/check.py examples/acc/pi.acc.c --showast --frontend Acc
				3. Show the OpenMP final code
				$ python bin/check.py Frontend/testC99/5_omp.c --showcode --frontend Omp
			
"""

from Frontend.C99.C99InternalRepr import C99AstToIR
from Frontend.FrontendFactory import FrontendFactory
from Frontend.C99.c99_prepro import C99Prepro
from Backends.C99.Writers.C99Writer import CWriter
from Backends.C99.Writers.AccWriter import AccWriter
from Backends.C99.Writers.OmpWriter import OmpWriter
from Backends.Frangollo.Mutators.Common import FrangolloMutatorError
from Backends.Frangollo.Runner import FrangolloTransformer
from Backends.C.Files.FileTypes import C_FileType
from Tools import SourceStorage
from Tools.DescriptionFile.XMLParser import XMLManager
from sys import argv, exit
import config
import argparse
import os
from datetime import datetime

# -----------------------------------
# Parse command line
parserline = argparse.ArgumentParser(description='Checker driver')
parserline.add_argument('-I', "--include", default=False, dest="include", action="store", help="Include source directory")
parserline.add_argument('-o', "--outfile", default=False, dest="output_file", action="store", help="File output")
parserline.add_argument('-l', "--showlexer", default=False, dest="showlexer", action="store_true", help="Show token from lexer")
parserline.add_argument('-a', "--showast", default=False, dest="showast", action="store_true", help="Show AST")
parserline.add_argument('-m', "--showmigrate", default=False, dest="showmigrate", action="store_true", help="Show Migration")
parserline.add_argument('-c', "--showcode", default=False, dest="showcode", action="store_true", help="Show Code")
parserline.add_argument('-f', "--frontend", default=False, dest="frontend", action="store", help="Parse with Frontend <Acc|OpenMP>")
parserline.add_argument('program', action="store")
options = parserline.parse_args()
# -----------------------------------

# -----------------------------------
# Includes
from Frontend.C99.c99_prepro import INCLUDEPATH as c99_INCLUDEPATH
dirs=c99_INCLUDEPATH
if options.include:
    for f in options.include:
	    for i in f.split(','):
        	if i != '':
	            dirs.append(i)
# -----------------------------------

filename = options.program
output_file = options.output_file
import os
if os.environ.has_key('YACF_CFLAGS'):
    print os.environ['YACF_CFLAGS']

tstart = datetime.now()

########################################################################
#                  First Layer  : File parsing
########################################################################
default = True
if (options.frontend != False):
	config.CURRENT_LANGUAGES = ['C99','GNU', options.frontend]
	default = False
else:
	config.CURRENT_LANGUAGES = ['C99','GNU']

original_source = " ".join(open(filename, 'r').readlines())
[ast_, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES, dirs)

def errfoo(msg, a, b):
    printme(msg)
    sys.exit()

def typelookup(namd):
    return False

if (options.showlexer == True):
	lex = lexer_class(errfoo, typelookup)
	lex.build()
	lex.input(original_source)
	while 1:
		tok = lex.token()
		if not tok:
			break
		print tok
########################################################################



########################################################################
#                            Show AST
########################################################################
if (options.showast == True):
    ast_.show()
########################################################################


#########################################################################
#         Migration to Internal representacion and AST annotation
########################################################################
# C99 
if (default == True):
	if (options.showmigrate == True or options.showcode == True):
		print 'Migrate to Internal Representation ...............',
		migrator = C99AstToIR(Writer = CWriter, ast = ast_)
		migrator.annotate()
		new_ast = migrator.ast
		print '[ok]'
else:
	# OpenACC
	if (options.showcode == True and options.frontend == 'Acc'):
		migrator = C99AstToIR(Writer = AccWriter, ast = ast_)
		migrator.annotate()
		new_ast = migrator.ast
		stripped_filename = filename.split('/')[-1].split('.')[0]
		ast_ = FrangolloTransformer.apply(new_ast, stripped_filename, options.output_file, xmlm = None)
	# OpenMP
	elif (options.showcode == True and options.frontend == 'Omp'):
		migrator = C99AstToIR(Writer = OmpWriter, ast = ast_)
		migrator.annotate()
		new_ast = migrator.ast
########################################################################


#########################################################################
#                Show final source code from the AST
#########################################################################
if (options.showcode == True and options.frontend != "Acc"):
	    code = str(ast_)
	    code = C99Prepro.writeIncludes(code)
	    print code
elif (options.showcode == True and options.frontend == "Acc"):
	st = SourceStorage.Storage('', options.output_file + "/C/")
	st.addFile(stripped_filename, C_FileType())
	st.append(stripped_filename, C_FileType(), open(filename,'r').read())
#########################################################################

tend = datetime.now()
print "Total time: ",
print tend - tstart
