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


class GMACTransformer:
    """ Apply the GMAC Transformers to a given AST
    
            Create a SourceStorage object to save all files created
            by translations.
            A separate file is used for each kernel.
    """
    
    @staticmethod
    def apply(ast, fname, output = ".", original_file = None, xmlm = None):
        """ Run the OpenMP to CUDA Translation
            
            From an ast, it produces a project directory with
                the translated sources.
        
        """
        from Backends.GMAC.Mutators.FM_LlcRegion import FM_LlcRegion
        #=======================================================================
        # st = SourceStorage.Storage('/', output + "/GMAC/")
        # # Create header and support files for the GMAC Target
        # st.addFile('llcomp_cuda', H_FileType())
        # st.append('llcomp_cuda', H_FileType(), template_header)
        #=======================================================================

        st = SourceStorage.Storage('/', output + "/GMAC/")
        # Create header and support files for the GMAC Target
        st.addFile('llcGMAC', H_FileType())
        st.append('llcGMAC', H_FileType(), "\n".join(open(config.WORKDIR + '/Backends/GMAC/Files/support/llcFunctions.h').readlines()))


        # Run the mutators
        # -> Region mutator will find the rest of llc constructs inside a particular region
        FM_LlcRegion(source_storage = st, xmlm = xmlm).apply_all(ast)
        st.addFile(fname, C_FileType())
        st.append(fname, C_FileType(), '#include "llcFunctions.h"\n')
        st.append(fname, C_FileType(), str(ast))
               
        # Storage write files when deleted
        del st
        
        return True
    
