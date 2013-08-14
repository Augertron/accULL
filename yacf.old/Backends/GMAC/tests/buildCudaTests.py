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


from Tools.Dump import Dump

from Backends.Cuda.Mutators.CM_OmpParallelFor import CM_OmpParallelFor
from Backends.Cuda.Mutators.CM_OmpParallel import FM_LlcRegion

from Frontend.Parse import parse_source

from Backends.Cuda.Writers.CUDAWriter import CUDAWriter

from Frontend.InternalRepr import AstToIR

BACKEND_NAME = 'Cuda'
TEST_PATH = 'Backends/' + BACKEND_NAME + '/tests/'
CODE_PATH =  TEST_PATH + 'codes/'
TREE_PATH =  TEST_PATH + 'trees/'

def build_mandel_tree():
     """ Builds the tests AST for both mandel.c and mandel.cu 
      
     """
#     [CODE_PATH, TREE_PATH] = getPath(
     # Mandel
     template_code = open(CODE_PATH + '/mandel.c', 'r').read()
     ast = parse_source(template_code, 'mandel_test')
     Dump.save(TREE_PATH + '/mandel_tree', ast)
#     tmp = AstToIR(Writer = CUDAWriter).annotate(ast)
     migrator = AstToIR(Writer = CUDAWriter, ast = ast)
     migrator.annotate()
     tmp = migrator.ast
     # CUDA version
     new_ast = CM_OmpParallelFor().apply_all(tmp) 
     Dump.save(TREE_PATH + '/mandelcu_tree', new_ast)

def build_pi_tree():
     """ Builds the tests AST for both pi.c and pi.cu 
      
     """
     # Pi
     template_code = open(CODE_PATH + '/pi.c', 'r').read()
     ast = parse_source(template_code, 'pi_test')
     Dump.save(TREE_PATH + '/pi_tree', ast)
     # CUDA version
#     new_ast = AstToIR(Writer = CUDAWriter).annotate(ast)
     migrator = AstToIR(Writer = CUDAWriter, ast = ast)
     migrator.annotate()
     new_ast = migrator.ast

     tmp = CM_OmpParallelFor().apply_all(new_ast)
     Dump.save(TREE_PATH + '/picu_tree', tmp)

def build_jacobi_tree():
     """ Builds the tests AST for both pi.c and pi.cu 
      
     """
     # Pi
     template_code = open(CODE_PATH + '/jacobi.c', 'r').read()
     ast = parse_source(template_code, 'pi_test')
     Dump.save(TREE_PATH + '/jacobi_tree', ast)
     # CUDA version
#     new_ast = AstToIR(Writer = CUDAWriter).annotate(ast)
     migrator = AstToIR(Writer = CUDAWriter, ast = ast)
     migrator.annotate()
     new_ast = migrator.ast

     tmp = FM_LlcRegion().apply_all(new_ast)
     Dump.save(TREE_PATH + '/jacobicu_tree', tmp)

