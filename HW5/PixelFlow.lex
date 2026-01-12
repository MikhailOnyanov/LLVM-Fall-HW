%{
/* ============================================
   SECTION 1: C/C++ CODE (Headers & Definitions)
   ============================================ */

// YYSTYPE - type of semantic value returned by the lexer
#define YYSTYPE void*

// Include Bison-generated header (tokens)
#include "PixelFlow.tab.h"

extern "C" int yylex();
%}

%option yylineno
%option noyywrap

%%

"//".*\n            { }
"fun"               { return FUN; }
"for"               { return FOR; }
"in"                { return IN; }
"array"             { return ARRAY; }
"canvas"            { return CANVAS; }
"pixel"             { return PIXEL; }
"show"              { return SHOW; }
"rgb"               { return RGB; }
"rand"              { return RAND; }
"assert"            { return ASSERT; }
"if"                { return IF; }
"else"              { return ELSE; }
[0-9]+              { yylval = strdup(yytext); return NUMBER; }
[A-Za-z_][A-Za-z0-9_]*  { yylval = strdup(yytext); return IDENTIFIER; }
".."                { return DOTDOT; }
"=="                { return TOK_EQ; }
"!="                { return TOK_NE; }
"<="                { return TOK_LE; }
">="                { return TOK_GE; }
[ \t\r\n]+          { }
.                   { return *yytext; }

%%
/* ============================================
   SECTION 3: EXTRA C/C++ CODE
   ============================================ */
