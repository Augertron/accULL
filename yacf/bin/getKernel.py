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

'''
 :date: Feb 17, 2011
 :author: rreyes
 
'''
# from Backends.Cuda.Writers.CUDAWriter import CUDAWriter

# from sys import argv, exit
from Tools import SourceStorage
from Frontend.FrontendFactory import FrontendFactory
from Backends.Cuda.Kernelize import cuda_outline_loop_stmts
import config
# from Backends.C.Files.FileTypes import C_FileType

output = None

def parse_test(test):
    config.CURRENT_LANGUAGES = ['C99', 'Omp']
    ast_ = FrontendFactory().parse_with_language_list(test, "test", config.CURRENT_LANGUAGES)[0]
    ast_.show()
    print " OK "
    print " Migrating to Internal Representation ...."
    from Frontend.C99.C99InternalRepr import C99AstToIR
    from Backends.C99.Writers.OmpWriter import OmpWriter
    # Transform the C ast into the internal representation
    migrator = C99AstToIR(Writer = OmpWriter, ast = ast_)
    migrator.annotate()
    new_ast = migrator.ast
    print " OK "
    return new_ast

def test_kernel_extract(block_to_extract):
    ###################### Second Layer  : Transformation tools
    print "Mutating ...",
    #===============================================================================
    # from Backends.Cuda.Mutators.Common import CudaMutatorError
    # from Backends.Cuda.Runner import CudaTransformer
    # try:
    #    end_ast = CudaTransformer.apply(new_ast, stripped_filename, output,)
    # except CudaMutatorError as cme:
    #    print " Error while mutating tree "
    #    print cme
    #===============================================================================


    kernel_source = cuda_outline_loop_stmts(block_to_extract, kernel_name = "tutu")
    return kernel_source


print " Original Input::: "
test00_input = """int main() {
     int i = 0;
     int N = 10;
     char * a[N];

   for (i = 0; i < N; i++)
      a[i] = i;

   #pragma llc region name("test")
   { 
      #pragma llc for  shared(a) 
      for (i = 0; i < N; i++)
      {
            a[i] = i;
      }
     
   }

   for (i = 0; i < N; i++)
      if (a[i] != i) {
         printf(" * Not valid %d \\n ", i);
      }



}
"""

filename = "Tutu"
original_source = test00_input
block_to_extract = parse_test(original_source).ext[-1].body.block_items[-2].stmt.stmt.block_items[0].stmt.stmt.stmt
kernel_source = test_kernel_extract(block_to_extract)
print " Resulting kernel::: "
print kernel_source

