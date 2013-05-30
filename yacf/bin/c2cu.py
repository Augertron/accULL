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
from Backends.Cuda.Writers.CUDAWriter import CUDAWriter

from sys import argv, exit
from Tools import SourceStorage

from Backends.C.Files.FileTypes import C_FileType

output = None

if len(argv) != 3:
	print " Incorrect number of parameters "
	print "Usage: c2cu.py original_source ProjectName"
	print "Example: bin/c2cu.py examples/pi.c PI"
	exit()

if len(argv) > 1:
	filename  = argv[1]
	if len(argv) > 2 :
		output = argv[2]
	else:
		output = "Default"

else:
	print ">>> File not found!"
	exit()



###################### First Layer  : File parsing

# Parse file
	
# from Frontend.Parse import parse_source, remove_includes

print "Parsing " + filename + " .... ", 
original_source = " ".join(open(filename, 'r').readlines())

import config
config.CURRENT_LANGUAGES = ['C99','Omp']
from Frontend.FrontendFactory import FrontendFactory
[ast_, prepro_class, lexer_class, parser_class,] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)
ast_.show()
ast = ast_
print " OK "

print " Migrating to Internal Representation ...."
from Frontend.C99.C99InternalRepr import C99AstToIR
from Backends.C99.Writers.OmpWriter import OmpWriter
# Transform the C ast into the internal representation
migrator = C99AstToIR(Writer = OmpWriter, ast = ast_)
migrator.annotate()
new_ast = migrator.ast


print " OK "

###################### Second Layer  : Transformation tools
print "Mutating ...",
# Optimize code
from MiddleEnd.Optimizer.Mutators.Optimizer import MatrixDeclToPtr

MatrixDeclToPtr(start_ast = new_ast).fast_apply_all(new_ast)

from Backends.Cuda.Mutators.Common import CudaMutatorError
from Backends.Cuda.Runner import CudaTransformer

stripped_filename = filename.split('/')[-1].split('.')[0]


try:
	end_ast = CudaTransformer.apply(new_ast, stripped_filename, output,)
except CudaMutatorError as cme:
	print " Error while mutating tree "
	print cme

################### 
# Recreate serial version in a separate storage

print "Normal file"
import config
from Tools import SourceStorage
st = SourceStorage.Storage('/', output + "/C/")
st.addFile(stripped_filename, C_FileType())
st.append(stripped_filename, C_FileType(), open(filename,'r').read())

# Delete SourceStorage ensures file saving
del st

if end_ast:
	print " OK "
else:
	print " No mutation applied "
	import sys
	sys.exit(-1)


print " OK "

## Print the AST

print " Writing result ...",
print " OK "



# Call pretty printer over the file
#--------------------------------------------------------------- if output_file:
	#---------------------------------- del v  # Close files opened by CUDA Writer
	#------------------------------------------------------------------- import os
	#------------ if os.system("indent -kr " + config.WORKDIR + output_file) != 0:
		#- print " You need to install the indent tool to pretty print ouput files "
#------------------------------------------------------------------------------ 


