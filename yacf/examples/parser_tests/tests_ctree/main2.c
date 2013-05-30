int main( int argc )
{
int   done=0;

        if ((fp = fopen(file_list[0],"r")) == NULL)
            return(1);
        
        while(yylex(&token)){

            switch(token.tok){

            case IDENT:
                printf("Token %d ('%s') obtained from (%d,%d)\n",
                    token.tok, token.sval,
                    token.line, token.col); 
                free(token.sval);
                break;

            case STRING:
                printf("Token %d ('%s') obtained from (%d,%d)\n",
                    token.tok, token.sval,
                    token.line, token.col); 
                free(token.sval);
                break;

            case RNUM:
                printf("Token %d ('%f') obtained from (%d,%d)\n",
                    token.tok, token.fval,
                    token.line, token.col); 
                break;

            default:
                printf("Token %d (%s) obtained from (%d,%d)\n",
                    token.tok, toksym(token.tok),
                    token.line, token.col); 
                break;
            }
        }

        fclose(fp);

        return(0);
	/* input, output, dribble, and emulate have char strings */
	/* file_list has all '.c' files */

	if (dribble && ((dupfp = fopen(dribble,"r")) == NULL)){
            fprintf(stderr,"Could not open file '%s' for dribble.\n",dribble);
            exit(1);
        }
}

/*  ###############################################################  */
