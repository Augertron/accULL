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
from Backends.C99.Writers.OmpWriter import OmpWriter
from Frontend.Common.IRNode import IRNodeVisitor
from Frontend.SymbolTable import getSymbolTableFromNode, Symbol
from Tools.Tree import getFileNode, InsertTool, getContainerAttribute,\
    ReplaceTool, RemoveTool, NodeNotFound

from MiddleEnd.DataAnalysis.LlcScope import DGraphBuilder
# Get the symbol table
from Frontend.SymbolTable import SymbolTable
from Frontend.C99.C99InternalRepr import C99AstToIR

from sys import argv, exit

import config
from Frontend.C99 import c99_ast
from Backends.Common.Visitors.GenericVisitors import FilterVisitor,\
    ArrayRefVisitor, GenericFilterVisitor
from MiddleEnd.Loop.Analysis import ParametrizeLoopTool
from MiddleEnd.Loop.Mutators.LoopInvariant import LoopInvariantMutator


def genTs(original_source, filename):
    ###################### First Layer  : File parsing
    # Parse file
    print "Parsing " + filename + " .... ",
    config.CURRENT_LANGUAGES = ['C99',]
    # original_source = " ".join(open(filename, 'r').readlines())
    from Frontend.FrontendFactory import FrontendFactory
    ast =FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)[0]
    print " OK "
    print " Migrating to Internal Representation ....", 
    # Transform the C99 ast into the intermediate representation
    migrator = C99AstToIR(Writer = OmpWriter, ast = ast)
    print "*************************"
    migrator.annotate()
    ast = migrator.ast
    print "*************************"
    print " OK "
    print " Build symbol table ....", 
    _st = migrator.symbolTable
    print "*************************"
    return [ast, _st]


output_file = None

array_test = """

int func(int a, int b) {
    return a > b?a:b;
}

int main () {
  double a[100];
  double b[100];
  double c[100];
  int i, j, k;
  
  for (i = 0; i < 100; i++) 
      for (j = 0; j < 100; j++) {
          a[i*100+j] = 0.0;
          for (k = 0; k < 100; k++) {
              a[i*100+j] = a[i*100+j] + (b[i*100+k]*c[k*100+j]);
          }
      }
  
  
}
"""

[new_ast, st] = genTs(array_test, filename = 'array_test')

loop = new_ast.ext[-1].body.block_items[-1] # .ext[-1].body.block_items[-3]
loop_contents = new_ast.ext[-1].body
print "Loop contents: " + str(loop_contents)

from MiddleEnd.DataAnalysis.LlcScope import DGraph, ArrayIndexAnalysis
dGraph = DGraph()
st = getSymbolTableFromNode(getFileNode(loop))

aia = ArrayIndexAnalysis(dGraph, st)
# Get index Variables
aia.visit(new_ast.ext[-1].body)

print "Dependency Graph "
from Tools.Debug import DGraphDebug
DGraphDebug().apply(dGraph)

print " Invariant loop mutator "
lim = LoopInvariantMutator(st, dGraph,)
lim.apply_all(loop_contents)




