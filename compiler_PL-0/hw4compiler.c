//author: Ian Harvey
//Date: 7/13/18
//purpose: pl/0 Compiler

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define IDENT_MAX_LENGTH 11
#define NUM_MAX_LENGTH 5
#define MAX_LEX_TABLE_SIZE 1000
#define MAX_CODE_LENGTH 100
#define	MAX_SYMBOL_TABLE_SIZE 100
#define MAX_BUFFER_SIZE 5000
#define MAX_STACK_HEIGHT 2000
#define MAX_LEXI_LEVELS 3

// Declaration of Token Types
typedef enum { nulsym = 1, identsym, numbersym, plussym, minussym, multsym, slashsym, oddsym, eqsym, neqsym, lessym, leqsym, gtrsym,
	geqsym, lparentsym, rparentsym, commasym, semicolonsym, periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym, dosym,
	callsym, constsym, varsym, procsym, writesym, readsym , elsesym
} token_type;

//because enum doesnt work backwards :(
char tokenNames[34][13] = {"", "nulsym", "identsym", "numbersym", "plussym", "minussym", "multsym", "slashsym", "oddsym", "eqsym", "neqsym", "lessym", "leqsym",
"gtrsym", "geqsym", "lparentsym", "rparentsym", "commasym", "semicolonsym", "periodsym", "becomessym", "beginsym", "endsym", "ifsym", "thensym", "whilesym",
"dosym", "callsym", "constsym", "varsym", "procsym", "writesym", "readsym" , "elsesym"};

//for converting
char opcodeNames[23][4] =  { "", "LIT", "RTN", "LOD", "STO", "CAL", "INC", "JMP", "JPC", "SIO", "NEG", "ADD", "SUB", "MUL", "DIV", "ODD", "MOD", "EQL",
"NEQ", "LSS", "LEQ", "GTR", "GEQ"};

typedef struct//for symboltable
    { 
	int kind; 		// const = 1, var = 2, proc = 3
	char name[10];	// name up to 11 chars
	int val; 		// number (ASCII value) 
	int level; 		// L level
	int addr; 		// M address
	int mark;		// to indicate that code has been generated already for a block.
    } symbol; 

typedef struct//for code
	{
	int OP;
	int R;
	int L;
	int M;
	} instruction;

// a few globals...
int symbolTablePosition,pd,cx,R,L,M;//symbol table position, print debug, code postition, register, lexicographical level, value, temp code pointer
int lex_token[MAX_LEX_TABLE_SIZE];//for lex table token values
int stack[MAX_STACK_HEIGHT] = {0};//initilize stack
int RF[16] = {0};//initilize registers
char *token;//for parser
char str[12];//for converting strings to ints.
char buf[MAX_BUFFER_SIZE] = "";//to store lex list
char lex_name[MAX_LEX_TABLE_SIZE][12];//for lex table token names
symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];//for storing constants and variables
instruction code[MAX_CODE_LENGTH];

int isValidNonAlphanumChar(char c){//list of approved non-alphanumeric characters
	if (c == '.' || c == '(' || c == ')' || c == ',' || c == ';' || c == ':' || c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '<' || c == '>') {
		return 1;
	} else 
		return 0;
}

int isValidIdent(char* string){//starts with a letter?
	if(!isalpha(string[0]))
		return 0;
	return 1;
}

int isValidNumber(char* string){//all numbers?
	int len = strlen(string);
	int i,valid = 1;
	for(i = 0; i < len; i++){
		if(!isdigit(string[i]))
			valid = 0;
	}

	return valid;
}

int tokanize(char* string, int len, int lexLevel){
	int failure = 0;

	//make the char array into a string
	string[len] = '\0';
	/*if(pd)
		printf("%s\t%d\t%d\n",string,len,lexLevel);*/

	lex_token[lexLevel] = constsym;
	strcpy(lex_name[lexLevel],string);

	//reserved words
	if(strcmp(string,"const")==0){
		lex_token[lexLevel] = constsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"var")==0){
		lex_token[lexLevel] = varsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"procedure")==0){
		lex_token[lexLevel] = procsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"call")==0){
		lex_token[lexLevel] = callsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"begin")==0){
		lex_token[lexLevel] = beginsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"end")==0){
		lex_token[lexLevel] = endsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"if")==0){
		lex_token[lexLevel] = ifsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"then")==0){
		lex_token[lexLevel] = thensym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"else")==0){
		lex_token[lexLevel] = elsesym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"while")==0){
		lex_token[lexLevel] = whilesym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"do")==0){
		lex_token[lexLevel] = dosym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"read")==0){	
		lex_token[lexLevel] = readsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"write")==0){
		lex_token[lexLevel] = writesym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"odd")==0){
		lex_token[lexLevel] = oddsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"+")==0){
		lex_token[lexLevel] = plussym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"-")==0){
		lex_token[lexLevel] = minussym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"*")==0){
		lex_token[lexLevel] = multsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"/")==0){
		lex_token[lexLevel] = slashsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"=")==0){
		lex_token[lexLevel] = eqsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"<>")==0){
		lex_token[lexLevel] = neqsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"<")==0){
		lex_token[lexLevel] = lessym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"<=")==0){
		lex_token[lexLevel] = leqsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,">")==0){
		lex_token[lexLevel] = gtrsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,">=")==0){
		lex_token[lexLevel] = geqsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,"(")==0){
		lex_token[lexLevel] = lparentsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,")")==0){
		lex_token[lexLevel] = rparentsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,",")==0){
		lex_token[lexLevel] = commasym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,";")==0){
		lex_token[lexLevel] = semicolonsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,".")==0){
		lex_token[lexLevel] = periodsym;
		strcpy(lex_name[lexLevel],string);
	} else if(strcmp(string,":=")==0){
		lex_token[lexLevel] = becomessym;
		strcpy(lex_name[lexLevel],string);
	} else if(isValidIdent(string)){
		if(strlen(string)>IDENT_MAX_LENGTH){
			printf("Error 23: Identifier too long.\n");
			failure = 1;
		}
		lex_token[lexLevel] = identsym;
		strcpy(lex_name[lexLevel],string);		
	} else if(isValidNumber(string)){
		if(strlen(string)>NUM_MAX_LENGTH){
			printf("Error 24: Number too large.\n"); 
			failure = 1;
		}
		lex_token[lexLevel] = numbersym;
		strcpy(lex_name[lexLevel],string);
	} else {
		printf("Error 25: Invalid identifier.\n"); 
		failure = 1;
	}
	return failure;
}

void Expression();//prototype for factor

void getToken(){//loads next token into 'token'
	strcpy(str,"nulsym");
	token = strtok(NULL, "| ");
	if(token == NULL)//to handle crashes with strcmp. when the input string is devored strtok returns NULL so if that happens I just keep on returning "nulsym"
		token = str;
}

void printTokenTrace(){//prints remaining tokens until getToken returns nullsym
	while(strcmp(token,"nulsym")!=0){
		fprintf(stdout," %s",token);
		getToken();
	}
}

int lookup(char* str){//returns index of an identifier in the symbol table, 0 if it is not found, -1 should never happen really
	int i;
	strcpy(symbolTable[0].name,str);
	for(i = symbolTablePosition; i >= 0; i--){
		if(strcmp(str,symbolTable[i].name)==0 && symbolTable[i].mark == 0)
			return i;
	}
	return -1;
}

int mark(int l){//marks all variables and constants for a given lexicographical level
	int i;

	for(i = symbolTablePosition; i >= 0; i--)
		if(symbolTable[i].level == l)
			if(symbolTable[i].kind == 1 || symbolTable[i].kind == 2)
				symbolTable[i].mark = 1;
	return 0;
}

void emit(int op, int r, int l, int m){//emits an assembly instruction that is placed on the code struct and increments the code pointer
	if(cx > MAX_CODE_LENGTH){
		fprintf(stdout,"parsing ERROR 1: maximum assembly code length reached, input file is too long.\n");
		exit(1);
	} 
	
	//sprintf(code[cx], "%d %d %d %d\n",op,r,l,m);
	code[cx].OP = op;
	code[cx].R = r;
	code[cx].L = l;
	code[cx].M = m;
	cx++;
	
}

void Factor(int r){
	int ix;

	if (strcmp(token,"identsym")==0){
		getToken();
		if((ix = lookup(token))==0){
			fprintf(stdout, "parsing ERROR 2: undeclared variable %s.\n",token);
			if(pd) printTokenTrace();
			exit(1);
		}
		if(symbolTable[ix].kind == 2)//if its a var
			emit(3,r,L - symbolTable[ix].level, symbolTable[ix].addr);
		else//if its a const
			emit(1,r,L - symbolTable[ix].level, symbolTable[ix].val);	
		getToken();
	} else if(strcmp(token,"numbersym")==0){//literal
		getToken(); 
		emit(1,r,0,atoi(token));//lit
		getToken();
	} else if(strcmp(token,"lparentsym")==0){
		getToken();
		Expression(r);
		if(strcmp(token,"rparentsym")!= 0){
			fprintf(stdout, "parsing ERROR 3: expected closing parenthesis around expression.\n");
			if(pd) printTokenTrace();
			exit(1);
		}
		getToken();
	} else {
		fprintf(stdout, "parsing ERROR 4: expected number or variable in Expression.\n");
		if(pd) printTokenTrace();
		exit(1);
	}

	return;
}

void Term(int r){
	char mulop[13];
	
	Factor(r);
	while(strcmp(token,"multsym")==0 || strcmp(token,"slashsym")==0){
		strcpy(mulop,token);
		getToken();	
		Factor(r+1);
		if(strcmp(mulop,"multsym")==0){
			emit(13, r, r, r+1);//mult
		} else {
			emit(14, r, r, r+1);//div
		}
	}

	return;
}

void Expression(int r){
	char addop[13];

	if (strcmp(token,"plussym")==0 || strcmp(token,"minussym")==0){
		strcpy(addop,token);
		getToken();
		Term(r);
		if(strcmp(addop, "minussym")==0)
			emit(10, r, 0, 0);//negate
	} else {
		Term(r);
	}
	while(strcmp(token,"plussym")==0 || strcmp(token, "minussym")==0){
		strcpy(addop, token);
		getToken();
		Term(r+1);
		if(strcmp(addop,"plussym")==0){
			emit(11,r,r,r+1);//add
		} else {
			emit(12,r,r,r+1);//sub
		}
	}

	return;
}

void Condition(int r){
	char relop[13];

	if (strcmp(token,"oddsym")==0){
		getToken();
		Expression(r);
		emit(15,r,0,r);
	} else {
		Expression(r);
		if(!(strcmp(token,tokenNames[9])==0 || strcmp(token,tokenNames[10])==0 || strcmp(token,tokenNames[11])==0 || strcmp(token,tokenNames[12])==0 || strcmp(token,tokenNames[13])==0 || strcmp(token,tokenNames[14])==0)){
			fprintf(stdout,"parsing ERROR 5: expected relational symbol in condition statement.\n");
			if(pd) printTokenTrace();
			exit(1);
		}
		strcpy(relop,token);//save op until we have both sides of the condition
		getToken();
		r++;
		Expression(r);
		if(strcmp(relop,"eqsym")==0){
			emit(17,r-1,r-1,r);
		} else if(strcmp(relop,"neqsym")==0){
			emit(18,r-1,r-1,r);
		} else if(strcmp(relop,"lessym")==0){
			emit(19,r-1,r-1,r);
		} else if(strcmp(relop,"leqsym")==0){
			emit(20,r-1,r-1,r);
		} else if(strcmp(relop,"gtrsym")==0){
			emit(21,r-1,r-1,r);
		} else if(strcmp(relop,"geqsym")==0){
			emit(22,r-1,r-1,r);
		}
	}
	return;
}

void Statement(int r){
	int ix, ctemp, ctemp2, ctemp3;

	if (strcmp(token,"identsym")==0){
		getToken();
		ix = lookup(token);//store variable index in ST
		if((ix = lookup(token))==0){//check to see if it exists
			fprintf(stdout, "parsing ERROR 6: undeclared variable %s.\n",token);
			if(pd) printTokenTrace();
			exit(1);
		}
		getToken();
		if(strcmp(token,"becomessym")!=0){
			if(strcmp(token,"eqsym")==0){
				fprintf(stdout,"parsing ERROR 7: expected ':=' instead of '=' following identifier in statement.\n");
				if(pd) printTokenTrace();
				exit(1);
			}
			fprintf(stdout,"parsing ERROR 8: expected ':=' after identifier in statement.\n");
			if(pd) printTokenTrace();
			exit(1);
		}
		getToken();
		Expression(r);
		if(symbolTable[ix].kind != 2){//check to see if it exists
			fprintf(stdout, "parsing ERROR 14: assignment to constant.\n");	
			if(pd) printTokenTrace();
			exit(1);
		}
		emit(4,r,L - symbolTable[ix].level,symbolTable[ix].addr);//store res of expr into var
	} else if (strcmp(token,"callsym")==0){
		getToken();
		if (strcmp(token,"identsym")!= 0){
			fprintf(stdout,"parsing ERROR 15: expected procedure name after call symbol.\n");
			if(pd) printTokenTrace();
			exit(1);			
		}
		getToken();
		ix = lookup(token);
		if((ix = lookup(token))==0){//check to see if it exists
			fprintf(stdout, "parsing ERROR 28: undeclared procedure %s.\n",token);
			if(pd) printTokenTrace();
			exit(1);
		}
		if(symbolTable[ix].kind != 3){//check to see if it is a procedure
			fprintf(stdout, "parsing ERROR 29: %s is not a procedure.\n",token);
			if(pd) printTokenTrace();
			exit(1);
		}
		if(symbolTable[ix].level > L){//check to see if we can even call it
			fprintf(stdout, "parsing ERROR 30: procedure %s is not in the scope.\n",token);
			if(pd) printTokenTrace();
			exit(1);
		}

		emit(5,0,L - symbolTable[ix].level,symbolTable[ix].addr);//call
		getToken();
	} else if (strcmp(token,"beginsym")==0){
		getToken();
		Statement(r);
		while (strcmp(token,"semicolonsym")==0){
			getToken();
			//fprintf(stdout,"%s ",token);
			//fprintf(stdout,"\n");
			Statement(r);
			//fprintf(stdout,"%s ",token);
		}
		if (strcmp(token,"endsym")!=0){
			fprintf(stdout,"parsing ERROR 9: expected 'end' after begin symbol.\n");
			if(pd) printTokenTrace();
			exit(1);
		}
		getToken();
	}else if (strcmp(token,"ifsym")==0){
		getToken();
		Condition(r);
		if (strcmp(token,"thensym") != 0){
			fprintf(stdout,"parsing ERROR 10: expected 'then' following if condition.\n");
			if(pd) printTokenTrace();
			exit(1);
		}
		getToken();
		ctemp = cx;//store where our jump condditional is.
		emit(8,r,0,0);//gen jpc with unfinished location
		Statement(r);
		if (strcmp(token,"elsesym")==0){
			getToken();
			ctemp2 = cx;//store next code place (stuff to do if the condition is false)
			emit(7,0,0,0);//put temp jmp for jumping to the end of the else statements.
			code[ctemp].M = cx;//we now know where the end of the if(TRUE) part is...
			Statement(r);
			code[ctemp2].M = cx;
		} else 	code[ctemp].M = cx;//put addr after if statement to the jpc (no else)
	}else if (strcmp(token, "whilesym")==0){
		ctemp = cx;//store condition place
		getToken();
		Condition(r);
		ctemp2 = cx;//store start of while loop
		emit(8,r,0,0);//while condition jpc
		if(strcmp(token,"dosym")!=0){
			fprintf(stdout, "parsing ERROR 11: expected 'do' following while condition.\n");
			if(pd) printTokenTrace();
			exit(1);
		}
		getToken();
		Statement(r);
		emit(7,0,0,ctemp);//jump at bottom of while
		code[ctemp2].M = cx;//put addr after if statement to the jpc
	}else if (strcmp(token,"readsym")==0){
		getToken();
		if (strcmp(token,"identsym")!=0){
			fprintf(stdout,"parsing ERROR 12: expected identifier following read.\n");
			if(pd) printTokenTrace();
			exit(1);
		}
		getToken();
		ix = lookup(token);
		if((ix = lookup(token))==0){//check to see if it exists
			fprintf(stdout, "parsing ERROR 13: undeclared variable %s.\n",token);
			if(pd) printTokenTrace();
			exit(1);
		}
		if(symbolTable[ix].kind != 2){//check to see if it is a var
			fprintf(stdout, "parsing ERROR 14: assignment to constant.\n");	
			if(pd) printTokenTrace();
			exit(1);
		}
		r++;
		emit(9,r,0,2);//get stdin int
		emit(4,r,L,symbolTable[ix].addr);//store stdin int in var
		r--;
		getToken();
	}else if (strcmp(token, "writesym")==0){
		getToken();
		Expression(r);
		emit(9,r,0,1);	
	} else

	return;
}

void Block(int r){
	char* constName;
	char* identName;
	int constNum;
	int identSym;
	int identAddr;
	int ctemp;
	int numVars=0;
	int l;

	if (strcmp(token,"constsym")==0 || strcmp(token, "varsym")==0){//if we need to allocate space for stack variables
		if (L == 0){
			identAddr = 0;//if we are in the base then we dont need space for FV, SL, DL, and RA.
		} else{
			identAddr = 4;//otherwise we do
		}
	} else{
		identAddr = 3;
	}
	if (strcmp(token,"constsym")==0){
		do {
			symbolTable[symbolTablePosition].kind = 1;//constant
			getToken();
			if (strcmp(token,"identsym")!=0){
				fprintf(stdout, "parsing ERROR 16: expected identifier after constant declaration.\n");
				exit(1);
			}
			getToken();
			strcpy(symbolTable[symbolTablePosition].name,token);
			symbolTable[symbolTablePosition].level = L;
			getToken();
			if (strcmp(token,"eqsym")!=0){
				fprintf(stdout, "parsing ERROR 17: expected '=' after identifier.\n");
				exit(1);
			}
			getToken();
			if (strcmp(token,"numbersym")!=0){
				fprintf(stdout, "parsing ERROR 18: expected integer after equal symbol in constant declaration.\n");
				exit(1);
			}
			getToken();
			symbolTable[symbolTablePosition].val = atoi(token);
			symbolTablePosition++;
			numVars++;
			getToken();
		} while(strcmp(token,"commasym")==0);
		if (strcmp(token,"semicolonsym")!=0){
			fprintf(stdout, "parsing ERROR 19: expeced constant declaration to end with ';'.\n");
			exit(1);
		}
		getToken();
	}
	if (strcmp(token, "varsym")==0){
		do {
			symbolTable[symbolTablePosition].kind = 2;//variable
			getToken();
			if (strcmp(token,"identsym")!= 0){
				fprintf(stdout, "parsing ERROR 20: expected identifier after variable declaration.\n");
				exit(1);
			}
			getToken();
			strcpy(symbolTable[symbolTablePosition].name,token);
			symbolTable[symbolTablePosition].level = L;
			symbolTable[symbolTablePosition].addr = identAddr;
			identAddr++;
			numVars++;
			symbolTablePosition++;
			getToken();
		} while(strcmp(token,"commasym")==0);
		if (strcmp(token,"semicolonsym")!= 0){
			fprintf(stdout, "parsing ERROR 21: expected variable declaration to end with ';'.\n");
			exit(1);
		}
		getToken();
	}
	if (L == 0) emit(6,0,0,identAddr);//allocate space on the stack for vars
	while (strcmp(token,"procsym")==0){
		symbolTable[symbolTablePosition].kind = 3;//procedure
		getToken();
		if (strcmp(token, "identsym")!= 0){
				fprintf(stdout, "parsing ERROR 31: expected identifier after procedure declaration.\n");
				if (pd) printTokenTrace();
				exit(1);			
		}
		getToken();
		strcpy(symbolTable[symbolTablePosition].name,token);
		symbolTable[symbolTablePosition].addr = cx+1;//address of a procedure is the address where the code first starts ie cx.
		symbolTable[symbolTablePosition].level = L;
		identAddr++;
		symbolTablePosition++;
		getToken();
		if (strcmp(token, "semicolonsym")!= 0){
				fprintf(stdout, "parsing ERROR 32: expected ';' after procedure declaration.\n");
				exit(1);			
		}
		getToken();
		L++;
		ctemp = cx;//save code address of jump
		emit(7,0,0,0);//emit jump we dont know where to yet;
		emit(6,0,0,numVars-1);
		Block(r);
		if (strcmp(token, "semicolonsym")!= 0){
				fprintf(stdout, "parsing ERROR 33: expected ';' after block in procedure.\n");
				if(pd) printTokenTrace();
				exit(1);			
		}
		getToken();
		emit(2,0,0,0);//return
		code[ctemp].M = cx;//update address of jump to jump over procedure by default
		mark(L);
		L--;
	}

	Statement(r);

	return;
	
}

void Program(){
	int r = 0;
	L = 0;

	Block(r);

	if (strcmp(token,"periodsym")!=0){
		fprintf(stdout, "parsing ERROR 22: expected '.' to end program.\n");
		if(pd) printTokenTrace();
		exit(1);
	}
	getToken();
	if (strcmp(token,"nulsym")!=0){
		fprintf(stdout, "parsing ERROR 27: tokens after program end.\n");
		if(pd) printTokenTrace();
		exit(1);
	}

	emit(9,0,0,3);//end of program

	return;
}

//to calculate the base of the lexocographical levels of the stack L levels down
int base(int l, int base) {
	int b1; //find base L levels down
	b1 = base;

	while (l > 0) {
		b1 = stack[b1 + 1];
		l--;
	}

	return b1;
}

void printStack(int sp, int bp, int lex){
    int i;
    if (bp == 1) {
    	if (lex > 0) {
			printf("|");
		}
    }	   
    else {
    	//Print the lesser lexical level
    	printStack(bp-1, stack[bp + 2], lex-1);
		printf("|");
    }
    //Print the stack contents - at the current level
    for (i = bp; i <= sp; i++) {
		printf("%3d ", stack[i]);	
    }
}

int main(int argc, char ** argv){
	char c, prev,next;
	int numLine = 1, charCount = 0, isValid = 1,lexLevel = 0,pl = 0, pa = 0, pv = 0,ps = 0, BP = 1, SP = 0, PC = 0, OP, i, halt = 0, lex = 0, cycles = 0,stay = 1;
	char tempStr[IDENT_MAX_LENGTH];
	char IR[12] = "0";
	char opcode[3] = "";
	FILE *ifp;

	if(argc < 2){
		fprintf(stderr, "Input ERROR: No file specified. Valid input specification as follows.\n <Executable> [-l] [-a] [-v] [-d] [-s] <filename.txt>\n");
		exit(1);
	}

	pd = 0;//debug extra printing is intitally off (I made it a global so I can use debug inside other functions without passing it.)
	for (i = 1; i < argc-1; i++){//look at every one of the arguments after the source file name and set printing flags
		if(strcmp(argv[i],"-l")==0) pl = 1;
		if(strcmp(argv[i],"-a")==0) pa = 1;
		if(strcmp(argv[i],"-v")==0) pv = 1;
		if(strcmp(argv[i],"-d")==0) pd = 1;
		if(strcmp(argv[i],"-s")==0) ps = 1;
	}

	/*--------------open the file.--------------*/
	char *fileName = argv[argc - 1];//this assumes that the file is at the end of the compiler directives
	ifp = fopen(fileName, "r");
	if (ifp == NULL) {
		fprintf(stderr, "File ERROR: File \"%s\" not found.\n", fileName);
		exit(1);
	}

	/*-------------see if input file is empty.----------*/
	fseek(ifp, 0, SEEK_END);
	if(ftell(ifp) == 0) {
		fprintf(stderr,"File ERROR: File \"%s\" is empty.\n", fileName);
		exit(1);
	} else {
		rewind(ifp);
	}

	/*-----------print the file to the screen.------------*/
	if (ps){
		fprintf(stdout, "\nSource Program:%s\n", argv[argc -1]); 
		while ((c = fgetc(ifp)) != EOF){
			printf("%c",c);
		}
		printf("\n\n");
		rewind(ifp);
	}

	/*-----------run thru the characters in the file--------*/
	while ((c = fgetc(ifp)) != EOF){

		//for fancier error messages
		if (c == '\n')
			numLine++;

		//skip over commented stuff
		if (prev == '/' && c == '*'){
			while (!(prev == '*' && c =='/')){
				prev = c;
				c = fgetc(ifp);
			}
		}

		if (isalnum(c)) {
			tempStr[charCount] = c;
			charCount++;
		} else if (isspace(c) || isValidNonAlphanumChar(c)) {
			charCount = 0;
		} else {
			isValid = 0;
		}

		prev = c;
	}

	if (isValid == 0) {
		printf("Error 26: Invalid symbol.\n");
		exit(1);
	}

	rewind(ifp);

	/*--------we scanned thru the file and know that the chars are valid so now we break it up into tokens----------*/
	charCount = 0;
	//if (pd) printf("String\tLen\tTable Index\n");
	while (stay){
		//printf("%s\n",tempStr);

		//skip over commented stuff
		if ((next = fgetc(ifp)) != 	EOF){
			if (c == '/' && next == '*'){
				while (!(c == '*' && next =='/')){
					c = next;
					next = fgetc(ifp);
				}
				c = fgetc(ifp);
			} else{
				fseek(ifp, -1, SEEK_CUR);
			}
		}

		if (isalnum(c)) {
			//build the string of alphanum chars
			tempStr[charCount] = c;
			charCount++;
		} else if (isspace(c)){
			if(charCount > 0){
				if(tokanize(tempStr,charCount,lexLevel)) 
					exit(1);
				lexLevel++;
				charCount = 0;
			}
		} else if (isValidNonAlphanumChar(c)){
			if(charCount > 0){
				if(tokanize(tempStr,charCount,lexLevel)) 
					exit(1);
				lexLevel++;
				charCount = 0;
			}

			//look ahead to see if it is a two char symbol
			if ((next = fgetc(ifp)) != 	EOF){
				if((c == ':' && next == '=') || (c == '<' && next == '>') || (c == '<' && next == '=') || (c == '>' && next == '=')){
					tempStr[0] = c;
					tempStr[1] = next;
					if(tokanize(tempStr,2,lexLevel)) 
						exit(1);
					lexLevel++;
				} else { //next isnt part of a two char symbol
					tempStr[0] = c;
					if(tokanize(tempStr,1,lexLevel))
						exit(1);
					lexLevel++;
					fseek(ifp, -1, SEEK_CUR);
				}
			} else {
				tempStr[0] = c;
				if(tokanize(tempStr,1,lexLevel))
					exit(1);
				lexLevel++;
			}
		}
		if((c = fgetc(ifp)) != EOF)
			stay = 2;
		else if (stay == 2){
			stay = 1;
			c = ' ';
		} else
			stay = 0;
	}

	/*------------print out the lexeme table------------
	fprintf(stdout, "\nLexeme Table:\n");
	fprintf(stdout, "lexeme\t\ttoken type\n");
	for(i = 0; i < lexLevel; i++)
		fprintf(stdout, "%s\t\t%d\n", lex_name[i], lex_token[i]);*/

	/*-----------print out the lexeme list--------------*/
	if(pl) fprintf(stdout, "\nLexeme List:\n");
	for(i = 0; i < lexLevel; i++){
		if(pl) fprintf(stdout, "%d", lex_token[i]);
		sprintf(tempStr, "%s", tokenNames[lex_token[i]]);
		strcat(buf,tempStr); 
		// If an identifier, print variable name 
		if(lex_token[i] == 2){
			if(pl) fprintf(stdout, " %s", lex_name[i]);
			sprintf(tempStr, " %s", lex_name[i]); 
			strcat(buf,tempStr);
		}
		// If number, print its ascii number value else 
		if(lex_token[i] == 3){
			if(pl) fprintf(stdout, " %d", atoi(lex_name[i]));
			sprintf(tempStr, " %d", atoi(lex_name[i]));
			strcat(buf,tempStr);
		}
		
		//this is the delimiter (the test cases use a space but i thought we used a pipe or \t)
		if(i != lexLevel - 1){
			if(pl) fprintf(stdout,"|");
			sprintf(tempStr,"|");
			strcat(buf,tempStr);
		}
	}
	if(pl) printf("\n");

	/*------print out symbolic representation------*/
	if(pl){
		fprintf(stdout, "\nSymbolic Representation:\n%s\n\n",buf);
	}


	//close input file we are done with it
	fclose(ifp);

	/*--------call the parser on the tokanized program----------*/
	symbolTablePosition=1;
	cx =0;
	R = 0;
	token = strtok(buf,"| ");//initilize the strtok before we call getToken for the other tokens(1st call to strtok has different parems)
	Program();//start symbol

	fprintf(stdout, "\nNo errors, program is syntatically correct.\n\n");

	if(pd){
		printf("\nSymbol Table:\n");
		for(i = 0; i < symbolTablePosition; i++){
			printf("%-2i Kind: %-2d Name: %-11s Val: %-11d Level: %-2d Adr: %-2d Mark: %-2d\n",i,symbolTable[i].kind,symbolTable[i].name,symbolTable[i].val,symbolTable[i].level,symbolTable[i].addr,symbolTable[i].mark);
		}
		printf("\n");
	}

	if(pa){
		printf("\nGenerated Code:\n");
		for(i = 0; i < cx; i++){
			printf("%d: %s %d %d %d\n",i, opcodeNames[code[i].OP], code[i].R, code[i].L, code[i].M);
		}
		printf("\n");
	}


	if (pv){
		printf("\nVM Execution Trace:");
		printf("\n OP   Rg Lx Vl[ PC BP SP]\n");//header
	}

	//body of the machine
	while(!halt){
		//fetch cycle
		//strcpy(IR,code[PC]);
		OP = code[PC].OP;
		R = code[PC].R;
		L = code[PC].L;
		M = code[PC].M;
		PC++;

		//execute cycle
		//sscanf(IR, "%d %d %d %d", &OP, &R, &L, &M);
		switch (OP){
			case 1://LIT
			RF[R] = M;
			strcpy(opcode,"LIT");
			break;
			case 2://RTN
			SP = BP - 1;
			BP = stack[SP + 3];
			PC = stack[SP + 4];
			lex--;
			strcpy(opcode,"RTN");
			break;
			case 3://LOD
			RF[R] = stack[base(L,BP) + M];
			strcpy(opcode,"LOD");
			break;
			case 4://STO
			stack[base(L,BP) + M] = RF[R];
			strcpy(opcode,"STO");
			break;
			case 5://CAL
			stack[SP + 1] = 0;				//return value
			stack[SP + 2] = base(L,BP);		//static link
			stack[SP + 3] = BP;				//dynamic link
			stack[SP + 4] = PC;				//return address
			BP = SP + 1;
			SP = SP + 4;
			PC = M;
			lex++;
			strcpy(opcode,"CAL");
			break;
			case 6://INC
			SP = SP + M;
			strcpy(opcode,"INC");
			break;
			case 7://JMP
			PC = M;
			strcpy(opcode,"JMP");
			break;
			case 8://JPC
			if (RF[R] == 0)
				PC = M;
			strcpy(opcode,"JPC");
			break;
			case 9://SIO
			switch(M){
				case 1:
				printf("%i\n",RF[R]);
				break;
				case 2:
				scanf("%i",&RF[R]);
				break;
				case 3:
				halt = 1;
				break;
				default:
				fprintf(stderr,"Invalid SIO format, Halting.\n");
				halt = 1;
				break;
			}
			strcpy(opcode,"SIO");
			break;
			case 10://NEG
			RF[R] *= -1;
			strcpy(opcode,"NEG");
			break;
			case 11://ADD
			RF[R] = RF[L] + RF[M];
			strcpy(opcode,"ADD");
			break;
			case 12://SUB
			RF[R] = RF[L] - RF[M];
			strcpy(opcode,"SUB");
			break;
			case 13://MUL
			RF[R] = RF[L] * RF[M];
			strcpy(opcode,"MUL");
			break;
			case 14://DIV
			RF[R] = RF[L] / RF[M];
			strcpy(opcode,"DIV");
			break;
			case 15://ODD
			RF[R] = RF[R] % 2;
			strcpy(opcode,"ODD");
			break;
			case 16://MOD
			RF[R] = RF[L] % RF[M];
			strcpy(opcode,"MOD");
			break;
			case 17://EQL
			RF[R] = RF[L] == RF[M];
			strcpy(opcode,"EQL");
			break;
			case 18://NEQ
			RF[R] = RF[L] != RF[M];
			strcpy(opcode,"NEQ");
			break;
			case 19://LSS
			RF[R] = RF[L] < RF[M];
			strcpy(opcode,"LSS");
			break;
			case 20://LEQ
			RF[R] = RF[L] <= RF[M];
			strcpy(opcode,"LEQ");
			break;
			case 21://GTR
			RF[R] = RF[L] > RF[M];
			strcpy(opcode,"GTR");
			break;
			case 22://GEQ
			RF[R] = RF[L] >= RF[M];
			strcpy(opcode,"GEQ");
			break;
			default:
			fprintf(stderr,"Invalid opcode format, Halting.\n");
			halt = 1;
			break;
		}

		if (lex > MAX_LEXI_LEVELS){
			fprintf(stderr, "Max lexocographical level exceeded, Halting.\n");
			halt = 1;
		}

		//print out what is goin on
		if (pv){
			printf("%-4s%3d%3d%3d[%3d%3d%3d] ", opcode, R, L, M, PC, BP, SP);
			printStack(SP,BP,lex);
			//printf("\n");
			printf("\tRegisters:[%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d]\n", 
				RF[0],RF[1],RF[2],RF[3],RF[4],RF[5],RF[6],RF[7],RF[8],
				RF[9],RF[10],RF[11],RF[12],RF[13],RF[14],RF[15]);
		}

		cycles++;
	}

	return 0;
}