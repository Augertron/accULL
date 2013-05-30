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


import unittest

import subprocess
from cStringIO import StringIO
from Frontend.C import c_parser, c_ast


#from Frontend.Parse import parse_source
from Tools.Dump import Dump



def build_test_trees():
     template_code = """ 
                int main (int a) {
                    printf(" Hello World!");
                }
          """
     ast = parse_source(template_code, 'helloWorld_test')
     Dump.save('Backends/C/tests/trees/helloWorld_tree', ast)
     template_code = """ 
int main()
{
     int i;
     int sum[10];

     for (i = 0; i <= 10; i++) {
    sum[i] = i;
     }

     #pragma omp parallel for reduction(+ : sum)
     for (i = 0; i <= 10; i++) {
    sum[i] = i;
     }

}
          """
     ast = parse_source(template_code, 'pragma_test')
     Dump.save('Backends/C/tests/trees/pragma_tree', ast)

     template_code = open('Backends/C/tests/codes/jacobi_big.c', 'r').read()
     ast = parse_source(template_code, 'jacobi_c')
     Dump.save('Backends/C/tests/trees/jacobi_tree', ast)




class TestParserFunctions(unittest.TestCase):

     def setUp(self):
          self.good_tree = None 

     def test_helloWorld(self):
          template_code = """ 
                int main (int a) {
                    printf(" Hello World!");
                }
          """
          ast = parse_source(template_code, 'helloWorld_test')

          self.good_tree = Dump.load('Backends/C/tests/trees/helloWorld_tree')
          ast_str = StringIO();
          good_str = StringIO();
          ast.show(ast_str)
          self.good_tree.show(good_str)
          self.assertEqual(ast_str.getvalue(), good_str.getvalue())

     def test_pragma(self):
          template_code = """
            int main()
            {
            int i;
            int sum[10];
            for (i = 0; i <= 10; i++) {
                sum[i] = i;
            }
            #pragma omp parallel for reduction(+ : sum)
            for (i = 0; i <= 10; i++) {
                sum[i] = i;
            }
            }
            """
          ast = parse_source(template_code, 'pragma_test')
          self.good_tree = Dump.load('Backends/C/tests/trees/pragma_tree')
          ast_str = StringIO();
          good_str = StringIO();
          ast.show(ast_str)
          self.good_tree.show(good_str)
          self.assertEqual(ast_str.getvalue(), good_str.getvalue())

     def test_jacobi(self):
         template_code = open('Backends/C/tests/codes/jacobi_big.c', 'r').read()
         ast = parse_source(template_code, 'jacobi_test')
         self.good_tree = Dump.load('Backends/C/tests/trees/jacobi_tree')
         ast_str = StringIO();
         good_str = StringIO();
         ast.show(ast_str)
         self.good_tree.show(good_str)
         self.assertEqual(ast_str.getvalue(), good_str.getvalue())
 

if __name__ == '__main__':
     build_test_trees()
     unittest.main()
