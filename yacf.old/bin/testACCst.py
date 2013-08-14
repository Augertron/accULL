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


print " ******* FUNCTION ***** "

array_test = """
double * array;

void testfunc (double array2[10][10]) {
  int i, j;


  for (i = 0; i < 10; i++)
    for (j = 0; i < 10; i++) 
     array2[i][j] = -5.0;

}

int main () {
  double array[10][20];
  int i, j;
  array[i][j] = 3.0;
  {
  for (i = 1; i < 10; i++) {
    for (j = i; j < 10; j++) 
     array[i][j] = -1.0;
    
  }
  }

  testfunc(array);

}


"""

[new_ast, st] = genTs(array_test, filename = 'array_test')


print "****> TS for Array Test "
print str(st)

print "****> Check validation "
first_array_access = new_ast.ext[-1].body.block_items[3].lvalue
print str(st.lookUp(first_array_access))

print "****> Check validation "
parameter_array_access = new_ast.ext[1].body.block_items[-1].stmt.stmt.lvalue
print str(st.lookUp(parameter_array_access))

print '*' * 40
print st.lookUpName('array2')

arr = new_ast.ext[1].decl.type.args.params[0]
print ' --------------------------------- '
print  str(st.lookUp(arr))
print ' --------------------------------- '


#################################################################
#                       Test 2                                  #
#################################################################
function_ptr = """

void func1(char * foo, char bar[10][10])
{
  char _bar = bar[0][0];
  {
      for (int i = 0; i < N; i++)
          foo[i*N+i] = _bar;
      bar[3][2] = 'c';
  }
}

void func2(int * foo, int bar[10][10])
{
  int _bar = bar[0][0];
  {
      for (int i = 0; i < N; i++)
          foo[i*N+i] = _bar;
      bar[3][2] = 3;
  }
}

"""

[new_ast2, st2] = genTs(function_ptr, filename = 'function_ptr')


print "****> TS for Array Test "
print str(st2)


