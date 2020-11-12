// Brian Koh Lit Yang
// a1782291

#include "tokeniser-extras.h"

// to shorten the code
using namespace std ;

/////////////////////////////////////////////////////////////////////////

namespace Assignment_Tokeniser
{

    static TokenKind new_token_kind ;

    // parse one extra character so that ch is the start of the next token
    // we have already parsed all the characters that make up the current token
    // we just need to remember the token kind
    static void parse_extra(TokenKind kind)
    {
        new_token_kind = kind;
        nextch();
    }

    // parse a single character of white space
    // wspace ::= '\n' | ' '
    // ch is the white space character
    static void parse_wspace(TokenKind kind)
    {
        parse_extra(kind);
    }

    //parsing tabs 
    static void parse_tab()
    {
        new_token_kind = tk_space ;
        nextch();
    }

    // parse an identifier - always read one extra character
    // identifier ::= ('a'-'z'|'A'-'Z')('a'-'z'|'A'-'Z'|'0'-'9')*
    // ch is the first character of the identifier
    static void parse_identifier()
    {
        new_token_kind = tk_identifier;

        do nextch() ; while ( c_have(cg_extends_identifier) ) ;
        //c_did_not_find(cg_start_of_identifier) ;
    }

    // handles the leading '0's of integers based on requirements
    static void parse_zeros(){

        new_token_kind = tk_integer;
        nextch();

        if(c_have('.'))
        {
            c_mustbe('.');
            while(c_have(cg_digit))
            {
                c_mustbe(cg_digit);
                c_mustbe(cg_start_of_exponent);
            }
        }
    }

    // parse a number - always read one extra character
    // ('0'-'9')('0'-'9')*
    // ch is the first digit of the number
    static void parse_integer()
    {
        // we are parsing an integer
        new_token_kind = tk_integer ;
        // read characters until we read past the end of the integer
        do nextch() ; while ( c_have(cg_digit) ) ;

        if(ch == '.')
        {
            c_mustbe('.');
            while(c_have(cg_digit))
            {
                c_mustbe(cg_digit);
            }
        }
    }
    // parse scientific numbers
    static void parse_scientific()
    {
        c_mustbe(cg_start_of_exponent);
        c_have_next(cg_sign);

        if(!c_have_next('0'))
        {
            c_mustbe(cg_digit19);
            c_have_next(cg_digit);
            c_have_next(cg_digit);
        }
    }

    // handling individual symbols
    // * symbol ::= '@'|'-='|'+='|'*='|'/='|'!='|'=='|'<<<'|'<<'|'>>>'|'>>'|'{'|'}'|'('|')'|'['|']'
    //   - symbols each have their own TokenKind
    static void parse_symbols(TokenKind kind){
        new_token_kind = kind;
        nextch();
    }

    static void parse_tk_lcb(){ // '{'

        new_token_kind = tk_lcb;
        nextch();
    }

    static void parse_tk_rcb(){ // '}'

        new_token_kind = tk_rcb;
        nextch();
    }

    static void parse_tk_lrb(){ // '('

        new_token_kind = tk_lrb;
        nextch();
    }

    static void parse_tk_rrb(){ // ')'

        new_token_kind = tk_rrb;
        nextch();
    }

    static void parse_tk_lsb(){ // '['

        new_token_kind = tk_lsb;
        nextch();
    }

    static void parse_tk_rsb(){ // ']'

        new_token_kind = tk_rsb;
        nextch();
    }
    
    // handling the error for '!=' tokens
    // checks if '=' is infront of '!'
    // returns the appropriate error message based on input
    static void parse_tk_not_eq(){

        // is ch the given character or does it belong to the given group of characters?
        // eg if ch is '0' c_have(cg_integer) returns true
        // eg if ch is '=' c_have('=') returns true
        c_have(cg_extends_identifier);
        // is ch expected, if it is also call nextch()?
        c_have_next('!');
        new_token_kind = tk_not_eq;
        // ch must be expected, if it is call nextch(),
        // otherwise call c_did_not_find(expected)?
        c_mustbe('=');
    }

    // handling the error for '==' tokens
    // checks if '=' is infront of '='
    // returns the appropriate error message based on input
    static void parse_tk_eq(){

        c_have(cg_extends_identifier);
        
        c_have_next('=') ;
        new_token_kind = tk_eq;
       
        c_mustbe('=');
    }

    // handling the error for '-=' tokens
    // checks if '=' is infront of '-'
    static void parse_tk_sub_assign(){
        c_have(cg_extends_identifier);
        
        c_have_next('-') ;
        new_token_kind = tk_sub_assign;
  
        c_mustbe('=');
    }

    // handling the error for '+=' tokens
    // checks if '=' is infront of '+'
    static void parse_tk_add_assign(){
        c_have(cg_extends_identifier);
        
        c_have_next('+') ;
        new_token_kind = tk_add_assign;
  
        c_mustbe('=');
    }
    // handling the error for '*=' tokens
    // checks if '=' is infront of '*'
    static void parse_tk_mult_assign(){
        c_have(cg_extends_identifier);
        
        c_have_next('*') ;
        new_token_kind = tk_mult_assign;
  
        c_mustbe('=');
    }

    static void parse_tk_div_assign(){
        c_have(cg_extends_identifier);
        
        c_mustbe('/') ;
        
        switch (ch){
            case '=': // case div_Assign
                new_token_kind = tk_div_assign;  
                c_mustbe('=');
                break;

            case '/': // case eol_comment
                new_token_kind = tk_eol_comment;
                c_mustbe('/');

                while (c_have(cg_eol_comment_char))
                    nextch();
                    c_mustbe('\n');
                    break;

            case '*': // case adhoc_comment
                new_token_kind = tk_adhoc_comment;
                c_mustbe('*');

                while (c_have(cg_adhoc_comment_char)){
                    nextch();

                    if (c_have_next('*')){
                        if (c_have('/')){
                            break;
                        }
                    }
                }
                c_mustbe('/');
        }
    }

    static void parse_tk_left_shift(){

        c_mustbe('<');
        c_mustbe('<');

        if (c_have_next('<')){
            new_token_kind = tk_lshift_l;
        } else {
            new_token_kind = tk_lshift;
        }
    }

    static void parse_tk_right_shift(){

        c_mustbe('>');
        c_mustbe('>');

        if (c_have_next('>')){
            new_token_kind = tk_rshift_l;
        } else {
            new_token_kind = tk_rshift;
        }
    }

    // return the next Token object by reading more of the input
    // you must read input using the nextch() function
    // the last character read is in the static variable ch
    // always read one character past the end of the token being returned
    Token next_token()
    {
        switch(ch)                      // ch is always the next char to read
        {
            case ' ':
                parse_wspace(tk_space);
                break;

            case '\n':
                parse_wspace(tk_newline);
                break;

            case 'a'...'z':             // identifier tokens
            case 'A'...'Z':
            case '$':
                parse_identifier();
                break;

            case '0':
                parse_zeros();
                break;

            case '1'...'9':             // integer tokens 
                parse_integer() ;
                break;

                    // symbol tokens
            case '@':
                parse_symbols(tk_at);
                break;

            case '.':
                parse_symbols(tk_stop);
                break;

            case '{':
                parse_symbols(tk_lcb);
                break;

            case '}':
                parse_symbols(tk_rcb);
                break;

            case '(':
                parse_symbols(tk_lrb);
                break;

            case ')':
                parse_symbols(tk_rrb);
                break;

            case '[':
                parse_symbols(tk_lsb);
                break;

            case ']':
                parse_symbols(tk_rsb);
                break;

                    // operators
            case '-':
                parse_tk_sub_assign();
                break;

            case '+':
                parse_tk_add_assign();
                break;

            case '*':
                parse_tk_mult_assign();
                break;

            case '/':
                parse_tk_div_assign();
                break;

            case '!':
                parse_tk_not_eq();
                break;

            case '=':
                parse_tk_eq();
                break;
            
            case '<':
                parse_tk_left_shift();
                break;

            case '>':
                parse_tk_right_shift();
                break;

        // End of Inptut
        case EOF:
            new_token_kind = tk_eoi ;
            break ;

        default:
            c_did_not_find(cg_start_of_token) ;
        }

        Token token = new_token(new_token_kind) ;

        // keyword tokens
        // if the token_kind is an identifier
        // retrieve the spelling of the token and check whether it is a keyword or identifier
        // then set and return the token to the correct keyword
        if (new_token_kind == tk_identifier){

            string spel = token_spelling(token);

            TokenKind tk = keyword_or_identifier(spel);

            if (tk == tk_done){
                set_token_kind(token, tk_done);
            } 
            else if (tk == tk_while){
                set_token_kind(token, tk_while);
            } 
            else if (tk == tk_procedure){
                set_token_kind(token, tk_procedure);
            } 
            else if (tk == tk_if_goto){
                set_token_kind(token, tk_if_goto);
            } 
            else if (tk == tk_this) {
                set_token_kind(token, tk_this);
            }
        }
        // adhoc and eol comments
        //if the token is found we have to delete the leading eol and adhoc comments
        // and also deleting \n at the end using substr
        else if (new_token_kind == tk_eol_comment)
        {
            string temp_string = token_spelling(token);
            set_token_spelling(token, temp_string.substr(2,temp_string.length()-3));
        }
        else if (new_token_kind == tk_adhoc_comment)
        {
            string temp_string = token_spelling(token);
            set_token_spelling(token, temp_string.substr(2,temp_string.length()-4));
        }
        return token;
    }
}
