%{
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include "ast.h"

extern int yylex();
void yyerror(const char *msg);

llvm::LLVMContext context;
llvm::Module* module = new llvm::Module("chordProg", context);
CodeGenContext codegen_context(module, context);

ASTNode* root;
%}

%union {
    int integer;
    bool boolean;
    char* string;
    ASTNode* node;
}

%token <integer> ICONST
%token <boolean> BCONST
%token <string> ID

%token CHAR INT FLOAT DOUBLE BOOL IF ELSE WHILE FOR CONTINUE BREAK VOID RETURN
%token ADDOP MULOP DIVOP INCR OROP ANDOP NOTOP EQUOP RELOP
%token LPAREN RPAREN LBRACK RBRACK LBRACE RBRACE SEMI DOT COMMA ASSIGN REFER
%token LARROW RARROW DOUBLECOLON

%type <node> program declarations declaration type names variable expression pointer
%type <node> statements statement if_statement else_part assignable for_statement while_statement tail
%type <node> reference functions function function_call

%start program

%%

program : LARROW declarations statements RARROW functions { $$ = new ProgramNode($2, $3, $5); root = $$; }
        ;

declarations : declarations declaration { $$ = new DeclarationsNode(); $$->declarations.push_back($2); }
             | declaration { $$ = new DeclarationsNode(); $$->declarations.push_back($1); }
             ;

declaration : type names SEMI { $$ = new DeclarationNode($1, $2, nullptr); }
            | type variable ASSIGN expression SEMI { $$ = new DeclarationNode($1, nullptr, nullptr, $4); }
            ;

type : INT { $$ = new TypeNode("int"); }
     | CHAR { $$ = new TypeNode("char"); }
     | FLOAT { $$ = new TypeNode("float"); }
     | DOUBLE { $$ = new TypeNode("double"); }
     | BOOL { $$ = new TypeNode("bool"); }
     | VOID { $$ = new TypeNode("void"); }
     ;

names : variable { $$ = new NamesNode(); $$->names.push_back($1); }
      | names COMMA variable { $$ = $1; $$->names.push_back($3); }
      ;

expression : expression ADDOP expression { $$ = new BinaryOpNode($2, $1, $3); }
           | expression MULOP expression { $$ = new BinaryOpNode($2, $1, $3); }
           | expression DIVOP expression { $$ = new BinaryOpNode($2, $1, $3); }
           | expression INCR { $$ = new UnaryOpNode($2, $1); }
           | INCR expression { $$ = new UnaryOpNode($1, $2); }
           | expression OROP expression { $$ = new BinaryOpNode($2, $1, $3); }
           | expression ANDOP expression { $$ = new BinaryOpNode($2, $1, $3); }
           | NOTOP expression { $$ = new UnaryOpNode($1, $2); }
           | expression EQUOP expression { $$ = new BinaryOpNode($2, $1, $3); }
           | expression RELOP expression { $$ = new BinaryOpNode($2, $1, $3); }
           | LPAREN expression RPAREN { $$ = $2; }
           | variable { $$ = $1; }
           | ICONST { $$ = new ConstantNode($1); }
           | BCONST { $$ = new BooleanConstantNode($1); }
           ;

variable : ID { $$ = new VariableNode($1); }
         | pointer ID { $$ = new PointerVariableNode($2, $1); }
         ;

pointer : pointer MULOP { $$ = new PointerNode($1, $2); }
        | MULOP { $$ = new PointerNode(nullptr, $1); }
        ;

statements : statements statement { $$ = new StatementsNode(); $$->statements.push_back($2); }
           | statement { $$ = new StatementsNode(); $$->statements.push_back($1); }
           ;

statement : if_statement { $$ = $1; }
          | for_statement { $$ = $1; }
          | while_statement { $$ = $1; }
          | assignable { $$ = $1; }
          | CONTINUE SEMI { $$ = new ContinueNode(); }
          | BREAK SEMI { $$ = new BreakNode(); }
          | RETURN SEMI { $$ = new ReturnNode(nullptr); }
          | RETURN expression SEMI { $$ = new ReturnNode($2); }
          ;

if_statement : IF DOUBLECOLON expression tail else_part { $$ = new IfNode($3, $4, $5); }
             ;

else_part : ELSE tail { $$ = $2; }
          | ELSE IF DOUBLECOLON expression tail else_part { $$ = new IfNode($4, $5, $6); }
          | { $$ = nullptr; } /* empty */
          ;

assignable : variable ASSIGN expression SEMI { $$ = new AssignmentNode($1, $3); }
           | function_call { $$ = $1; }
           ;

for_statement : FOR DOUBLECOLON declaration expression SEMI expression tail { $$ = new ForNode($3, $4, $6, $7); }
              ;

while_statement : WHILE DOUBLECOLON expression tail { $$ = new WhileNode($3, $4); }
                ;

tail : statement SEMI { $$ = $1; }
     | LARROW statements RARROW { $$ = $2; }
     ;

reference : REFER { $$ = new ReferenceNode(); }
          | { $$ = nullptr; } /* empty */
          ;

functions : functions function { $$ = new FunctionsNode(); $$->functions.push_back($2); }
          | function { $$ = new FunctionsNode(); $$->functions.push_back($1); }
          ;

function : ID LBRACK RBRACK DOUBLECOLON type LARROW declarations statements RARROW { $$ = new FunctionNode($1, $6, $8, $9); }
         | { $$ = nullptr; } /* empty */
         ;

function_call : ID LBRACK RBRACK SEMI { $$ = new FunctionCallNode($1); }
              ;

%%

void yyerror(const char *msg) {
    fprintf(stderr, "Syntax error: %s\n", msg);
    exit(1);
}

int main(int argc, char *argv[]) {
    int flag;
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error opening file: %s\n", argv[1]);
        exit(1);
    }

    printf("Parsing file: %s\n", argv[1]);
    flag = yyparse();
    fclose(yyin);

    // Generate LLVM IR from the AST
    codegen_context.generateCode(root);

    // Print the generated IR
    module->print(llvm::errs(), nullptr);

    return flag;
}