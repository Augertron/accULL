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
from Frontend.SymbolTable import getSymbolTableFromNode
from Tools.Tree import getFileNode

from MiddleEnd.DataAnalysis.LlcScope import DGraphBuilder
# Get the symbol table
from Frontend.SymbolTable import SymbolTable
from Frontend.C99.C99InternalRepr import C99AstToIR

from sys import argv, exit

import config


def genTs(original_source, filename):
    ###################### First Layer  : File parsing
    # Parse file
    print "Parsing " + filename + " .... ",
    config.CURRENT_LANGUAGES = ['C99','GNU']
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
  double array[10];
  int i, j, d;
  j = 3;
  array[i] = 3.0;
  
  {
    int a;
     for (i = 1; i < 10; i++) {

         for(j = 1; j < 5; j++) {
             array[i] = -1.0 * func(i,10);
             a = 3 + j + d;
         }
     }
  }
}
"""

from config_local import WORKDIR



[new_ast, st] = genTs(array_test, filename = 'array_test')


loop = new_ast # .ext[-1].body.block_items[-3]

from MiddleEnd.DataAnalysis.LlcScope import DNode,DGraph

dGraph = DGraph()
st = getSymbolTableFromNode(getFileNode(loop))
DGraphBuilder(dGraph, st).visit(loop.ext[-1].body)

if st[3][0] in dGraph[st[3][0]].succ:
   print " KO : Error, variable: " + str(st[3][0]) + " cannot be a succ of itself "
else:
   print " OK "

pi_test = """
int main ()
{
  int i;
  int n = 100;
  double pi, sum, x;
  double mysum = 0.0;
  double h;
  double a[100];
  h = 1.0 / (double) n;
  sum = 0.0;
  for (i = 0; i <= n; i++)
    {
      a[3] = a[5] * a[i];
      x = h * ((double) i - 0.5);
      sum += 4.0 / (1.0 + x * x);
    }
 {
   pi = h * sum;
 }
}
"""
del st
del new_ast
del loop
del dGraph

[new_ast, st] = genTs(pi_test, filename = 'pi_test')

loop = new_ast # .ext[-1].body.block_items[-3]

from MiddleEnd.DataAnalysis.LlcScope import DNode,DGraph

dGraph = DGraph()
st = getSymbolTableFromNode(getFileNode(loop))
DGraphBuilder(dGraph, st).visit(loop.ext[-1].body.block_items[-2].stmt)

xSymbol = st.lookUp(loop.ext[-1].body.block_items[-2].stmt.block_items[1].lvalue)
sumSymbol = st.lookUp(loop.ext[-1].body.block_items[-2].stmt.block_items[2].lvalue)
aSymbol = st.lookUp(loop.ext[-1].body.block_items[-2].stmt.block_items[0].lvalue)
iSymbol = st.lookUp(loop.ext[-1].body.block_items[-2].init.lvalue)

if dGraph.checkDependency(dGraph[sumSymbol], dGraph[xSymbol]):
    print " It works "
else:
    print " FAIL "
 
print "Dependency Graph "
from Tools.Debug import DGraphDebug
DGraphDebug().apply(dGraph)




