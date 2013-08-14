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

import subprocess
import re

from Tools.SourceStorage import FileType

class OpenCl_FileType(FileType):
    """ A file type to store OpenCl source code files
    """
    def __init__(self):
        FileType.__init__(self, 'OpenCl', '.cl')
            
    def pretty_print(self, text):
        """ Return a pretty representation of the file 
        
            :param text: The original text to be pretty printed
        """
        p = subprocess.Popen("indent -kr -", shell=True, bufsize=1, stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
        # Send / Retrieve string to the preprocessor
        return p.communicate(text)[0]
    
    def _removeComments(self, string):
        """
            Delete the comments of a given string 
            
            :param string: The original text
        """
        def replacer(match):
            s = match.group(0)
            if s.startswith('/'):
                return ""
            else:
                return s
        pattern = re.compile(
            r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"',
            re.DOTALL | re.MULTILINE
        )
        return re.sub(pattern, replacer, string)

    def importMacros(self, line_list):
        """ Get the preprocessor macros from a given string
                and store it on the file.


                Currently supported macros are::

                    #include <[A-Za-Z0-9.]+>
                    #include "[A-Za-Z0-9.]+"
            
            :parameter line_list: The original text
        """      
        identifier = "[A-Za-z0-9.-_]+"
        define = '#define ' + identifier
        ifdef = '#if[n]*def ' + identifier
        endif = '#endif'
        _else = '#else'
        include = '#include "' + identifier + '"'
        include_sys = '#include <' + identifier + '>'
        macro_re = re.compile("|".join([include, include_sys]))
        macros = macro_re.findall(self._removeComments(''.join(line_list)))
        return "\n".join(macros)
        # self._files[name + file_type.extension] += "\n".join(macros)
    
