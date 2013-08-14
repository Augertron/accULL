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

# -*- coding: utf-8 -*-

'''
 :author: Rayco Abad-Martín <rayco.abad@gmail.com>
 :author: Elena Marrero-Méndez <elemarez@gmail.com>
'''

from Backends.C.Writers.OmpWriter import OmpWriter
from Backends.C99.Writers.OmpWriter import OmpWriter
from Backends.MPI.Mutators.Common import MPIMutatorError
from Backends.MPI.Runner import MPITransformer
from Frontend.C99.C99InternalRepr import C99AstToIR
from Frontend.FrontendFactory import FrontendFactory
from MiddleEnd.Optimizer.Mutators.Optimizer import MatrixDeclToPtr
from sys import argv, exit
import config
import sys

output = None
if len(argv) > 1:
    filename  = argv[1]
    if len(argv) > 2 :
        output = argv[2]
    else:
        output = "Default"
else:
    print "Incorrect number of parameters "
    print "Usage: c2mpi.py original_source ProjectName"
    print "Example: bin/c2mpi.py examples/MPI_examples/PI/pi.c PI"
    exit()

###################### First Layer  : File parsing

# Parse file
print "Parsing " + filename + " ...\n ", 
original_source = " ".join(open(filename, 'r').readlines())
config.CURRENT_LANGUAGES = ['C99','Omp']
[ast_, prepro_class, lexer_class, parser_class,] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)
# ast_.show()
print "OK \n"

# Transform the C ast into the internal representation
print "Migrating to Internal Representation ... \n" 
migrator = C99AstToIR(Writer = OmpWriter, ast = ast_)
migrator.annotate()
new_ast = migrator.ast
print "OK \n"

###################### Second Layer  : Transformation tools

# Optimize code
print "Mutating ... \n"
MatrixDeclToPtr(start_ast = new_ast).fast_apply_all(new_ast)
stripped_filename = filename.split('/')[-1].split('.')[0]
try:
    end_ast = MPITransformer.apply(new_ast, stripped_filename, output,)
except MPIMutatorError as cme:
    print "Error while mutating tree ", cme
if end_ast:
    print "OK \n"
else:
    print "No mutation applied \n"
    sys.exit(-1)
