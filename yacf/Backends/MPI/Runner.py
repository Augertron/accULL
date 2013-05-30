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

# -*- coding: utf-8 -*-

'''
 :author: Rayco Abad-Martín <rayco.abad@gmail.com>
 :author: Elena Marrero-Méndez <elemarez@gmail.com>
'''

from Backends.C.Files.FileTypes import C_FileType, H_FileType
from Backends.Common.Files.FileTypes import Makefile_FileType
from Backends.Common.TemplateEngine.TemplateParser import TemplateParser
from Backends.MPI.Mutators.MPI_MainCreator import MPI_MainCreator
from Backends.MPI.Mutators.MPI_OmpParallel import MPI_OmpParallel
from Backends.MPI.Mutators.MPI_OmpParallelFor import MPI_OmpParallelFor
from Tools import SourceStorage
import config

mpi_make_template = """
#   Include Makefile

# Compilers
#
# ANSI C Compiler
CC_COMP                 = mpicc
# Optimization global flags
CC_GOPTFLAGS    =
# Lib global flags
CC_GLIBFLAGS    =
# Debbuging global flags
CC_GDBGFLAGS        =
# Warning global flags
CC_GWARNFLAGS   =
# Other global flags
CC_GCFLAGS      = -lm
CC              = $(CC_COMP) $(CC_GOPTFLAGS) $(CC_GDBGFLAGS) $(CC_GLIBFLAGS) $(CC_GWARNFLAGS) $(CC_GCFLAGS)

# MPI Compiler
MPICC_COMP      = mpicc
# Optimization global flags
MPICC_GOPTFLAGS =
# Other global flags  
MPICC_GFLAGS        = -lm
# Libs
MPICC_GLIBS         =
MPICC               = $(MPICC_COMP) $(MPICC_GOPTFLAGS) $(MPICC_GFLAGS) $(MPICC_GLIBS)

# Dependences
BIN      = $(SRC:.c=)
UOBJ     = $(USRC:.c=.o)
.SUFFIXES: .c .o .f .l .y .tex .aux

# Rules
CLEAN_LIST      = *.o core 
DEL_LIST        = $(TARGETS) *.o core 
START_LIST      = *.o *.err *.log core

.c.o:
\t$(CC) -c  $< $(CFLAGS)

default: all

clean:
\trm -f $(CLEAN_LIST)

del:
\trm -f $(DEL_LIST)

start:
\trm -f $(START_LIST)

# Sources
# Set here source file name. 
SRC         = ${original_filename}.c
# Set here dependences for source file.
DEPEND      = 
# If you need auxiliar files .c for source file, set here its names.
USRC        = ${' '.join([str(elem) for elem in others_c_files])}

# Local flags
# Local debug flags
DBG_LFLAGS  =
# Local warning flags
WARN_LFLAGS =
# Local optmitation flags
OPT_LFLAGS  = -O3
# Other local flags
L_FLAGS         =
CFLAGS  = $(DBG_LFLAGS) $(WARN_LFLAGS) $(OPT_LFLAGS) $(L_FLAGS)

# Local libs 
L_LIBS          =
# Targets
TARGETS  = $(BIN)

all: $(TARGETS)

$(BIN): $(SRC) $(UOBJ)
\t$(MPICC) -o $@ $(SRC) $(DEPEND) $(UOBJ) $(CFLAGS) $(L_LIBS)

"""

class MPITransformer:
    """ 
        Apply the MPI Transformers to a given AST
        Create a SourceStorage object to save all files created by translations.
        A separate file is used for each kernel.
    """
    
    @staticmethod
    def apply(ast, fname, output = ".", original_file = None):
        """ 
            Run the OpenMP to MPI Translation
            From an ast, it produces a project directory with the translated sources.
        """
        st = SourceStorage.Storage(config.WORKDIR, output + "/MPI/")
        
        # Create header and support files for the MPI Target
        st.addFile('llcomp_llc', H_FileType())
        st.append('llcomp_llc', H_FileType(), open(config.WORKDIR + '/Backends/MPI/Templates/include/llcomp_llc.h', 'r').read())
        
        # Run the mutators
        MPI_MainCreator(source_storage = st).fast_apply_all(ast)
        MPI_OmpParallelFor(source_storage = st).apply_all(ast)
        MPI_OmpParallel(source_storage = st).apply_all(ast)
        
        # Create the output file
        st.addFile(fname, C_FileType())
        st.append(fname, C_FileType(), '#include "llcomp_llc.h"\n')
        st.append(fname, C_FileType(), str(ast))
        
        # Create the Makefile 
        st.addFile('Makefile', Makefile_FileType())
        subs_dir = {'original_filename' : fname, 'others_c_files' : ['uni.c', 'loop.c'],}
        mpi_make = TemplateParser(mpi_make_template).render(**subs_dir)
        st.append('Makefile', Makefile_FileType(), mpi_make)
        
        # Storage write files when deleted
        del st
        return True
