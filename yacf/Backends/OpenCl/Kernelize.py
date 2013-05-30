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
Created on Feb 2, 2011

@author: rreyes
'''


from Frontend.Shortcuts import getCurrentLanguageAst
import config
from Frontend.InternalRepr import AstToIR
from Backends.Cuda.Writers.CUDAWriter import CUDAWriter
from mako.template import Template
c_ast = getCurrentLanguageAst()

from MiddleEnd.Loop.Analysis import AccLoopParametrize
from MiddleEnd.Functions.Mutators.Outliner import block_is_SESE, OutlineMutator,\
    ParameterStyle
from Backends.Common.Visitors.GenericVisitors import UndefIDFilter,\
    FuncCallFilter, FuncDeclOfNameFilter,  FuncDefOfNameFilter
from Tools.Tree import NodeNotFound
from Tools.Tree import getContainerAttribute, ReplaceTool, getFileNode
from Backends.Common.Kernelize import Kernelize


class CLKernelize(Kernelize):
    
    opencl_proto_template = """
        <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr
           from Backends.OpenCl.Templates.Functions import varlist_to_clparams
        %>
         __global__ void ${system_parameters['kernel_name']} ( 
                ${varlist_to_clparams(array_to_ptr(system_parameters['kernel_parameters']))} );
         
        """
 
    kernel_template = config.DEFAULT_INCLUDES + """
        #pragma OPENCL EXTENSION cl_khr_fp64:enable
    
        %for typedef in typedeflist:
            ${typedef};
        %endfor
        
        %for func in funccalllist:
             ${func};
        %endfor
           
        ${kernelFunction};
        
        """
   
    def kernelize_opencl_function(self,loop_parameters, system_parameters):
        opencl_template = """
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl
           from Backends.OpenCl.Templates.Functions import varlist_to_clparams
           from Backends.C99.Writers.AccWriter import AccWriter
           from Frontend.InternalRepr import AstToIR
        %>
         __kernel void ${system_parameters['kernel_name']} ( 
                ${varlist_to_clparams(array_to_ptr(system_parameters['kernel_parameters']))}
         )
         {
            int __idx__ = get_global_id(0);
            ${var_to_declaration(id_to_var_node(loop_parameters['loop_variable']))}  = (__idx__ + ${loop_parameters['init_iteration']}) * ${loop_parameters['stride']};
             %for var in system_parameters['register_vars']:
              ${var_to_declaration(var)};
            %endfor
    
    
    		  /* Basic reduction operation, see XXX */
            %for op,varlist in loop_parameters['reduction_vars']:
              %if op == '+':
              %for var in varlist:
                  ${var_to_declaration(var)}_private = 0; 
              %endfor
       	  %elif op == '*':
    	 %for var in varlist:
                  ${var_to_declaration(var)}_private = 1;  
              %endfor
    
    	  %endif
            %endfor
            
            /* We need to check if idx is inside limits (in case we have more threads than iterations) */
            if (${loop_parameters['cond_node']}) {
               %if hasattr(system_parameters['code'], 'block_items'):
                   ${";\\n".join(str(elem) for elem in AstToIR(Writer=AccWriter, ast=system_parameters['code']).ast.block_items)}; 
               %else:
                   ${AstToIR(Writer=AccWriter, ast=system_parameters['code']).ast}; 
               %endif
            }
    		  /* Basic reduction operation, see XXX */
    		  %for op,varlist in loop_parameters['reduction_vars']:
                %for var in varlist:
                  ${var.name}[__idx__] = ${var.name}_private;  // Lets assume reduction is always addition for now
                %endfor
            %endfor
    
    
         }
        """
        
        kernel_source = Template(opencl_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,)
        
        kernelProto_source = Template(self.opencl_proto_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,)
    
        return [kernel_source, kernelProto_source]
    
    
    def kernelize2d_opencl_function(self,loop_parameters, system_parameters, loop2_parameters ):
        opencl_template = """
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl
           from Backends.OpenCl.Templates.Functions import varlist_to_clparams
           from Backends.C99.Writers.AccWriter import AccWriter
           from Frontend.InternalRepr import AstToIR
        %>
         __kernel void ${system_parameters['kernel_name']} ( 
                ${varlist_to_clparams(array_to_ptr(system_parameters['kernel_parameters']))}
         )
         {
            int __idx__ = get_global_id(0);
            int __idy__ = get_global_id(1);
            ${var_to_declaration(id_to_var_node(loop_parameters['loop_variable']))}  = (__idx__ + ${loop_parameters['init_iteration']}) * ${loop_parameters['stride']};
            ${var_to_declaration(id_to_var_node(loop2_parameters['loop_variable']))}  = (__idy__ + ${loop2_parameters['init_iteration']}) * ${loop_parameters['stride']};
             %for var in system_parameters['register_vars']:
              ${var_to_declaration(var)};
            %endfor
    
    
    		  /* Basic reduction operation, see XXX */
            %for op,varlist in loop_parameters['reduction_vars']:
              %for var in varlist:
                  ${var_to_declaration(var)}_private = 0;  // Lets assume reduction is always addition for now
              %endfor
            %endfor
            
            /* We need to check if idx is inside limits (in case we have more threads than iterations) */
            if ((${loop_parameters['cond_node']}) && ${loop2_parameters['cond_node']} ){
               %if hasattr(system_parameters['code'], 'block_items'):
                   ${";\\n".join(str(elem) for elem in AstToIR(Writer=AccWriter, ast=system_parameters['code']).ast.block_items)}; 
               %else:
                   ${AstToIR(Writer=AccWriter, ast=system_parameters['code']).ast}; 
               %endif
            }
    		  /* Basic reduction operation, see XXX */
    		  %for op,varlist in loop_parameters['reduction_vars']:
                %for var in varlist:
                  ${var.name}[__idx__] = ${var.name}_private;  // Lets assume reduction is always addition for now
                %endfor
            %endfor
    
    
         }
        """
        kernel_source = Template(opencl_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,
                                                                 loop2_parameters = loop2_parameters,)
        
    
        kernelProto_source = Template(self.opencl_proto_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,)
    #                                                             outlinedFunction = outlinedFunction,)
    
        return [kernel_source, kernelProto_source]
    
    
    
    def kernelize3d_opencl_function(self,loop_parameters, system_parameters, loop2_parameters, loop3_parameters ):
        opencl_template = """
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl
           from Backends.OpenCl.Templates.Functions import varlist_to_clparams
        %>
         __kernel void ${system_parameters['kernel_name']} ( 
                ${varlist_to_clparams(array_to_ptr(system_parameters['kernel_parameters']))}
         )
         {
            int __idx__ = get_global_id(0);
            int __idy__ = get_global_id(1);
            int __idz__ = get_global_id(2);
            ${var_to_declaration(id_to_var_node(loop_parameters['loop_variable']))}  = (__idx__ + ${loop_parameters['init_iteration']}) * ${loop_parameters['stride']};
            ${var_to_declaration(id_to_var_node(loop2_parameters['loop_variable']))}  = (__idy__ + ${loop2_parameters['init_iteration']}) * ${loop2_parameters['stride']};
            ${var_to_declaration(id_to_var_node(loop3_parameters['loop_variable']))}  = (__idz__ + ${loop3_parameters['init_iteration']}) * ${loop3_parameters['stride']};
    
             %for var in system_parameters['register_vars']:
              ${var_to_declaration(var)};
            %endfor
    
    
    		  /* Basic reduction operation, see XXX */
            %for op,varlist in loop_parameters['reduction_vars']:
              %for var in varlist:
                  ${var_to_declaration(var)}_private = 0;  // Lets assume reduction is always addition for now
              %endfor
            %endfor
            
            /* We need to check if idx is inside limits (in case we have more threads than iterations) */
            if ((${loop_parameters['cond_node']}) 
                    && (${loop2_parameters['cond_node']})
                    &&  (${loop3_parameters['cond_node']}) ){
               %if hasattr(system_parameters['code'], 'block_items'):
               ${";\\n".join(str(elem) for elem in system_parameters['code'].block_items)}; 
               %else:
               ${system_parameters['code']}; 
               %endif
            }
    
    		  /* Basic reduction operation, see XXX */
    		  %for op,varlist in loop_parameters['reduction_vars']:
                %for var in varlist:
                  ${var.name}[__idx__] = ${var.name}_private;  // Lets assume reduction is always addition for now
                %endfor
            %endfor
    
    
         }
        """
        
        from mako.template import Template
    
        kernel_source = Template(opencl_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,
                                                                 loop2_parameters = loop2_parameters,
                                                                 loop3_parameters = loop3_parameters,)
    
        kernelProto_source = Template(opencl_proto_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,)
    #                                                             outlinedFunction = outlinedFunction,)
    
        return [kernel_source, kernelProto_source]    
    
    def opencl_outline_loop_stmts(self,loop_parameters, system_parameters, 
                                    objetive, 
                                    kernel_name = "tutu", 
                                    loop2_parameters = None,
                                    loop3_parameters = None, om = None, ):
        """
            Outline a cuda kernel from a For Loop
            
            :param objetive: statements from a loop (For.stmt)
        """
        # Abort if not a SESE block   
        assert block_is_SESE(objetive)
        
        if not om:    
                # Do not replace original subtree
                om = OutlineMutator(name = kernel_name + "_support", 
                                        replace = False, 
                                        change_code_block = False) 
                om.add_additional_decls([elem.node for elem in loop_parameters['global_vars']])
                om.apply(objetive,)
           

        # Nodos son distintos, pero el padre es el mismo
        #typedeflist = om.getTypedefList()
        # TODO: Funccall prototypes must be changed acording to global/local keywords
        # funccalllist = om.getFuncCallList()
        outlinedProto = om.getOutlinedPrototype()    
        outlinedFunction = om.getOutlinedFunction()
    
        non_visited_funcalls = om.funcCalls[:]
        allow_native = True
        native_list =  ['sqrt', 'min', 'max', 'log', 'fabs', 'pow']
        called_list = self.inline_called_functions(non_visited_funcalls, objetive, 
    			    native_list, allow_native)

#        assert self.is_kernelizable(outlinedFunction, loop_parameters)
        
        kernelFunction = None
        kernelProto = None
        if loop3_parameters and loop2_parameters:
            print " Use kernelize 3d"
            [kernelFunction, kernelProto] = self.kernelize3d_opencl_function(loop_parameters, 
                                                                        system_parameters, 
                                                                        loop2_parameters, loop3_parameters)
        elif loop2_parameters:
            print " Use kernelize 2d (OpenCL)"
            [kernelFunction, kernelProto] = self.kernelize2d_opencl_function(loop_parameters, system_parameters, loop2_parameters)
        else:
            [kernelFunction, kernelProto] = self.kernelize_opencl_function(loop_parameters, system_parameters)
     
     
        #===========================================================================
        kernel_source = Template(self.kernel_template).render_unicode(typedeflist = om.getTypedefList(),
                                                                 funccalllist = called_list,
                                                                 outlinedProto = outlinedProto,
                                                                 kernelProto = kernelProto,
                                                                 kernelFunction = kernelFunction,
                                                                 )
         
        return kernel_source

    def kernelize_opencl_stmts(self, system_parameters):
        opencl_template = """
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl
        %>
         __global__ void ${system_parameters['kernel_name']} ( 
                ${varlist_to_funcproto(array_to_ptr(system_parameters['kernel_parameters']))}
         )
         {
            int __idx__ = blockIdx.x * blockDim.x + threadIdx.x;
            %for var in system_parameters['register_vars']:
              ${var_to_declaration(var)};
            %endfor
     
            
               %if hasattr(system_parameters['code'], 'block_items'):
               ${";\\n".join(str(elem) for elem in system_parameters['code'].block_items)}; 
               %else:
               ${system_parameters['code']}; 
               %endif
            
    		      
         }
        """
        kernel_source = Template(opencl_template).render_unicode(system_parameters = system_parameters,)
        kernelProto_source = Template(self.opencl_proto_template).render_unicode(system_parameters = system_parameters)
        return [kernel_source, kernelProto_source]


    
    def opencl_outline_stmts(self, system_parameters, objetive, 
                                    kernel_name, om ):
        """
            Write a block statement as an OpenCL kernel (each thread executes the entire statement)
            
            :param objetive: block statement (stmt)
        """ 
        
        #typedeflist = om.getTypedefList() 
        #called_list = []
        allow_native = True
        native_list =  ['sqrt', 'min', 'max', 'log', 'fabs', 'pow',]
        non_visited_funcalls = om.funcCalls[:]
        called_list = self.inline_called_functions(non_visited_funcalls, objetive, 
    			    native_list, allow_native)
        outlinedProto = om.getOutlinedPrototype()    
        outlinedFunction = om.getOutlinedFunction()
            
        assert self.is_kernelizable(outlinedFunction, None)
        
        kernelFunction = None
        kernelProto = None
        [kernelFunction, kernelProto] = self.kernelize_opencl_stmts(system_parameters)
         
        
        kernel_source = Template(self.kernel_template).render_unicode(typedeflist = om.getTypedefList(),
                                                                 funccalllist = called_list,
                                                                 outlinedProto = outlinedProto,
                                                                 kernelProto = kernelProto,
                                                                 kernelFunction = kernelFunction,
                                                                 )
        return kernel_source
 
