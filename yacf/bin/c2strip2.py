from Frontend.Shortcuts import getCurrentLanguageAst
c_ast = getCurrentLanguageAst()


from Backends.C99.Writers.OmpWriter import OmpWriter
from Backends.C99.Writers.OmpWriter import CWriter

from sys import argv, exit

import config


def genTs(original_source, filename):
        ###################### First Layer  : File parsing
        # Parse file
        print "Parsing " + filename + " .... ",
        config.CURRENT_LANGUAGES = ['C99','Omp']
        # original_source = " ".join(open(filename, 'r').readlines())
        from Frontend.FrontendFactory import FrontendFactory
        [ast, prepro_class, lexer_class, parser_class] = FrontendFactory().parse_with_language_list(original_source, filename, config.CURRENT_LANGUAGES)
        print " OK "
        print " Migrating to Internal Representation ...."
        from Frontend.C99.C99InternalRepr import C99AstToIR
        # Transform the C99 ast into the intermediate representation
        migrator = C99AstToIR(Writer = OmpWriter, ast = ast)
        print "*************************"
        migrator.annotate()
        new_ast = migrator.ast
        print "*************************"
        print " OK "
        # Get the symbol table
        from Frontend.SymbolTable import *
        print " Build symbol table ...." 
        st = migrator.symbolTable
        print "*************************"
        return [new_ast, st]


output_file = None

tiling_test = """
#define N 1024

int main()
{
    int a[N];
    int i, j, k;

    #pragma llc tiling (256)
    for (i = 0; i < N; i++) {
      a[i] = 0;
    }

    if (k != N)
	printf("** FAIL \\n");
    else
	printf("** Ok \\n");
}
"""

if len(argv) > 1:
    filename  = argv[1]
    if len(argv) > 2 :
        output_file = argv[2]
    original_source = " ".join(open(filename, 'r').readlines())
else:
    filename  = 'tiling_test'
    original_source = tiling_test

[new_ast, st] = genTs(original_source, filename)

#===============================================================================
# print "****> TS for Tiling "
# print str(st)
# print "****> For loop "
# print new_ast.ext[-1].body.block_items[-1].show()
#===============================================================================



# for_loop = new_ast.ext[-1].body.block_items[-2].stmt.loop

from MiddleEnd.Loop.Mutators.LoopStrip2 import LoopStripMiningMutator
from MiddleEnd.Loop.Mutators.LoopTiling import LoopTilingFilter
 

# for_loop = LoopTilingFilter().apply(new_ast).loop 

#===============================================================================
# from Tools.Debug import DotDebugTool
# DotDebugTool(highlight = [for_loop,]).apply(new_ast)
#===============================================================================

# Filter interest loop by llc pragma
tiling_ast = LoopTilingFilter().apply(new_ast)
tiling_loop = tiling_ast.loop
tiling_size = tiling_ast.size

# Apply loopTiling mutatorFunction with a given size as parameter 
new_tiling_ast = LoopStripMiningMutator().mutatorFunction(tiling_loop, tiling_size)

# Also size parameter can be given through __init__ of mutator
# new_tiling_ast = LoopStripMiningMutator(4).mutatorFunction(tiling_ast)


############################################3
# Write file

# Call pretty printer over the file
if output_file:
    v = CWriter(filename = output_file)
    v.visit(new_ast)
    del v  # Ensure file closing
    import os
    if os.system("indent -kr " + output_file) != 0:
        print " You need to install the indent tool to pretty print ouput files "
else:
    new_ast.show(attrnames = True)
    print "************"
    print str(new_ast)

