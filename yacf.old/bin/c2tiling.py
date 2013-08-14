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

from Frontend.Shortcuts import getCurrentLanguageAst
c_ast = getCurrentLanguageAst()


from Backends.C99.Writers.OmpWriter import OmpWriter
from Backends.C99.Writers.OmpWriter import CWriter

from sys import argv, exit

import config


def genTs(original_source, filename):
        ###################### First Layer  : File parsing
        # Parse file
        print "Parsing " + filename + " .... ",
        config.CURRENT_LANGUAGES = ['C99','Omp']
        # original_source = " ".join(open(filename, 'r').readlines())
        from Frontend.FrontendFactory import FrontendFactory
        [ast, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)
        print " OK "
        print " Migrating to Internal Representation ...."
        from Frontend.C99.C99InternalRepr import C99AstToIR
        # Transform the C99 ast into the intermediate representation
        migrator = C99AstToIR(Writer = OmpWriter, ast = ast)
        print "*************************"
        migrator.annotate()
        new_ast = migrator.ast
        print "*************************"
        print " OK "
        # Get the symbol table
        from Frontend.SymbolTable import *
        print " Build symbol table ...." 
        st = migrator.symbolTable
        print "*************************"
        return [new_ast, st]


output_file = None

tiling_test = """
#define N 10
#define M 10
#define P 10

int main()
{
    int a[N][M][P], b[N][M][P];
    int i, j, k;

    for (i = 0; i < N; i++) {
      a[i][0][0] = 0;
      for (j = 0; j < M; j++) {
	for (k = 0; k < P; k++) {
	  a[i][j][k] = (b[i][j-1][k] + b[i][j][k] + b[i][j+1][k]) / 3;
	}
      }
    }

    if (k != P)
	printf("** ERROR \\n");
    else
	printf("** Ok \\n");
}
"""

if len(argv) > 1:
    filename  = argv[1]
    if len(argv) > 2 :
        output_file = argv[2]
    original_source = " ".join(open(filename, 'r').readlines())
else:
    filename  = 'tiling_test'
    original_source = tiling_test

[new_ast, st] = genTs(original_source, filename)

#===========================================================================
# print "****> TS for Tiling "
# print str(st)
# print "****> For loop "
# print new_ast.ext[-1].body.block_items[-1].show()
#===========================================================================



# for_loop = new_ast.ext[-1].body.block_items[-2].stmt.loop

from MiddleEnd.Loop.Mutators.LoopTiling import LoopTilingMutator
from MiddleEnd.Loop.Common import LoopFilter


#=======================================================================
# from Tools.Debug import DotDebugTool
# DotDebugTool(highlight = [for_loop,]).apply(new_ast)
#========================================================================

# Filter first loop
first_loop = LoopFilter().apply(new_ast)

# Apply tiling mutatorFunction with a given tiling size as parameter
new_sm_ast = LoopTilingMutator().mutatorFunction(first_loop, [4,4,4])

# Also size parameter can be given through __init__ of mutator
# new_sm_ast = LoopTilingMutator(4).mutatorFunction(first_loop)


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
    new_ast.show(attrnames = True)
    print "************"
    print str(new_ast)

