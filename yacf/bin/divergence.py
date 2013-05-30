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

from Frontend.C import parse_file

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
from Frontend.Common.IRNode import IRNodeVisitor


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

#define MAX 10
int main () {
  
    int i, j;
    int array[MAX];
    int factor = 3;

    for (i = 0; i < MAX; i++) {
      for (j = 0; j < MAX; j++) {
                if ((i == 0) && (j == 0)) {
                   array[i * MAX + j] = array[(i + 1)  * MAX + (j + 1)] * factor;
                } else {
                   array[i * MAX + j] = array[(i + 1)  * MAX + (j + 1)] * factor;
                }
                array[i * MAX + j] = 1.0;
        }
    }  

}
"""

[new_ast, st] = genTs(array_test, filename = 'array_test')

loop = new_ast.ext[-1].body.block_items[-1] # .ext[-1].body.block_items[-3]
loop_contents = new_ast.ext[-1].body.block_items[-1].stmt.block_items[-1].stmt
print "Loop contents: " + str(loop_contents)

from MiddleEnd.DataAnalysis.LlcScope import DGraph, ArrayIndexAnalysis
dGraph = DGraph()
st = getSymbolTableFromNode(getFileNode(loop))

aia = ArrayIndexAnalysis(dGraph, st)
# Get index Variables
aia.visit(new_ast.ext[-1].body)

print "Dependency Graph "
from Tools.Debug import DGraphDebug,DotDebugTool
#DGraphDebug().apply(dGraph)

#print " Invariant loop mutator "
#lim = LoopInvariantMutator(st, dGraph,)
#lim.apply_all(loop_contents)

#loop_contents.show()
#DotDebugTool().apply(loop_contents)


# Check for optimization feasibility 

class DivergenceTestFilter(GenericFilterVisitor):
    def __init__(self, ):
        self._access = None
        self._selected_if = None
        def condition(node):
            if isinstance(node, c99_ast.If):
                return self.check_If(node)
            return False
        super(DivergenceTestFilter, self).__init__(condition_func = condition,)


    def check_assignment(self, node):
        potential_assignment = node
        if isinstance(node, c99_ast.Compound):
            # Check first node only for now
            potential_assignment = node.block_items[0]
        print " Inside "
        if isinstance(potential_assignment, c99_ast.Assignment):
            access = potential_assignment.lvalue
            if not self._access:
                print " Access is none "
                self._access = access
                return True
            elif str(self._access) == str(access):
                print " Acces is correct "
                return True
            else:
                pass
                print "  Access: " + str(access)
                print " previous access: " + str(self._access)
        return False
                
        

    def check_If(self, node):
        print " Check If "
        was_first_access = (not self._access)
        print " Was first access? " + str(was_first_access)
        # Check if both sides contain a Assginement
        if self.check_assignment(node.iftrue) and self.check_assignment(node.iffalse):
            if was_first_access:
                print " Selected "
                self._selected_if = node.iftrue
            else:
                print " It was not the first "
            return True
        return False

dtf = DivergenceTestFilter()
dtf.apply(loop_contents)

#if_stmt = loop_contents.block_items[0];

if dtf._selected_if:
    if_stmt = loop_contents.block_items[0];
else:
    print " Error: No selected if "
    import sys
    sys.exit(-1)


class IfVisitor(IRNodeVisitor):
    
    def getNestedIfCount(self):
        return self._nestedIf

    def getExpr(self):
        return c99_ast.Assignment(op = '=' , lvalue = self._lvalue, rvalue = self._expr)

    def __init__(self, lvalue):
        self._nestedIf = 0
        self._lvalue = lvalue
        self._expr = c99_ast.Constant(type = "integer", value = "0")
        super(IfVisitor, self).__init__()

    def visit_If(self, node):
        print " Visit IF "
        self._nestedIf = self._nestedIf + 1
        # True value
        tmp_true = c99_ast.BinaryOp(op = "*", left = node.cond , right = node.iftrue.block_items[0].rvalue)
        not_cond = c99_ast.UnaryOp(op = "!", expr = node.cond)

        # False
        old = self._expr 
        self._expr = c99_ast.Constant(type="integer", value = "0")
        self.visit(node.iffalse)
        tmp_false = c99_ast.BinaryOp(op = "*", left = not_cond , right = self._expr)

        # Put together True and False expressions
        tmp_expr = c99_ast.BinaryOp(op = "+", left = tmp_true, right = tmp_false)

        # Accumulate with previous expressions
        self._expr = c99_ast.BinaryOp(op = "+", left = tmp_expr, right = old)

# Create equivalent subexpression
ifv = IfVisitor(lvalue = c99_ast.ID(name = "a")) # vector_lvalue)
ifv.visit(if_stmt)
print " Nested if count : " + str(ifv.getNestedIfCount())
print " Expression : " + str(ifv.getExpr())




