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

from Frontend.C99 import c99_ast as c_ast

from Backends.Common.Writers.CommonWriter import OffsetNodeVisitor

import sys

notSemicolonSet = [c_ast.Compound, c_ast.For, c_ast.Pragma, c_ast.If,
		    c_ast.FuncDef]
tabCols = 3

class CWriter(OffsetNodeVisitor):
    """ A visitor which translates the IR to plain C99 code
    """
    inside = False

    def __init__(self, filename = None, stream = None):
        self.filename = filename or sys.stdout
        self._is_a_function_definition = False
        if not stream:
            try:
                self.file = open(self.filename, 'w+') 
            except TypeError:
                self.file = sys.stdout
        else:
            try:
                self.file = stream
            except TypeError:
                print "Error: Unknown stream error"

        self.inside = False
        self._enclosing = False

    def __del__(self):
        """ Ensure closing the file when object disappears 
        """
        self.file.close()


    # ********** Writing support **********

    def writeLn(self, offset, string):
        self.write(offset, string)
        self.file.write("\n")

    def write(self, offset, string):
        self.file.write(" " * offset + string)

    def debug(self, node):
        print " >>>>> * <<<< "
        node.show()
        print " >>>> " + str(dir(node)) + "<<< "
        print " >>>>> * <<<< "

    def write_blank(self):
        self.file.write(" ")

    # ********** Support functions **********

    def generic_visit_nodeList(self, nodeList, separator, offset = 0):
        """ Visit a list of nodes , writing values within a separator """
        i = 0
        for i in range(0, len(nodeList) - 1):
            self.visit(nodeList[i])
            self.write(offset, separator)
        self.visit(nodeList[len(nodeList) - 1])

    def visit_FileAST(self, node, offset = 0):
        for elem in node.children():
            self.visit(elem, offset)
            if type(elem) not in notSemicolonSet:
                self.writeLn(0, ";")


    # ********** Grammar **********

    def visit_FuncDef(self, node, offset = 0):
        # Inherited attribute
        # self._is_a_function_definition = True
        self.writeLn(0, "")
        self.visit_Decl(node.decl, offset)
        self.writeLn(0, "")
        self.visit_Compound(node.body, offset)
        # self._is_a_function_definition = False

    def visit_ParamList(self, node, offset = 0, enclosing = True):
        if enclosing:
           self.write(offset, "(")
        self.generic_visit_nodeList(node.children(), ',', offset)
        if enclosing:
           self.write(offset, ")")
        self.write_blank()

    def visit_Compound(self, node, offset = 0):
        if (hasattr(node, "block_items") and not node.block_items):
            return
        self.writeLn(offset, "{")
        new_offset = offset + tabCols
        self.inside = True        
        for c in node.children():
            self.visit(c, offset = new_offset)
            if type(c) not in notSemicolonSet:
		self.writeLn(0, ";")
        self.inside = False
        self.writeLn(offset, "}")

    def visit_Decl(self, node, offset = 0):

        storage = "".join(['%s'%stor for stor in node.storage])
        self.write(offset, storage);
        self.write_blank()

        # In case it's a function, we need to print the parameter list in a different order
        #    so we send the node.name attribute to the children, where will be printed correctly
        #node.show()
        #print '***************************'
        #print dir(node.type)
        #print dir(node.type.type)
        #print node.type.type.show()
        #for i in node.type.type:
        #    print i
        #print '-----------------------------' 
        if isinstance(node.type, c_ast.FuncDecl):
            self.visit_FuncDecl(node.type, node.name, offset)
        else:
            if isinstance(node.type, c_ast.ArrayDecl):
                # Array declarations are also different, you need to pass the node name attribute
				# If constant => print 
                quals = " ".join(['%s'%qual for qual in node.quals])
                if (node.quals):
                    self.write(0, quals) 
                #decl_name = node.name + " ".join(['%s'%qual for qual in node.quals])
                decl_name = " " + node.name + " "
                new_offset = offset
                self.write_blank()
                
                tmp_node = node.type
		try:
                	#while not (isinstance(tmp_node, c_ast.TypeDecl) or isinstance(tmp_node, c_ast.PtrDecl)) or isinstance(tmp_node, c_ast.IdentifierType) and tmp_node:
			while tmp_node and hasattr(tmp_node, 'type'):
                    		tmp_node = tmp_node.type
		except:
			print " Malformed tree "
			import pdb
			pdb.set_trace()

                self.write_blank()
                self.visit(tmp_node, offset)
                self.write(0, decl_name)
                self.write_blank()
                self.visit_ArrayDecl(node = node.type, node_name = decl_name, offset = new_offset) 	
            
            elif isinstance(node.type, c_ast.PtrDecl):
                self.write(offset, " ")
                
                # Final type first
                tmp_node = node.type
                while not (isinstance(tmp_node, c_ast.TypeDecl)) and tmp_node:
                    tmp_node = tmp_node.type
                self.visit(tmp_node, offset)
                
                self.write(0, "(")
                
                # Then all necessary pointer types
                tmp_node = node.type
                while isinstance(tmp_node, c_ast.PtrDecl):
                    tmp_node = tmp_node.type
                    self.write(0, "*")
                
                # Then name
                # Check if node.name is not NoneType
                if type(node.name) != None.__class__:
                    self.write(0, node.name)
                self.write(0, ")")
                    
                # Then ArrayDecl if exist
                if isinstance(tmp_node, c_ast.ArrayDecl):
                    self.visit_ArrayDecl(tmp_node, node.name, offset)
		# Check if this was a pointer to a function
		if isinstance(tmp_node, c_ast.FuncDecl):
		    self.visit_ParamList(tmp_node.args) 
            else:
                self.write(offset, " ".join(['%s'%qual for qual in node.quals]))
                self.write_blank()
                self.visit(node.type, offset) 
                # Check if node.name is not NoneType
                if type(node.name) != None.__class__:
                    self.write(offset, node.name)
            if hasattr(node, 'init') and node.init:
                self.write_blank()
                self.write(0, "=")
                self.write_blank()
                if isinstance(node.init, c_ast.ExprList):
                    self.write(0, '{')
                    self._enclosing = False
                    self.visit(node.init)
                    self._enclosing = True
                    self.write(0, '}')
                else:
                    self.visit(node.init)
            # if not self.inside:
                #self.write(0, ";\n")
                #self.write_blank()
				#pass

    def visit_FuncDecl(self, node, name = "None", offset = 0 ):
        
        self.visit(node.type, offset = 0)
        self.write(offset, str(name))
        if node.args:
            self.inside = 1
            self.visit_ParamList(node.args)
            self.inside = 0
        else:
            self.write(offset, "()")
#        if (self._is_a_function_definition):
#            self.write_blank()
#        else:
#            self.writeLn(offset, ";")
    
    def visit_TypeDecl(self, node, offset = 0):
        self.generic_visit(node)
        self.write_blank()

    def visit_Typedef(self, node, offset = 0):
        # A Typedef is almost like any other Decl
        self.visit_Decl(node, offset)


    def visit_PtrDecl(self, node, offset = 0):
        
      	if not isinstance(node.type, c_ast.ArrayDecl):
	    self.visit(node.type)
	else:
	    tmp_node = node.type
	    while not isinstance(tmp_node, c_ast.TypeDecl):
	        tmp_node = tmp_node.type
	    self.visit(tmp_node)
	    self.write(offset, "(")
        # self.write(offset, node.quals)
        self.write(0, "*")
        if isinstance(node.type, c_ast.ArrayDecl):
	    self.write(0, ")")
            self.visit(node.type)
            

    def visit_RefDecl(self, node, offset = 0):
        self.visit(node.type)
        # self.write(offset, node.quals)
        self.write(offset, "&")

    # ******************** Types ********************
    def visit_IdentifierType(self, node, offset = 0):
        self.generic_visit(node, offset)
        if len(node.names) >= 1:
            #self.write(offset, node.names[0])
            # need to reverse the attributes to show in original order
            node.names.reverse()
            self.write(offset, ' '.join(node.names))
            # preserve original ordering
            node.names.reverse()

        # self.write_blank()


    def visit_FuncCall(self, node, offset = 0):
        self.visit(node.name, offset)
        if node.args:
            self.write(0, "(")
            self.generic_visit_nodeList(node.args.exprs, ", ")
            self.write(0, ")")
        else:
            self.write(0, "(")
            self.write_blank()
            self.write(0, ")")
        #self.write_blank()

    def visit_ID(self, node, offset = 0):
        self.write(offset, node.name)
        # self.write_blank()

    def visit_ExprList(self, node, offset = 0,):
        if node.exprs:
            if len(node.exprs) > 1 and self._enclosing:
                self.write(offset, "(")
            self.generic_visit_nodeList(node.exprs, ",", offset)
            if len(node.exprs) > 1 and self._enclosing:
                self.write(offset, ")")
            
        self.write_blank()
 
    def visit_Constant(self, node, offset = 0):
        if node:
            if type(node.value) != type(""):
                import pdb
                pdb.set_trace()
            self.write(offset, node.value)

    def visit_Break(self, node, offset = 0):
        self.write(offset, 'break')
        self.write_blank()

    def visit_Continue(self, node, offset = 0):
        self.write(offset, 'continue')
        self.write_blank()


    def visit_Return(self, node, offset = 0):
        self.write(offset, "return")
        self.write_blank()
        if node.expr:
            self.visit(node.expr)

    # ******************** Loops ********************
    def visit_For(self, node, offset = 0):
        self.write(offset, "for (")
        if node.init:
            prev = self.inside
            self.inside = True
            self.visit(node.init)
            self.inside = prev
        self.write(0, ";")
        self.write_blank()
        if node.cond:
            self.visit(node.cond)
        self.write(0, ";")
        self.write_blank()
        if node.next:
            self.visit(node.next)
        self.write(0, ")")
        if node.stmt:
	    self.writeLn(0, "")
            if type(node.stmt) == c_ast.Compound:
                self.visit_Compound(node.stmt, offset)
            else:
                #self.writeLn(0, "")
                new_offset = offset + tabCols
                self.visit(node.stmt, new_offset)
                self.writeLn(0, ";")
        else:
	    self.writeLn(0, ";")
        #self.write_blank()


    def visit_For99(self, node, offset = 0):
        self.write(offset, "for (")
        self.inside = 1
        for elem in node.init:
            self.visit_Decl(elem, offset)
        self.inside = 0
        self.write(0, ";")
        self.write_blank()
        self.visit(node.cond)
        self.write(0, ";")
        self.write_blank()
        self.visit(node.next)
        self.write(0, ")")
        if node.stmt:
            if type(node.stmt) == c_ast.Compound:
                self.visit_Compound(node.stmt, offset)
            else:
                self.writeLn(0, "")
                new_offset = offset + tabCols
                self.visit(node.stmt, new_offset)
        self.write_blank()


    def visit_While(self, node, offset = 0):
        self.write(offset, "while (")
        self.visit(node.cond)
        self.write(0, ")")
        self.visit(node.stmt)

    def visit_DoWhile(self, node, offset = 0):
        self.write(offset, "do")
        self.visit(node.stmt)
        self.write(offset, "while (")
        self.visit(node.cond)
        self.write(0, ")")


    # ******************** Expressions ********************
    def visit_Assignment(self, node, offset = 0):
	self.write(offset, "")
        self.visit(node.lvalue)
        self.write_blank()
        self.write(0, node.op)
        self.write_blank()
        self.visit(node.rvalue, offset = 0)
        # self.write_blank()

    def visit_TernaryOp(self, node, offset = 0):
        self.write(offset, '(')
        self.write(offset, '(')
        self.visit(node.cond)
        self.write(0, ')')
        self.write(0, '?')
        self.visit(node.iftrue)
        self.write(0, ':')
        self.visit(node.iffalse)
        self.write(0, ')')

    def visit_BinaryOp(self, node, offset = 0):
#        self.write(offset, "(")
        if (isinstance(node.left, c_ast.BinaryOp) or isinstance(node.left, c_ast.Assignment)):
            self.write(offset, "(")
            self.visit(node.left)
            self.write(offset, ")")
        else:
            self.visit(node.left)

        self.write_blank()
        self.write(0, node.op)
        self.write_blank()
        if (isinstance(node.right, c_ast.BinaryOp) or isinstance(node.left, c_ast.Assignment)):
            self.write(offset, "(")
            self.visit(node.right)
            self.write(offset, ")")
        else:   
            self.visit(node.right)
#        self.write(offset, ")")
        # self.write_blank()

    def visit_UnaryOp(self, node, offset = 0):
        
        if node.op == 'sizeof':
            # sizeof is considered as an UnaryOp
            self.write(0, 'sizeof(')
            if node.expr:
                self.visit(node.expr)
            self.write(0, ')')
            self.write_blank()
        elif node.op == '*':
            self.write(0, node.op)
            self.write(offset, "(")
            self.visit(node.expr)
            self.write(offset, ")")
            self.write_blank()
        elif len(node.op) == 1:
            self.write(offset, "(")
            self.write(0, node.op)
            self.visit(node.expr)
            self.write_blank()
            self.write(0, ")")
        else:
            self.write(offset, "(")
            # node.op is like p-- (always a p), we need to remove it
            if node.op[0] == 'p':
                   self.visit(node.expr)
                   self.write(0, node.op[1:])
                   #self.write_blank()
            else:
                   # If it is the -- operator, the p does not appear for whatever reason
                   self.write(0, node.op)
                   self.visit(node.expr)
                   #self.write_blank()
            self.write(0, ")")

        


    # ******************** Array ********************
    def visit_ArrayDecl(self, node, node_name, offset = 0):
        self.write(0, "[")
        if node.dim:
            self.visit(node.dim)
        self.write(0, "]")
        # self.write_blank()
        if isinstance(node.type, c_ast.ArrayDecl):
            self.visit_ArrayDecl(node.type, node_name, offset)


    def visit_ArrayRef(self, node, offset = 0):
        self.visit(node.name)
        self.write(0, "[")
        if node.subscript:
            self.visit(node.subscript, offset)
        self.write(0, "]")
        # self.write_blank()


    # ******************** Enum ********************
    def visit_Enum(self, node, offset = 0):
        self.write(offset, "enum")
        self.write_blank()
        if node.name:
            self.write(offset, node.name)
        if node.values:
            self.visit(node.values, offset)
        self.write_blank()


    def visit_EnumeratorList(self, node, offset = 0):
        self.write(0, "{")
        for i in range(0,len(node.enumerators)):
            self.visit(node.enumerators[i],offset)
            if i < len(node.enumerators) - 1:
                self.write(0,",")
        self.write(0, "}")

    def visit_Enumerator(self, node, offset = 0):
        self.write(offset, node.name)
        if node.value:
            self.write(0, "=")
            self.visit(node.value)
            self.write_blank()
        


    # ******************** Struct ********************

    def visit_Struct(self, node, offset = 0):
        self.write(offset, "struct")
        self.write_blank()
        if node.name:
            self.write(offset, node.name)
        if type(node.decls) != None.__class__:
            self.write(0, "{")
            for elem in node.decls:
                self.visit(elem, offset)
                self.write(0, ";")
            self.write(0, "}")
        self.write_blank()


    def visit_StructRef(self, node, offset = 0):
        self.visit(node.name)
        self.write(0, node.type)
        self.visit(node.field)
        self.write_blank()

    # ******************** Conditionals ********************
    def visit_If(self, node, offset = 0):
	self.writeLn(0, "")
        self.write(offset, "if");
        self.write_blank();
        self.write(0, "(");
        self.visit(node.cond)
        self.write(0, ")");
        #self.write_blank();
        self.writeLn(0, "")
        new_offset = offset + tabCols
        self.visit(node.iftrue, new_offset);
        if type(node.iftrue) not in notSemicolonSet:
            self.writeLn(0, ";")
        if node.iffalse:
            self.write(offset, "else");
	    self.writeLn(0, "")
	    new_offset = offset + tabCols
	    self.visit(node.iffalse, new_offset);
	    if type(node.iffalse) not in notSemicolonSet:
		self.writeLn(0, ";")
	self.writeLn(0, "")

    def visit_Switch(self, node, offset = 0):
        self.write(offset, "switch (")
        self.visit(node.cond)
        self.write(0, ")")
        self.visit(node.stmt)

    def visit_Case(self, node, offset = 0):
        self.write(offset, "case ")
        self.visit(node.expr)
        self.write(0, " : ")
        self.visit(node.stmt)

    def visit_Default(self, node, offset = 0):
        self.write(offset, "default ")
        self.write(0, " : ")
        self.visit(node.stmt)


    # ******************** Typecast ********************
    def visit_Cast(self, node, offset = 0):
        self.write(offset, '(')
        self.visit(node.to_type)
        if (hasattr(node, 'pointer')):
            if (node.pointer != None):
        	self.visit(node.pointer)
        self.write(offset, ')')
        self.write(offset, '(')
        self.visit(node.expr)
        self.write(offset, ')')

    def visit_CastPointer(self, node, offset = 0):
        self.write(offset, '*')

    # ******************** Language Extensions ********************
    def visit_Pragma(self, node, offset = 0):
	self.writeLn(offset, "")
        self.write(offset, "#pragma")
        self.write_blank()
	if type(node.name) == type(""):
	        self.write(0, node.name)
	elif node.name:
		self.visit(node.name, offset)
	
	if node.stmt:
	        self.visit(node.stmt, offset)
        self.writeLn(0, "")
    #    self.writeLn(offset, node.name)


    # ******************** Include ********************
    def visit_Include(self, node, offset = 0):
        self.write(offset, "#include <")
        self.write(0, node.name)
        self.write(offset, ">")
        self.writeLn(offset, "")


    # ******************** Enum ***********************
#    def visit_Enum(self, node, offset = 0):
#        self.write(0, "enum ")
#        self.write(0, node.name)
#        self.write(offset, "{")
#        for i in node.values.children()[:-1]:
#            self.write(offset, str(i.name))
#            self.write(offset, ',')
#        self.write(offset, str(node.values.children()[-1].name))
#        self.write(offset, "}")
#
