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

# from Visitors.clone_visitor import CWriter, OmpWriter
# from Backends.C.Writers.OmpWriter import OmpWriter
from Backends.C99.Writers.C99Writer import CWriter

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
from Frontend.Parse import parse_source
config.CURRENT_LANGUAGES = ['C99',]
original_source = " ".join(open(filename, 'r').readlines())
from Frontend.FrontendFactory import FrontendFactory
[ast, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)
ast.show()
print " OK "

print " Migrating to Intermediate Representation ....", 
from Frontend.C99.C99InternalRepr import C99AstToIR
# Transform the C99 ast into the intermediate representation
migrator = C99AstToIR(Writer = CWriter, ast = ast_)
migrator.annotate()
new_ast = migrator.ast
print " OK "

###################### Second Layer  : Transformation tools


# Optimize code
from MiddleEnd.Optimizer.Mutators.Optimizer import MatrixDeclToPtr, ConstantCalc

MatrixDeclToPtr(start_ast = new_ast).fast_apply_all(new_ast)

ConstantCalc().fast_apply_all(new_ast)


# Dot debug example 
# from Tools.Debug import DotDebugTool
# DotDebugTool(highlight = [ new_ast.ext[-1].body.stmts[-1] ]).apply(new_ast.ext[-1].body)

############################################3
# Write file


# new_ast.show()

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
    print ast

