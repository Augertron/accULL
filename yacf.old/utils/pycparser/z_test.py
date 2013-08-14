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

from Frontend.C import c_parser, c_ast

source_code = """
/* #include "fus.h" */
int main() {
   int i = 0;
   char a[10];
   {
   #pragma omp target device (cuda) copy_out(a)
   #pragma omp parallel shared(a)
   {
      #pragma omp for collapse(1)
      for (i = 0; i <= 10; i++) {
        for (i = 0; i <= 10; i++) {
           a[i] = 'c';
        }
      }
   }
}
}
"""

import subprocess
from cStringIO import StringIO


try:
    p = subprocess.Popen("cpp -ansi -pedantic -CC -U __USE_GNU  -P", shell=True, bufsize=1, stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
except IOError:
    exit(0)
clean_source = p.communicate(source_code)[0]

#try:
#	process = subprocess.Popen("sed -nf /home/rreyes/Frontend.C-read-only/nocomments", shell = True, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
#except IOError:
#    exit(0)
# stripped_code = process.communicate(clean_source)[0]
stripped_code = clean_source


parser = c_parser.CParser(
            lex_optimize=False,
            yacc_optimize=False,
            yacc_debug=True,
)
ast = parser.parse(stripped_code, filename = 'tutu')
ast.show()

# function_body = ast.ext[-1].body #hardcoded to the main() function

print ast.ext[-1].body.stmts[0].show()

#for stmt in function_body.stmts:
#    print stmt.coord, stmt
    
    
#~ class StructRefVisitor(c_ast.NodeVisitor):
    #~ def visit_StructRef(self, node):
        #~ print node.name.name, node.field.name


#~ parser = c_parser.CParser()
#~ ast = parser.parse(code)

#~ ast.show()

#~ v = StructRefVisitor()
#~ v.visit(ast)

