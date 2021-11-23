%filenames = "scanner"

 /*
  * Please don't modify the lines above.
  */

 /* You can add lex definitions here. */
 /* TODO: Put your lab2 code here */


%x COMMENT STR IGNORE ESCAPE CONTROL
/* escape sequence */
escape_seq \\([nt\"\\]|"[[:ctrl:]]c"|[0-9]{3})
neglect_seq \\[[:blank:]]+\\

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
"(" {adjust(); return Parser::LPAREN;}
")" {adjust(); return Parser::RPAREN;}
"[" {adjust(); return Parser::LBRACK;}
"]" {adjust(); return Parser::RBRACK;}
"{" {adjust(); return Parser::LBRACE;}
"}" {adjust(); return Parser::RBRACE;}
"." {adjust(); return Parser::DOT;}
"+" {adjust(); return Parser::PLUS;}
"-" {adjust(); return Parser::MINUS;}
"*" {adjust(); return Parser::TIMES;}
"/" {adjust(); return Parser::DIVIDE;}
"=" {adjust(); return Parser::EQ;}
"<>" {adjust(); return Parser::NEQ;}
"<" {adjust(); return Parser::LT;}
"<=" {adjust(); return Parser::LE;}
">" {adjust(); return Parser::GT;}
">=" {adjust(); return Parser::GE;}
"&" {adjust(); return Parser::AND;}
"|" {adjust(); return Parser::OR;}
":=" {adjust(); return Parser::ASSIGN;}
[[:digit:]]+ {adjust(); return Parser::INT;}
"int" {adjust(); return Parser::ID;}
"string" {adjust(); return Parser::ID;}
[[:alpha:]]([[:alnum:]]|_)* {adjust(); return Parser::ID;}
\" {
  adjust();
  begin(StartCondition__::STR);
}
<STR> {
  (([[:print:]]{-}[\"\\])|[[:space:]])* {
    adjustStr();
    string_var += matched();
  }
  \\ {
    adjustStr();
    begin(StartCondition__::ESCAPE);
  }
  \" {
    adjustStr();
    begin(StartCondition__::INITIAL);
    setMatched(string_var);
    string_var.clear();
    return Parser::STRING;
  }
}
<ESCAPE> {
    n {
      adjustStr();
      string_var += '\n';
      begin(StartCondition__::STR);
    }
    t {
      adjustStr();
      string_var += '\t';
      begin(StartCondition__::STR);
    }
    [0-9]{3} {
      adjustStr();
      string_var += (char)atoi(matched().data());
      begin(StartCondition__::STR);
    }
    \" {
      adjustStr();
      string_var += '\"';
      begin(StartCondition__::STR);
    }
    \\ {
      adjustStr();
      string_var += '\\';
      begin(StartCondition__::STR);
    }
    "^" {
      adjustStr();
      begin(StartCondition__::CONTROL);
    }
    [[:blank:]]{+}[\n] {
      adjustStr();
      begin(StartCondition__::IGNORE);
    }
}
<CONTROL> {
  A {
    string_var += (char)1;
    adjustStr();
    begin(StartCondition__::STR);
  }
  C {
    string_var += (char)3;
    adjustStr();
    begin(StartCondition__::STR);
  }
  O {
    string_var += (char)15;
    adjustStr();
    begin(StartCondition__::STR);
  }
  M {
    string_var += (char)13;
    adjustStr();
    begin(StartCondition__::STR);
  }
  P {
    string_var += (char)16;
    adjustStr();
    begin(StartCondition__::STR);
  }
  I {
    string_var += (char)9;
    adjustStr();
    begin(StartCondition__::STR);
  }
  L {
    string_var += (char)12;
    adjustStr();
    begin(StartCondition__::STR);
  }
  E {
    string_var += (char)5;
    adjustStr();
    begin(StartCondition__::STR);
  }
  R {
    string_var += (char)18;
    adjustStr();
    begin(StartCondition__::STR);
  }
  S {
    string_var += (char)19;
    adjustStr();
    begin(StartCondition__::STR);
  }
}
<IGNORE> {
  ([[:blank:]]{+}[\n])* {
    adjustStr();
  }
  \\ {
    adjustStr();
    begin(StartCondition__::STR);
  }
}
R"(/*)" {
  adjust();
  begin(StartCondition__::COMMENT);
  annotation_count++;
}
<COMMENT> {
  R"(/*)" {
    adjust();
    annotation_count++;
  }
  R"(*/)" {
    adjust();
    annotation_count--;
    if(annotation_count == 0) {
      begin(StartCondition__::INITIAL);
    }
  }
  . |
  \n {
    adjust();
  }
}


 /*
  * skip white space chars.
  * space, tabs and LF
  */
[ \t]+ {adjust();}
\n {adjust(); errormsg_->Newline();}

 /* illegal input */
<*>. {
  adjust();
  std::cout << "wrong message: " << matched() << std::endl;
  errormsg_->Error(errormsg_->tok_pos_, "illegal token");
}
