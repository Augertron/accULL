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

from Tools import SourceStorage
# from Backends.Cuda.Writers.CUDAWriter import CUDAWriter

import config
from Backends.Cuda.Files.FileTypes import Cuda_FileType
from Backends.C.Files.FileTypes import C_FileType, H_FileType
from Backends.Common.Files.FileTypes import Makefile_FileType
from Backends.Common.TemplateEngine.TemplateParser import TemplateParser

template_header = """
    /***************************************************
    
                   llcomp_cuda.h
               =====================
    
               llCoMP - c2cu driver
    
    
    See GPL.txt for license terms.
    
    Author: Ruyman Reyes Castro
    e-mail: rreyes@ull.es
    
    *****************************************************/
    #include <stdio.h>

    /* CUDA Header */
    #include "/usr/local/cuda/include/cuda.h"
    

    /* Error check for CUDA */
    void checkCUDAError (const char *msg);


    /* Fast reduction methods for CUDA */
    #include "reduction_snippets.h"

\n
"""

reduction_template_header = """
/***********************
    Reduction declaratations (header file to C parser)



    Ruyman Reyes Castro, 25 Mar 2010
*/
#define EMUSYNC



double kernelReduction_double(double *in_data, int numElems, double ini_value);

int kernelReduction_int(int *in_data, int numElems, int ini_value);


void * kernelReduction_newVector(size_t vector_size);

"""    


#### Cuda makefile template

cuda_make_template = """
NVCC=nvcc 
CC=gcc
NVCCFLAGS=-g -arch=sm_20
CCFLAGS=-g

default : all

all: main kernel
\t$(NVCC) $(NVCCFLAGS) ${original_filename}.o ${' '.join([str(elem) + '.cu' for elem in kernel_list])} -o main.exe

main:
\t$(CC) $(CCFLAGS) -c ${original_filename}.c

kernel:
%for filename in kernel_list:
\t$(NVCC) $(NVCCFLAGS) -c ${filename}.cu -o ${filename}.o
%endfor
"""


class CudaTransformer:
    """ Apply the Cuda Transformers to a given AST
    
            Create a SourceStorage object to save all files created
            by translations.
            A separate file is used for each kernel.
    """
    
    @staticmethod
    def apply(ast, fname, output = ".", original_file = None):
        """ Run the OpenMP to CUDA Translation
            
            From an ast, it produces a project directory with
                the translated sources.
        
        """
        from Backends.Cuda.Mutators.CM_OmpParallelFor import CM_OmpParallelFor
        # from Backends.Cuda.Mutators.CM_OmpParallel import CM_OmpParallel
        # CUDA Backend currently works with ABSOLUTE paths
        st = SourceStorage.Storage('/', output + "/Cuda/")
        # Create header and support files for the Cuda Target
        st.addFile('llcomp_cuda', H_FileType())
        st.append('llcomp_cuda', H_FileType(), template_header)

        # Reduction optimization
        st.addFile('reduction_snippets', H_FileType())
        st.append('reduction_snippets', H_FileType(), reduction_template_header)
        st.addFile('reduction_snippets', Cuda_FileType())
        st.append('reduction_snippets', Cuda_FileType(), 
                  ' '.join(open('Backends/Cuda/Files/support/reduction_snippets.cu').readlines()))
        # Memory manager files
        st.addFile('cuda_memoryManager', Cuda_FileType())
        st.append('cuda_memoryManager', Cuda_FileType(), 
                  ' '.join(open('Backends/Cuda/Files/support/cuda_memoryManager.cu').readlines()))

        # Run the mutators
        CM_OmpParallelFor(source_storage = st).apply_all(ast)
        # TODO Need to link parents after this?
        # CM_OmpParallel(source_storage = st).apply_all(ast)
        # Create the output file
        st.addFile(fname, C_FileType())
        st.append(fname, C_FileType(), '#include "llcomp_cuda.h"\n')
        st.append(fname, C_FileType(), str(ast))
        
        
        st.addFile('Makefile', Makefile_FileType())
        subs_dir = {'kernel_list' : ['CM_loopKernel0','reduction_snippets','cuda_memoryManager'],
                    'original_filename' : fname}
        cuda_make = TemplateParser(cuda_make_template).render(**subs_dir)
        st.append('Makefile', Makefile_FileType(), cuda_make)

        
        # Storage write files when deleted
        del st
        
        return True
    
