%{
#include <stdio.h>
#include <iostream>

using namespace std;

#include "../inc/parser.hpp"


%}

%option noyywrap

digit [0-9]
letter [a-zA-Z_]
hex 0[xX][0-9a-fA-F]+
slash ['\\\']


%%

".global" { return GLOBAL; }
".extern" { return EXTERN; }
".section" { return SECTION; }
".word" { return WORD; }
".skip" { return SKIP; }
".ascii" { return ASCII; }
".equ" { return EQU; }
".end" { return END; }
"halt" { return HALT; }
"int" { return INT; }
"iret" { return IRET; }
"call" { return CALL; }
"ret" { return RET; }
"jmp" { return JMP; }
"jeq" { return JEQ; }
"jne" { return JNE; }
"jgt" { return JGT; }
"push" { return PUSH; }
"pop" { return POP; }
"xchg" { return XCHG; }
"add" { return ADD; }
"sub" { return SUB; } 
"mul" { return MUL; }
"div" { return DIV; }
"cmp" { return CMP; }
"not" { return NOT; }
"or"  { return OR; }
"and" { return AND; }
"xor" { return XOR; }
"test" { return TEST; }
"shr"  { return SHR; }
"shl"  { return SHL; }
"ldr" { return LDR; }
"str" { return STR; }
(r[0-7]) { yylval.number = atoi(yytext + 1); return REG; }
"sp" { yylval.number = 6; return REG; }
"pc" { yylval.number = 7; return REG; }
"psw" { yylval.number = 8; return REG; }
"+" { return PLUS; }
"-" { return MINUS; }
"*" { return STAR; }
"/" { return DIVISION; }
"$" { return DOLLAR; }
"%" { return PERCENT; }
"," { return COMMA; }
":" { return COLON; }
"[" { return LBRACKET; }
"]" { return RBRACKET; }
"(" { return LPAREN; }
")" { return RPAREN;  }
["] { return QUOTE; }

\n { return NEWL; }

{slash} { return SLASH; }
{letter}({letter}|{digit})* { yylval.ident = strdup(yytext); return IDENT; }
{digit}+  {  yylval.number = atoi(yytext); return NUMBER; }
{hex}	{ yylval.number = stoi(yytext, nullptr, 16); return NUMBER; }

#.*\n { }
[ \t \r \f] { }

. { printf("Unknown character [%c]\n",yytext[0]); }

%%