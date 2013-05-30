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


from sys import argv, exit

output_file = None

if len(argv) > 1:
	filename  = argv[1]
	if len(argv) > 2 :
		output_file = argv[2]
else:
	print ">>> File not found!"
	exit()

###################### First Layer  : File parsing
# Parse file
#===============================================================================
print "Parsing " + filename + " .... " 
original_source = " ".join(open(filename, 'r').readlines())
# stripped_source = C99_Prepro.parse_source(template_code)
# print 'Prepro ....... ok'
# ast_ = omp_parser.AccParser(lex_optimize = False, yacc_optimize = False).parse(stripped_source, filename)
#===============================================================================

import config
config.CURRENT_LANGUAGES = ['C99','Acc',]
from Frontend.FrontendFactory import FrontendFactory
[ast_, prepro_class, lexer_class, parser_class,] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)
ast_.show()
ast = ast_
print " OK "



print " Migrating to Internal Representation ....", 
from Frontend.C99.C99InternalRepr import C99AstToIR
from Backends.C99.Writers.AccWriter import AccWriter

# Transform the C99 ast into the internal representation
migrator = C99AstToIR(Writer = AccWriter, ast = ast_)


print 'ok'
print 'Annotate ....',
migrator.annotate()
new_ast = migrator.ast
print "OK "


# Call pretty printer over the file
if output_file:
    v = AccWriter(filename = output_file)
    v.visit(new_ast)
    del v  # Ensure file closing
    import os
    if os.system("indent -kr " + output_file) != 0:
        print " You need to install the indent tool to pretty print ouput files "
else:
    new_ast.show(attrnames = True)
    print "************"
    print new_ast

