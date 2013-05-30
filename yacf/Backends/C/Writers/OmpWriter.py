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


from Frontend.C import c_parser, c_ast

from Backends.C.Writers.CWriter import CWriter

class OmpWriter(CWriter):
    """ Visitor which translates the IR to C/OpenMP.

    """
    def visit_OmpParallel(self, node, offset):
        self.write_blank();
        self.write(offset, node.name)
        self.write_blank();
        # self.write(offset, 'parallel')
        # self.write_blank();
        if node.clauses:
            for elem in node.clauses:
                self.visit(elem)
                self.write_blank();
        if node.stmt:
            self.writeLn(offset, "")
            self.visit(node.stmt)
        self.write_blank();

    def visit_OmpThreadPrivate(self, node, offset):
        self.write_blank();
        self.write(offset, node.name)
        self.write_blank()
        if node.identifiers:
            for elem in node.identifiers.params:
                self.write(offset, '(')
                self.visit(elem)
                self.write(offset, ')')
                self.write_blank()

    def visit_OmpFor(self, node, offset):
        self.write_blank();
        self.write(offset, node.name)
        self.write_blank();
        if node.clauses:
            for elem in node.clauses:
                self.visit(elem)
                self.write_blank();

        self.write_blank();
        # OmpFor always has an stmt 
        if node.stmt:
            self.writeLn(offset, " ")
            self.visit(node.stmt)
        self.write_blank();  


    def visit_OmpParallelFor(self, node, offset):
        self.write_blank();
        self.write(offset, node.name)
        self.write_blank();

        if node.clauses:
            for elem in node.clauses:
                self.visit(elem)
                self.write_blank();

        self.write_blank();
        
        
        # OmpParallelFor always has a stmt 
        if node.stmt:
            self.writeLn(offset, " ")
            self.visit(node.stmt)
        self.write_blank();

    def visit_OmpSyncronization(self, node, offset):
        self.write_blank();
        self.write(offset, node.name)

    def visit_llcResult(self, node, offset):
        self.write_blank()
        self.write('#pragma llc result')
        self.visit(node.stmt)

    def visit_OmpClause(self, node, offset):
        # Handwrite the case of device clause
        if node.name == 'cuda':
            self.write(offset, 'device(' + node.name.lower() + ")")
        else:
            self.write(offset, node.name.lower())
        # TODO: Fix reduction parenthesis error
        if node.name == 'REDUCTION':
            self.write(0, '(');
            self.write(0, node.type)
            self.write(0, ':')

        if node.identifiers:
	        self.visit(node.identifiers)

        if node.name == 'REDUCTION':
            self.write(0, ')');


