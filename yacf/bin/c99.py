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

from Backends.C99.Writers.C99Writer import CWriter

from sys import argv, exit
import config
import argparse
import os
from Frontend.C99.c99_prepro import C99Prepro
from datetime import datetime

# -----------------------------------
# Parse command line
parserline = argparse.ArgumentParser(description='C99 driver.')
parserline.add_argument('-I', "--include", default=False, dest="include", action="store", help="Include source directory")
parserline.add_argument('-o', "--outfile", default=False, dest="output_file", action="store", help="File output")
parserline.add_argument('program', action="store")

options = parserline.parse_args()
# -----------------------------------

# -----------------------------------
# Includes
dirs=['.']
dirs.append(os.path.dirname(options.program))
if options.include:
    for i in options.include.split(','):
        if i != '':
            dirs.append(i)

# -----------------------------------

filename = options.program
output_file = options.output_file
tstart = datetime.now()

###################### First Layer  : File parsing
# Parse file
print "Parsing " + filename + " .... ",
config.CURRENT_LANGUAGES = ['C99','GNU', ]
original_source = " ".join(open(filename, 'r').readlines())
from Frontend.FrontendFactory import FrontendFactory
[ast_, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES, dirs)
#ast_.show()
ast = ast_
print " OK "

#~ print 'Contenido de AST:::::::::::: '
#~ print (str(dir(ast_)))

#~ print " Migrating to Intermediate Representation ....", 

from Frontend.C99.C99InternalRepr import C99AstToIR
# Transform the C99 ast into the intermediate representation
print 'Contructor'
migrator = C99AstToIR(Writer = CWriter, ast = ast_)
#~ print 'Ahora al annotate'

#import pdb
#pdb.set_trace()

print 'End migrating'
#~ print '>>>>>>>>>>>>>>>>> Migrator content: ' + str(dir(migrator))
migrator.annotate()
new_ast = migrator.ast

print " OK "


############################################3
# Write file

# Call pretty printer over the file
if output_file:
    v = CWriter(filename = output_file)
    v.visit(new_ast)
    del v  # Ensure file closing
    import os
    if os.system("indent -kr " + output_file) != 0:
        print " You need to install the indent tool to pretty print ouput files "
else:
    code = str(ast)
    code = C99Prepro.writeIncludes(code)
    new_ast.show(attrnames = True)
#    print "************"
#    print ast
#    print "************"
#    print "************"
#    print code

tend = datetime.now()
print "Total time: ",
print tend - tstart
