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

import unittest
import config
from Frontend.FrontendFactory import FrontendFactory



from MiddleEnd.Functions.Mutators.Outliner import compound_has_got_pointers

config.CURRENT_LANGUAGES = ['C99',]

class TestMiddleEndFunctions(unittest.TestCase):
    def test_compound_has_got_pointers(self):
        
        original_source = """ 
                int main () {
                    int a; 
                    int* p; 
                    int list [10]; 
                    typedef struct {
                        int x;
                        int y;
                    } point;
                    point start;
    
                    a *= -4; 
                    a = *p; 
                    a = list [0];
                    start->x = 7;
                    *p = 3;
                    return 0;
                }

        """
        [ast, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, 'test', config.CURRENT_LANGUAGES)
        items = ast.ext[0].body.block_items
          
        # Declarations
        ##############################
          
        #int a;          
        self.assertEqual(False, compound_has_got_pointers(items[0]))
         
        #int* p; 
        self.assertEqual(True, compound_has_got_pointers(items[1]))
          
        #int list [10]; 
        self.assertEqual(False, compound_has_got_pointers(items[2]))
          
        #typedef struct {
        #    int x;
        #    int y;
        #} point;
        self.assertEqual(False, compound_has_got_pointers(items[3]))
          
        #point start;
        self.assertEqual(False, compound_has_got_pointers(items[4]))
         
          
        # Statements
        ###############################
          
        #a *= -4; 
        self.assertEqual(False, compound_has_got_pointers(items[5]))
         
        #a = *p; 
        self.assertEqual(True, compound_has_got_pointers(items[6]))
         
        #a = list [0];
        self.assertEqual(False, compound_has_got_pointers(items[7]))
          
        #start->x = 7;
        self.assertEqual(False, compound_has_got_pointers(items[8]))
          
        #*p = 3;
        self.assertEqual(True, compound_has_got_pointers(items[9]))
         
        #return 0;
        self.assertEqual(False, compound_has_got_pointers(items[10]))
