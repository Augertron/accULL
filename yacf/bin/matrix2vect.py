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
from Backends.C99.Writers.OmpWriter import CWriter

from sys import argv, exit

import config


def genTs(original_source, filename):
        ###################### First Layer  : File parsing
        # Parse file
        print "Parsing " + filename + " .... ",
        config.CURRENT_LANGUAGES = ['C99',]
        # original_source = " ".join(open(filename, 'r').readlines())
        from Frontend.FrontendFactory import FrontendFactory
        [ast, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)
        print " OK "
        print " Migrating to Internal Representation ....", 
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
        print " Build symbol table ....", 
        st = migrator.symbolTable
        print "*************************"
        return [new_ast, st]


output_file = None


struct_test = """

int array_sum(int array[20][30], int c) {
    array[3][5] = c;
    
    (*array + 10 + 20) = 10; 
}

int main() {

    int c[20][30];

    c[3][2] = 3;

    if(c[3][7] == c[3][2]) {
         printf(\"Value %d \\n\", c[3][2]);
    }
    
    for (int i = 0; i < c[3][2]; i++) {
      c[i][0] = c[3][2];
   }


    c[3][2] = 3;
}
"""


[new_ast, st] = genTs(struct_test, filename = 'flatten_test')

print "****> Flatten test (TS)"
print str(st)
print "****> Lookup check "
array_symbol = st.lookUp(new_ast.ext[-1].body.block_items[-1].lvalue)
print str(array_symbol)

print "***> Decl of matrix 2 vect "

from MiddleEnd.Optimizer.Mutators.Optimizer import MatrixDeclToPtr, VectRef2Ptr, VectDecl2Ptr
MatrixDeclToPtr(start_ast = new_ast).fast_apply_all(new_ast)

vptr = VectRef2Ptr(vect_name = array_symbol.name).fast_apply_all(new_ast)
# new_ast.show()

# print str(new_ast)


vptr2 = VectDecl2Ptr(vect_name = array_symbol.name).fast_apply_all(new_ast)

print str(new_ast)

