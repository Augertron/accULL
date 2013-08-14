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

from Tools.Dump import Dump

from Backends.Cuda.Mutators.CM_OmpParallelFor import CM_OmpParallelFor
from Backends.Cuda.Mutators.CM_OmpParallel import FM_LlcRegion

from Frontend.Parse import parse_source

from Backends.Cuda.Writers.CUDAWriter import CUDAWriter

from Frontend.InternalRepr import AstToIR


from Backends.Common.tests.common import TestCase

BACKEND_NAME = 'Cuda'
TEST_PATH = 'Backends/' + BACKEND_NAME + '/tests/'
CODE_PATH =  TEST_PATH + 'codes/'
TREE_PATH =  TEST_PATH + 'trees/'

    

class TestCudaFunctions(TestCase):

     def setUp(self):
          pass

     def test_pi(self):
          """ Test basic pi source """
          template_code = open(CODE_PATH + '/pi.c', 'r').read()
          ast = parse_source(template_code, 'pi_test')
          # new_ast = AstToIR(Writer = CUDAWriter).annotate(ast)
          migrator = AstToIR(Writer = CUDAWriter, ast = ast)
          migrator.annotate()
          new_ast = migrator.ast

          good_tree = Dump.load(TREE_PATH + '/pi_tree')
          self.check_output(new_ast, good_tree) 

     def test_picu(self):
          """ Test mutating pi to cuda """
          template_code = open(CODE_PATH + '/pi.c', 'r').read()
          ast = parse_source(template_code, 'pi_test')
#          tmp = AstToIR(Writer = CUDAWriter).annotate(ast)
          migrator = AstToIR(Writer = CUDAWriter, ast = ast)
          migrator.annotate()
          tmp = migrator.ast

          new_ast = CM_OmpParallelFor().apply_all(tmp)
          good_tree = Dump.load(TREE_PATH + '/picu_tree')
          self.check_output(new_ast, good_tree)


     def test_mandel(self):
          """ Test mandel source """
          template_code = open(CODE_PATH + '/mandel.c', 'r').read()
          ast = parse_source(template_code, 'mandel_test')
 #         new_ast = AstToIR(Writer = CUDAWriter).annotate(ast)
          migrator = AstToIR(Writer = CUDAWriter, ast = ast)
          migrator.annotate()
          new_ast = migrator.ast

          good_tree = Dump.load(TREE_PATH + '/mandel_tree')
          self.check_output(new_ast, good_tree)

     def test_mandelcu(self):
          """ Test mutating mandel to cuda """
          template_code = open(CODE_PATH + '/mandel.c', 'r').read()
          ast = parse_source(template_code, 'mandel_test')
#          tmp = AstToIR(Writer = CUDAWriter).annotate(ast)
          migrator = AstToIR(Writer = CUDAWriter, ast = ast)
          migrator.annotate()
          tmp = migrator.ast

          new_ast = CM_OmpParallelFor().apply_all(tmp)
          good_tree = Dump.load(TREE_PATH + '/mandelcu_tree')
          self.check_output(new_ast, good_tree)


     def test_jacobicu(self):
          """ Test mutating mandel to cuda """
          template_code = open(CODE_PATH + '/jacobi.c', 'r').read()
          ast = parse_source(template_code, 'jacobi_test')
#          tmp = AstToIR(Writer = CUDAWriter).annotate(ast)
          migrator = AstToIR(Writer = CUDAWriter, ast = ast)
          migrator.annotate()
          tmp = migrator.ast

          new_ast = FM_LlcRegion().apply_all(tmp)
          good_tree = Dump.load(TREE_PATH + '/jacobicu_tree')
          self.check_output(new_ast, good_tree)


if __name__ == '__main__':
    test_suite = unittest.TestLoader().loadTestsFromTestCase(TestCudaFunctions)
    unittest.TextTestRunner(verbosity=2).run(test_suite)
