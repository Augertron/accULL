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


from Frontend.C99 import c99_ast

from Backends.C99.Writers.C99Writer import CWriter

class AccWriter(CWriter):
    """ Visitor which translates the IR to C99/OpenAcc.

    """

    def _write_acc_region(self, node, offset):
        self.write_blank();
        self.write(0, node.name)
        self.write_blank();
        if node.clauses:
            print "  Region " + str(node.name)  + " contains clauses "
            for elem in node.clauses:
                print " Calling to visit_" + str(elem.__class__)
                self.visit(elem)
                self.write_blank();
        if node.stmt:
            self.writeLn(offset, "")
            self.visit(node.stmt, offset)
        self.write_blank();

    def visit_AccParallel(self, node, offset):
        self._write_acc_region(node, offset);
        
    def visit_AccKernels(self, node, offset):
        self._write_acc_region(node, offset);
 
    def visit_AccData(self, node, offset):
        self._write_acc_region(node, offset);

      
        
    def visit_AccLoop(self, node, offset):
        self.write_blank();
        self.write(0, node.name)
        self.write_blank();
        if node.clauses:
            for elem in node.clauses:
                self.visit(elem)
                self.write_blank();

        self.write_blank();
        # AccFor always has an stmt 
        if node.stmt:
            self.writeLn(offset, " ")
            self.visit(node.stmt, offset)
        self.write_blank();  

    def visit_AccSyncronization(self, node, offset):
        self.write_blank();
        self.write(offset, node.name)

    def visit_AccWait(self, node, offset):
        self.write_blank();
        self.write(offset, node.name)
        if node.value:
            self.visit(offset, node.value)


    def visit_AccParallelOption(self, node, offset):
        is_list = False
        if (type(node.value) == type([])):
		is_list = True;
        self.write(offset, node.name.lower())
	if is_list and len(node.value) > 0:
        	self.write(0, '(');
	if is_list:
            for v in node.value:
                self.visit(v)
        else:
            self.visit(node.value)
	if is_list and len(node.value) > 0:
        	self.write(0, ')');

    def visit_AccClause(self, node, offset):
        self.write(offset, node.name.lower())
        # TODO: Fix reduction parenthesis error
        if node.name == 'REDUCTION':
            self.write(0, '(');
            self.write(0, node.type)
            self.write(0, ':')

        if node.identifiers:
	        self.visit_ParamList(node.identifiers, enclosing=not (node.name == 'REDUCTION'))

        if node.name == 'REDUCTION':
            self.write(0, ')');
    
    ############################ Acc nodes

#    def visit_AccRegion(self, node, offset):
#        self.write_blank();
#        self.write(offset, node.name)
#        self.write_blank();
#        # self.write(offset, 'parallel')
#        # self.write_blank();
#        if node.clauses:
#            for elem in node.clauses:
#                self.visit(elem)
#                self.write_blank();
#        if node.stmt:
#            self.writeLn(offset, "")
#            self.visit(node.stmt)
#        self.write_blank();
#


    def visit_AccName(self, node, offset):
        self.write_blank();
        self.write(offset, "name( \"")
        self.write(offset, node.label)
        self.write(offset, "\")")
 
    def _write_copy_clause(self, node, offset):
        self.write_blank();
        self.write(offset, node.name.lower() +"(");
        if node.identifiers:
	        self.visit_ParamList(node.identifiers, enclosing=False)

        self.write(offset, ")")

    def visit_AccCopyin(self, node, offset):
        self._write_copy_clause(node, offset)
    
    def visit_AccCopyout(self, node, offset):
        self._write_copy_clause(node, offset)
 
    def visit_AccCopy(self, node, offset):
        self._write_copy_clause(node, offset)

    def visit_AccCreate(self, node, offset):
        self._write_copy_clause(node, offset)

    def visit_AccIgnore(self, node, offset):
        self.write(offset, node.code[13:-13])

