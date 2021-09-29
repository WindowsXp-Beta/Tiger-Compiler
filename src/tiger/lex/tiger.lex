%filenames = "scanner"

 /*
  * Please don't modify the lines above.
  */

 /* You can add lex definitions here. */
 /* TODO: Put your lab2 code here */

%x COMMENT STR IGNORE
/* escape sequence */
escape_seq \\([nt\"\\]|"[:ctrl:]c"|[0-9]{3})
neglect_seq \\[:blank:]+\\

%%

 /*
  * Below is examples, which you can wipe out
  * and write regular expressions and actions of your own.
  *
  * All the tokens:
  *   Parser::ID
  *   Parser::STRING
  *   Parser::INT
  *   Parser::COMMA ,
  *   Parser::COLON :
  *   Parser::SEMICOLON ;
  *   Parser::LPAREN (
  *   Parser::RPAREN )
  *   Parser::LBRACK [
  *   Parser::RBRACK ]
  *   Parser::LBRACE {
  *   Parser::RBRACE }
  *   Parser::DOT .
  *   Parser::PLUS +
  *   Parser::MINUS -
  *   Parser::TIMES *
  *   Parser::DIVIDE /
  *   Parser::EQ =
  *   Parser::NEQ <>
  *   Parser::LT <
  *   Parser::LE <=
  *   Parser::GT >
  *   Parser::GE >=
  *   Parser::AND &
  *   Parser::OR |
  *   Parser::ASSIGN :=
  *   Parser::ARRAY
  *   Parser::IF
  *   Parser::THEN
  *   Parser::ELSE
  *   Parser::WHILE
  *   Parser::FOR
  *   Parser::TO
  *   Parser::DO
  *   Parser::LET
  *   Parser::IN
  *   Parser::END
  *   Parser::OF
  *   Parser::BREAK
  *   Parser::NIL
  *   Parser::FUNCTION
  *   Parser::VAR
  *   Parser::TYPE
  */

 /* reserved words */
"while" {adjust(); return Parser::WHILE;}
"for" {adjust(); return Parser::FOR;}
"to" {adjust(); return Parser::TO;}
"break" {adjust(); return Parser::BREAK;}
"let" {adjust(); return Parser::LET;}
"in" {adjust(); return Parser::IN;}
"end" {adjust(); return Parser::END;}
"function" {adjust(); return Parser::FUNCTION;}
"var" {adjust(); return Parser::VAR;}
"type" {adjust(); return Parser::TYPE;}
"array" {adjust(); return Parser::ARRAY;}
"if" {adjust(); return Parser::IF;}
"then" {adjust(); return Parser::THEN;}
"else" {adjust(); return Parser::ELSE;}
"do" {adjust(); return Parser::DO;}
"of" {adjust(); return Parser::OF;}
"nil" {adjust(); return Parser::NIL;}
, {adjust(); return Parser::COMMA;}
: {adjust(); return Parser::COLON;}
; {adjust(); return Parser::SEMICOLON;}
( {adjust(); return Parser::LPAREN;}
) {adjust(); return Parser::RPAREN;}
[ {adjust(); return Parser::LBRACK;}
] {adjust(); return Parser::RBRACK;}
{ {adjust(); return Parser::LBRACE;}
} {adjust(); return Parser::RBRACE;}
R"." {adjust(); return Parser::DOT;}
+ {adjust(); return Parser::PLUS;}
- {adjust(); return Parser::MINUS;}
* {adjust(); return Parser::TIMES;}
/ {adjust(); return Parser::DIVIDE;}
= {adjust(); return Parser::EQ;}
"<>" {adjust(); return Parser::NEQ;}
< {adjust(); return Parser::LT;}
"<=" {adjust(); return Parser::LE;}
> {adjust(); return Parser::GT;}
">=" {adjust(); return Parser::GE;}
& {adjust(); return Parser::AND;}
| {adjust(); return Parser::OR;}
":=" {adjust(); return Parser::ASSIGN;}
"int" {adjust(); return Parser::ID;}
"string" {adjust(); return Parser::ID;}
[:alpha:]([:alnum:]|_)* {adjust(); return Parser::ID;}
\"([:print:]|[:space:]|{escape_seq}|{neglect_seq})*\" {adjust(); setMatched(matched()); return Parser:STRING;}


 /*
  * skip white space chars.
  * space, tabs and LF
  */
[ \t]+ {adjust();}
\n {adjust(); errormsg_->Newline();}

 /* illegal input */
. {adjust(); errormsg_->Error(errormsg_->tok_pos_, "illegal token");}
