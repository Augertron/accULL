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

import config

from Frontend.Shortcuts import getCurrentLanguageAst
from Frontend.InternalRepr import AstToIR
from Backends.Cuda.Writers.CUDAWriter import CUDAWriter
c_ast = getCurrentLanguageAst()
from Backends.Common.Visitors.GenericVisitors import UndefIDFilter,\
    FuncCallFilter, FuncDeclOfNameFilter,  FuncDefOfNameFilter

from Backends.Cuda.Platform import build_cuda_system_parameters
    
from MiddleEnd.Functions.Mutators.Outliner import block_is_SESE, OutlineMutator,\
        ParameterStyle

from mako.template import Template

from Tools.Tree import getFileNode, NodeNotFound


from Backends.Common.Kernelize import Kernelize

class CudaKernelize(Kernelize):
    cuda_proto_template = """
        <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr
        %>
         __global__ void ${system_parameters['kernel_name']} ( 
                ${varlist_to_funcproto(array_to_ptr(system_parameters['kernel_parameters']))} );
         
        """
    kernel_template = config.DEFAULT_INCLUDES + """
        %for typedef in typedeflist:
            ${typedef};
        %endfor
        
        %for func in funccalllist:
            __device__ ${func};
        %endfor
        
        extern "C" {
        
        ${kernelProto};
        
        };
        
        ${kernelFunction};
        
        
     

   """

    def kernelize_cuda_stmts(self, system_parameters):
        cuda_template = """
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
        kernel_source = Template(cuda_template).render_unicode(system_parameters = system_parameters,)
        kernelProto_source = Template(self.cuda_proto_template).render_unicode(system_parameters = system_parameters)
        return [kernel_source, kernelProto_source]

    def kernelize_cuda_stmts_loop(self, loop_parameters, system_parameters):
        cuda_template = """
         {
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl
        %>
            ${var_to_declaration(id_to_var_node(loop_parameters['loop_variable']))}  = (__idx__ + ${loop_parameters['init_iteration']}) * ${loop_parameters['stride']};         

            if (loop_parameters['loop_variable'] < loop_parameters['last_iteration']) {
                ${system_parameters['code']};
            }
         }
        """
        kernel_source = Template(cuda_template).render_unicode(system_parameters = system_parameters,)
        return kernel_source
    
    
    def kernelize_cuda_function(self,loop_parameters, system_parameters):
        cuda_template = """
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl, var_to_extended_declaration
           from Frontend.C99 import c99_ast
           from Backends.C99.Writers.AccWriter import AccWriter
           from Frontend.InternalRepr import AstToIR
        %>
         __global__ void ${system_parameters['kernel_name']} ( 
                ${varlist_to_funcproto(array_to_ptr(system_parameters['kernel_parameters']))}
         )
         {
            int __idx__ = blockIdx.x * blockDim.x + threadIdx.x;
            ${var_to_declaration(id_to_var_node(loop_parameters['loop_variable']))}  = (__idx__ + ${loop_parameters['init_iteration']}) * ${loop_parameters['stride']};
            
            %for var in system_parameters['register_vars']:
              ${var_to_extended_declaration(var)};
            %endfor
     
    
    		  /* Basic reduction operation, see XXX */
            %for op,varlist in loop_parameters['reduction_vars']:
                %if op == '+':
           	   %for var in varlist:
           	       ${var_to_declaration(var)}_private = 0;  // Lets assume reduction is always addition for now
           	   %endfor
    	     % elif op == '*':
    	   %for var in varlist:
           	       ${var_to_declaration(var)}_private = 1;  // Lets assume reduction is always addition for now
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
        
        
        # Apply all available source mutators to the template
    
        _loop_parameters = loop_parameters
        kernel_source = Template(cuda_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = _loop_parameters,)
    
        kernelProto_source = Template(self.cuda_proto_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,)
    
        return [kernel_source, kernelProto_source]
    
    
    def kernelize2d_cuda_function(self,loop_parameters, system_parameters, loop2_parameters):
        cuda_template = """
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl
           from Frontend.C99 import c99_ast
           from Backends.C99.Writers.AccWriter import AccWriter
           from Frontend.InternalRepr import AstToIR
        %>
         __global__ void ${system_parameters['kernel_name']} ( 
                ${varlist_to_funcproto(array_to_ptr(system_parameters['kernel_parameters']))}
         )
         {
            int __idx__ = blockIdx.x * blockDim.x + threadIdx.x;
            int __idy__ = blockIdx.y * blockDim.y + threadIdx.y;
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
        # Apply all available source mutators to the template
        kernel_source = Template(cuda_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,
                                                                 loop2_parameters = loop2_parameters)
    
        kernelProto_source = Template(self.cuda_proto_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,)
    
        return [kernel_source, kernelProto_source]
    
    
    def kernelize3d_cuda_function(self, loop_parameters, system_parameters, loop2_parameters, loop3_parameters):
        cuda_template = """
         <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, array_to_ptr, write_left_part_of_var_decl
           from Frontend.C99 import c99_ast
    
        %>
         __global__ void ${system_parameters['kernel_name']} ( 
                ${varlist_to_funcproto(array_to_ptr(system_parameters['kernel_parameters']))}
         )
         {
            int __idx__ = blockIdx.x * blockDim.x + threadIdx.x;
            int __idy__ = blockIdx.y * blockDim.y + threadIdx.y;
            int __idz__ = blockIdx.z * blockDim.z + threadIdx.z;
    
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
        kernel_source = Template(cuda_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,
                                                                 loop2_parameters = loop2_parameters,
                                                                 loop3_parameters = loop3_parameters)
    
        kernelProto_source = Template(self.cuda_proto_template).render_unicode(system_parameters = system_parameters,
                                                                 loop_parameters = loop_parameters,)
    
        return [kernel_source, kernelProto_source]
    
    
    
    
    
    def writeCudaKernelLoadFunction(self, for_loop, _system_parameters, _loop_parameters):
             
        ######### Kernel Load Template
        kernelLoad_template = """
        
        <%!
           from Backends.Common.TemplateEngine.Functions import varlist_to_funccall, size_of_var, is_complex_type
           from Backends.Common.TemplateEngine.Functions import varlist_to_funcproto, var_to_declaration, id_to_var_node, write_left_part_of_var_decl
        %>
    
        
        {
            
            int iterations = ${loop_parameters['number_of_iterations']};
            int numThreadsPerBlock = ${system_parameters['num_threads']};
            int numBlocks = ${system_parameters['num_blocks']};
                
            int numElems = numBlocks * numThreadsPerBlock;
            
            dim3 dimGrid;
            dim3 dimBlock;
            
            dimGrid.x = numBlocks;
            dimBlock.x = numThreadsPerBlock;
    
                   printf("*** Launch kernel : < %d, %d> ***", dimGrid.x, dimBlock.x);
            printf("*** Number of iterations : %d", iterations);
    
    
            {
                 tparams * params;
                %for var in system_parameters['kernel_parameters']:
                  params = malloc(sizeof(tparams));
                  params->isValue = var.isValue
                  params->_value = var.value;
                  params->_base_ptr = var.ptr;  // Base address is used to identify the correspondant HostVar structure
                  params->prev = NULL;
                %endfor
                  
                  launch_kernel(${system_parameters['kernel_name'], params);
            }
    
            %endfor
    
    
            
        }
        """      
        for func in _system_parameters['source_mutators']:
            for elem in _system_parameters['kernel_parameters']:
                func(elem)
        kernelLoad_source = Template(kernelLoad_template).render_unicode(system_parameters = _system_parameters,
                                                                 loop_parameters = _loop_parameters)
        return kernelLoad_source
         
  
        
    def cuda_outline_loop_stmts(self,loop_parameters, system_parameters, objetive, 
                                    kernel_name = "tutu", loop2_parameters = None, 
                                    loop3_parameters = None, om = None, ):
        """
            Outline a cuda kernel from a For Loop
            
            :param objetive: statements from a loop (For.stmt)
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
            
        # assert self.is_kernelizable(outlinedFunction, loop_parameters)
        
        kernelFunction = None
        kernelProto = None
        if loop3_parameters and loop2_parameters:
            print " Use kernelize 3d"
            [kernelFunction, kernelProto] = self.kernelize3d_cuda_function(loop_parameters, 
                                                                    system_parameters, 
                                                                    loop2_parameters, 
                                                                    loop3_parameters)
        elif loop2_parameters:
            print " Use kernelize 2d"
            [kernelFunction, kernelProto] = self.kernelize2d_cuda_function(loop_parameters, system_parameters, loop2_parameters)
        else:
            [kernelFunction, kernelProto] = self.kernelize_cuda_function(loop_parameters, system_parameters)
         
        
        kernel_source = Template(self.kernel_template).render_unicode(typedeflist = om.getTypedefList(),
                                                                 funccalllist = called_list,
                                                                 outlinedProto = outlinedProto,
                                                                 kernelProto = kernelProto,
                                                                 kernelFunction = kernelFunction,
                                                                 )
        return kernel_source
    
    def cuda_outline_stmts(self, system_parameters, objetive, 
                                    kernel_name, om ):
        """
            Write a block statement as a CUDA kernel (each thread executes the entire statement)
            
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
            
        # assert self.is_kernelizable(outlinedFunction, None)
        
        # Skip repeated register_vars       
        register_vars = []
        for var in system_parameters['register_vars']:
            if not var in register_vars:
                register_vars.append(var)
        system_parameters['register_vars'] = register_vars
        
        kernelFunction = None
        kernelProto = None
        [kernelFunction, kernelProto] = self.kernelize_cuda_stmts(system_parameters)
         
        
        kernel_source = Template(self.kernel_template).render_unicode(typedeflist = om.getTypedefList(),
                                                                 funccalllist = called_list,
                                                                 outlinedProto = outlinedProto,
                                                                 kernelProto = kernelProto,
                                                                 kernelFunction = kernelFunction,
                                                                 )
        return kernel_source
 
