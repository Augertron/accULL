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
Created on Nov 24, 2010

@author: rreyes
'''


from Backends.Cuda.Writers.CUDAWriter import CUDAWriter

from sys import argv, exit

output = None

if len(argv) != 2:
    print " Incorrect number of parameters "
    print "Usage: c2cu.py original_source"
    print "Example: bin/c2cu.py examples/pi.c"
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
    
from Frontend.Parse import parse_source, remove_includes

print "Parsing " + filename + " .... ", 
template_code = "".join(remove_includes(open(filename, 'r').readlines()))

ast = parse_source(template_code, filename)

print " OK "

print " Migrating to Internal Representation ...."


from Frontend.InternalRepr import AstToIR
# Transform the C ast into the internal representation
migrator = AstToIR(Writer = CUDAWriter, ast = ast)
migrator.annotate()
new_ast = migrator.ast


print " OK "

###################### Second Layer  : Transformation tools
print "Mutating ...",

from Frontend.C import c_ast
from Backends.Common.Visitors.GenericVisitors import GlobalTypedefFilter


for decl in GlobalTypedefFilter().iterate(new_ast):
    print str(decl)


if __name__ == '__main__':
    pass