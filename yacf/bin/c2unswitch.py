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


# from Visitors.clone_visitor import CWriter, OmpWriter
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
        migrator = C99AstToIR(Writer = CWriter, ast = ast)
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


unroll_test = """
#define MAX 10
int main()
{
    int a[MAX];
    int x[MAX];
    int i = 0;
    int w;

    #pragma llc unswitch
    for (i = 0; i < MAX; i++) {
      a[i] = a[i] + x[i];
      x[i]++;
      if (w) {
        a[i] = 0;
      }
    }

    i = 2;

    return 0;
}
"""


[new_ast, st] = genTs(unroll_test, filename = 'unswitch_test')

#===============================================================================
# print "****> TS for Unswitch "
# print str(st)
# print "****> For loop "
# print new_ast.ext[-1].body.block_items[-1].show()
#===============================================================================



# for_loop = new_ast.ext[-1].body.block_items[-2].stmt.loop

from MiddleEnd.Loop.Mutators.LoopUnswitch import UnswitchLoopMutator, \
    LoopUnswitchFilter
 

for_loop = LoopUnswitchFilter().apply(new_ast).loop 

#===============================================================================
#from Tools.Debug import DotDebugTool
#DotDebugTool(highlight = [for_loop,]).apply(new_ast)
#===============================================================================

# UnswitchLoopMutator().mutatorFunction(for_loop)
UnswitchLoopMutator().fast_apply_all(new_ast,)


print str(new_ast)
