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

from Backends.C99.Writers.C99Writer import CWriter
from Frontend.FrontendFactory import FrontendFactory
from Frontend.C99.C99InternalRepr import C99AstToIR

import sys
import os
import argparse
import config
import re

class Parser():

	def __init__(self, directory, idirs=''):
		self.directory = directory
		self.idirs = ['.']
		for i in idirs.split(','):
			if i != '':
				self.idirs.append(i)
		self.filessuccess = []
		self.filesfailure = []

	def parse(self, filename):
		try:
			original_source = " ".join(open(filename, 'r').readlines())
			config.CURRENT_LANGUAGES = ['C99', 'GNU', 'OMP']
			#import pdb
			#pdb.set_trace()
			[ast_, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES, self.idirs)
			migrator = C99AstToIR(Writer = CWriter, ast = ast_)
			migrator.annotate()
			new_ast = migrator.ast
			self.filessuccess.append(filename)
		except Exception:
			self.filesfailure.append(filename)
			print 'Error in file: ' + filename
		
	def getResult(self):
		return (self.filessuccess, self.filesfailure,)
		

	def parseSources(self, idirs=''):
		#Para toda la lista de ficheros, parsear 1 por 1
		listfiles = os.listdir(self.directory)
		for source in listfiles:
			if re.search('\.c$', source):
				print 'Parsing source: ' + source
				self.parse(source)


### main ###
parser = argparse.ArgumentParser()
parser.add_argument('-d', "--dir", default=False, dest="directory", action="store", help="Directory to find *.c files")
parser.add_argument('-o', "--out", default=False, dest="fileout", action="store", help="File output")
parser.add_argument('-I', "--lib", default=False, dest="include", action="store", help="Include source directory")

options = parser.parse_args()

if (options.directory):
	print "Parsing all files in directory: " + options.directory
	idirs = ''
	if options.include:
		idirs= options.include
	p = Parser(options.directory, idirs)
	p.parseSources()
	suc, fail = p.getResult()
	if not options.fileout:
		print " "
		print " ------------ Success ------------- "
		for i in suc:
			print 'Source: ' + i
	
		print " ------------ Failure ------------- "
		for i in fail:
			print 'Source: ' + i
		print 'Num success: ' + str(len(suc))
		print 'Num failures: ' + str(len(fail))
	else:
		outf = open(options.fileout, 'w')
		outf.write(" ------------ Success ------------- \n")
		for i in suc:
			outf.write('Source: ' + i + '\n')
		outf.write(" ------------ Failure ------------- \n")
		for i in fail:
			outf.write('Source: ' + i + '\n')
		outf.write('Num success: ' + str(len(suc)) + '\n')
		outf.write('Num failures: ' + str(len(fail)) + '\n')

else:
	print 'Usage: python spck.py --help'
	sys.exit()

