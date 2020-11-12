// Brian Koh Lit Yang
// a1782291
#include <string>
#include <vector>
#include "iobuffer.h"
#include "symbols.h"
#include "abstract-syntax-tree.h"

// to shorten our code:
using namespace std;
using namespace CS_IO_Buffers;
using namespace CS_Symbol_Tables;
using namespace Jack_Compiler;

// ***** WHAT TO DO *****
//
// MODIFY the skeleton code below to parse a Jack class and construct the equivalent abstract syntax tree, ast.
//        The main function will print the ast formatted as XML
//
// NOTE: the correct segment and offset is recorded with every variable in the ast
//       so the code generator does not need to use any symbol tables
//
// NOTE: use the fatal_token_context() function to print fatal error messages with the current
//       input file position marked.
//
// The skeleton code has one function per non-terminal in the Jack grammar
//  - comments describing how to create relevant ast nodes prefix most functions
//  - ast nodes are immutable - you must create all the sub-trees before creating a new node
//
// The structure of the skeleton code is just a suggestion
//  - it matches the grammar but does not quite match the ast structure
//  - you will need to change the parameters / results / functions to suit your own logic
//  - you can change it as much as you like
//
// NOTE: the best place to start is to write code to parse the input without attempting to create
//       any ast nodes. If correct this parser will report errors when it finds syntax errors but
//       correct programs will fail because the tree printer will not be provided with a valid ast.
//       Once the parsing works modify your code to create the ast nodes.
//
// NOTE: the return create_empty() ; statements are only present so that this skeleton code will compile.
//       these statements need to be replaced by code that creates the correct ast nodes.
//

// The Jack grammar to be recognised:
// program          ::= One or more classes, each class in a separate file named <class_name>'.Jack'
// class            ::= 'class' identifier '{' class_var_decs subr_decs '}'
// class_var_decs   ::= (static_var_dec | field_var_dec)*
// static_var_dec   ::= 'static' type identifier (',' identifier)* ';'
// field_var_dec    ::= 'field' type identifier (',' identifier)* ';'
// type             ::= 'int' | 'char' | 'boolean' | identifier
// vtype            ::= 'void' | type
// subr_decs        ::= (constructor | function | method)*
// constructor      ::= 'constructor' identifier identifier '(' param_list ')' subr_body
// function         ::= 'function' vtype identifier '(' param_list ')' subr_body
// method           ::= 'method' vtype identifier '(' param_list ')' subr_body
// param_list       ::= ((type identifier) (',' type identifier)*)?
// subr_body        ::= '{' var_decs statements '}'
// var_decs         ::= var_dec*
// var_dec          ::= 'var' type identifier (',' identifier)* ';'

// statements       ::= statement*
// statement        ::= let | if | while | do | return
// let              ::= 'let' identifier index? '=' expr ';'
// if               ::= 'if' '(' expr ')' '{' statements '}' ('else' '{' statements '}')?
// while            ::= 'while' '(' expr ')' '{' statements '}'
// do               ::= 'do' identifier (id_call | call) ';'
// return           ::= 'return' expr? ';'

// expr             ::= term (infix_op term)*
// term             ::= integer_constant | string_constant | 'true' | 'false' | 'null' | 'this' | '(' expr ')' | unary_op term | var_term
// var_term         ::= identifier (index | id_call | call)?
// id_call          ::= '.' identifier call
// call             ::= '(' expr_list ')'
// expr_list        ::= (expr (',' expr)*)?
// infix_op         ::= '+' | '-' | '*' | '/' | '&' | '|' | '<' | '>' | '='
// unary_op         ::= '-' | '~'

// forward declarations of one function per non-terminal rule in the Jack grammar - except for program
// NOTE: parse_type(), parse_vtype(), parse_unary_op() all return a Token not an ast
//
ast parse_class();
ast parse_class_var_decs();
ast parse_static_var_dec();
ast parse_field_var_dec();
Token parse_type();
Token parse_vtype();
ast parse_subr_decs();
ast parse_constructor();
ast parse_function();
ast parse_method();
ast parse_param_list();
ast parse_subr_body();
ast parse_var_decs();
ast parse_var_dec();

ast parse_statements();
ast parse_statement();
ast parse_let();
ast parse_if();
ast parse_while();
ast parse_do();
ast parse_return();

ast parse_expr();
ast parse_term();
ast parse_var_term();
ast parse_index();
ast parse_id_call();
ast parse_call();
ast parse_expr_list();
ast parse_infix_op();
Token parse_unary_op();

// global variables
// holds the count for static offsets
static int static_offset = 0;

// holds the count for this offsets
static int field_offset = 0;

// holds the count for local offsets
static int local_offset = 0;

// holds the count for arg offsets
static int arg_offset = 0;

// holds the current classname
static string curr_classname = "";

// symbols help with recording a mapping from a string key to a value
// symbol table for variables
symbols var_table;

// symbol table for function variables
symbols func_table;

// class ::= 'class' identifier '{' class_var_decs subr_decs '}'
// create_class(myclassname,class_var_decs,class_subrs)
ast parse_class()
{
    push_error_context("parse_class()");

    string className;
    ast var_decs = nullptr;
    ast subr_decs = nullptr;

    // retrieve class token and assign name to current classname in global variable
    mustbe(tk_class);
    string name = token_spelling(mustbe(tk_identifier));
    curr_classname = name;
    mustbe(tk_lcb);

    var_decs = parse_class_var_decs();
    subr_decs = parse_subr_decs();

    mustbe(tk_rcb);

    pop_error_context();
    return create_class(name, var_decs, subr_decs);
}

// class_var_decs ::= (static_var_dec | field_var_dec)*
// returns ast_class_var_decs: create_class_var_decs(vector<ast> decs)
// create_class_var_decs must be passed a vector of ast_var_dec
//
ast parse_class_var_decs()
{
    push_error_context("parse_class_var_decs()");
    vector<ast> decs;

    // if have static_var_dec or field_var_dec
    while (have(tk_static) || have(tk_field))
    {
        switch (token_kind())
        {
        case tk_field:
            decs.push_back(parse_field_var_dec());
            break;

        case tk_static:
            decs.push_back(parse_static_var_dec());
            break;

        default:
            did_not_find(tg_starts_class_var);
            fatal_token_context("did not find tk_static or tk_field.");
            break;
        }
    }
    pop_error_context();
    ast ret = create_class_var_decs(decs);
    return ret;
}

// static_var_dec ::= 'static' type identifier (',' identifier)* ';'
// returns vector of ast_var_dec
//
// ast_var_dec: create_var_dec(string name,string segment,int offset,string type)
// create_var_dec must be passed:
// . name: the variable's name
// . segment: the variable's segment
// . offset: the variable's position in it's segment
// . type: the variable's type
//
ast parse_static_var_dec()
{
    push_error_context("parse_class()");

    vector<ast> decs;
    string segment = "static";

    mustbe(tk_static);
    Token type = parse_type();
    Token name = mustbe(tk_identifier);
    int offset = static_offset;
    static_offset++;

    // store variable to lookup
    st_variable insert = st_variable(token_spelling(name), token_spelling(type), segment, offset);
    
    if (!insert_variables(func_table, token_spelling(name), insert))
    {
        fatal_token_context("variable already declared");
    }
    decs.push_back(create_var_dec(token_spelling(name), segment, offset, token_spelling(type)));

    // (',' identifier)*
    while (have(tk_comma))
    {
        mustbe(tk_comma);
        name = mustbe(tk_identifier);
        offset = static_offset;
        static_offset++;

        insert = st_variable(token_spelling(name), token_spelling(type), segment, offset);
        
        if (!insert_variables(func_table, token_spelling(name), insert))
        {
            fatal_token_context("variable already declared");
        }
        decs.push_back(create_var_dec(token_spelling(name), segment, offset, token_spelling(type)));
    }

    mustbe(tk_semi);
    pop_error_context();
    ast ret = create_var_decs(decs);
    return ret;
}

// field_var_dec ::= 'field' type identifier (',' identifier)* ';'
// returns vector of ast_var_dec
//
// ast_var_dec: create_var_dec(string name,string segment,int offset,string type)
// create_var_dec must be passed:
// . name: the variable's name
// . segment: the variable's segment
// . offset: the variable's position in it's segment
// . type: the variable's type
//
ast parse_field_var_dec()
{
    push_error_context("parse_class()");

    vector<ast> decs;
    mustbe(tk_field);
    string segment = "this";
    Token type = parse_type();
    Token name = mustbe(tk_identifier);
    int offset = field_offset;
    field_offset++;

    // store variable to lookup
    st_variable insert = st_variable(token_spelling(name), token_spelling(type), segment, offset);
    
    if (!insert_variables(func_table, token_spelling(name), insert))
    {
        fatal_token_context("variable already declared");
    }
    decs.push_back(create_var_dec(token_spelling(name), segment, offset, token_spelling(type)));

    // (',' identifier)*
    while (have(tk_comma))
    {
        mustbe(tk_comma);
        name = mustbe(tk_identifier);
        offset = field_offset;
        field_offset++;

        insert = st_variable(token_spelling(name), token_spelling(type), segment, offset);
        
        if (!insert_variables(func_table, token_spelling(name), insert))
        {
            fatal_token_context("variable already declared");
        }
        decs.push_back(create_var_dec(token_spelling(name), segment, offset, token_spelling(type)));
    }

    mustbe(tk_semi);
    pop_error_context();
    ast ret = create_var_decs(decs);
    return ret;
}

// type ::= 'int' | 'char' | 'boolean' | identifier
// returns the Token for the type
Token parse_type()
{
    push_error_context("parse_type()");
    Token tokentype = nullptr;

    // if one of the type tokens found
    switch (token_kind())
    {
    case tk_int:
        tokentype = mustbe(tk_int);
        break;

    case tk_char:
        tokentype = mustbe(tk_char);
        break;

    case tk_boolean:
        tokentype = mustbe(tk_boolean);
        break;

    case tk_identifier:
        tokentype = mustbe(tk_identifier);
        break;

    default:
        did_not_find(tg_starts_type);
        fatal_token_context("did not find valid parse_type()");
        break;
    }

    pop_error_context();
    Token ret = tokentype;
    return ret;
}

// vtype ::= 'void' | type
// returns the Token for the type
Token parse_vtype()
{
    push_error_context("parse_vtype()");

    Token tokentype = nullptr;

    switch (token_kind())
    {
    case tk_void:
        tokentype = mustbe(tk_void);
        break;

    default:
        tokentype = parse_type();
        break;
    }

    pop_error_context();
    Token ret = tokentype;
    return ret;
}

// subr_decs ::= (constructor | function | method)*
// returns ast_subr_decs: create_subr_decs(vector<ast> subrs)
// create_subr_decs must be passed an vector of ast_subr
//
// ast_subr: create_subr(ast subr)
// create_subr must be passed one of: ast_constructor, ast_function or ast_method
//
ast parse_subr_decs()
{
    vector<ast> subrs;
    push_error_context("parse_subr_decs()");

    // if one of the tokens found
    while (have(tk_function) || have(tk_constructor) || have(tk_method))
    {
        if (have(tk_constructor))
        {
            subrs.push_back(create_subr(parse_constructor()));
        }
        else if (have(tk_function))
        {
            subrs.push_back(create_subr(parse_function()));
        }
        else if (have(tk_method))
        {
            subrs.push_back(create_subr(parse_method()));
        }
        else
        {
            fatal_token_context("no token found for subrs_decs");
        }
    }
    pop_error_context();
    ast ret = create_subr_decs(subrs);
    return ret;
}

// constructor ::= 'constructor' identifier identifier '(' param_list ')' subr_body
// returns ast_constructor: create_constructor(string vtype,string name,ast params,ast body)
// . vtype: the constructor's return type, this must be it's class name
// . name: the constructor's name within its class
// . params: ast_param_list - the constructor's parameters
// . body: ast_subr_body - the constructor's body
//
ast parse_constructor()
{
    push_error_context("parse_constructor()");

    func_table = create_variables();
    string vtype = "";
    string name = "";
    ast params = nullptr;
    ast body = nullptr;
    arg_offset = 0;
    local_offset = 0;

    // if 'constructor' found
    mustbe(tk_constructor);
    vtype = token_spelling(mustbe(tk_identifier));
    name = token_spelling(mustbe(tk_identifier));
    mustbe(tk_lrb);

    params = parse_param_list();
    mustbe(tk_rrb);
    body = parse_subr_body();
    delete_variables(func_table);

    pop_error_context();
    ast ret = create_constructor(vtype, name, params, body);
    return ret;
}

// function ::= 'function' vtype identifier '(' param_list ')' subr_body
// returns ast_function: create_function(string vtype,string name,ast params,ast body)
// . vtype: the function's return type
// . name: the function's name within its class
// . params: ast_param_list - the function's parameters
// . body: ast_subr_body - the function's body
//
ast parse_function()
{
    push_error_context("parse_function()");

    func_table = create_variables();
    string vtype = "";
    string name = "";
    ast params = nullptr;
    ast body = nullptr;
    local_offset = 0;
    arg_offset = 0;

    // start of parse statement
    mustbe(tk_function);
    vtype = token_spelling(parse_vtype());
    name = token_spelling(mustbe(tk_identifier));
    mustbe(tk_lrb);

    //insert variables (?)
    params = parse_param_list();
    mustbe(tk_rrb);
    body = parse_subr_body();
    delete_variables(func_table);

    pop_error_context();
    ast ret = create_function(vtype, name, params, body);
    return ret;
}

// method ::= 'method' vtype identifier '(' param_list ')' subr_body
// returns ast_method: create_method(string vtype,string name,ast params,ast body)
// . vtype: the method's return type
// . name: the method;s name within its class
// . params: ast_param_list - the method's explicit parameters
// . body: ast_subr_body - the method's body
//
ast parse_method()
{
    push_error_context("parse_method()");
    func_table = create_variables();
    ast params = nullptr;
    ast body = nullptr;
    string vtype = "";
    string name = "";
    local_offset = 0;
    arg_offset = 1;

    // start of parse statement
    mustbe(tk_method);
    vtype = token_spelling(parse_vtype());
    name = token_spelling(mustbe(tk_identifier));
    mustbe(tk_lrb);

    params = parse_param_list();
    mustbe(tk_rrb);
    body = parse_subr_body();

    delete_variables(func_table);
    pop_error_context();
    ast ret = create_method(vtype, name, params, body);
    return ret;
}

// param_list ::= ((type identifier) (',' type identifier)*)?
// returns ast_param_list: create_param_list(vector<ast> params)
// create_param_list must be passed a vector of ast_var_dec
//
// ast_var_dec: create_var_dec(string name,string segment,int offset,string type)
// create_var_dec must be passed:
// . name: the variable's name
// . segment: the variable's segment
// . offset: the variable's position in it's segment
// . type: the variable's type
//
ast parse_param_list()
{
    push_error_context("parse_param_list()");

    vector<ast> decs;

    // if one of the tokens found
    if (!have(tg_starts_type))
    {
        return create_param_list(decs);
    }

    Token type = parse_type();
    Token name = mustbe(tk_identifier);
    string segment = "argument";
    int offset = arg_offset;
    arg_offset++;

    // store variable to lookup
    st_variable insert = st_variable(token_spelling(name), token_spelling(type), segment, offset);
    
    if (!insert_variables(func_table, token_spelling(name), insert))
    {
        fatal_token_context("variable already declared");
    }
    decs.push_back(create_var_dec(token_spelling(name), segment, offset, token_spelling(type)));

    while (have(tk_comma))
    {
        mustbe(tk_comma);
        type = parse_type();
        name = mustbe(tk_identifier);
        offset = arg_offset;
        arg_offset++;

        // store variable to lookup
        st_variable insert = st_variable(token_spelling(name), token_spelling(type), segment, offset);
        
        if (!insert_variables(func_table, token_spelling(name), insert))
        {
            fatal_token_context("variable already declared");
        }
        decs.push_back(create_var_dec(token_spelling(name), segment, offset, token_spelling(type)));
    }

    pop_error_context();
    ast ret = create_param_list(decs);
    return ret;
}

// subr_body ::= '{' var_decs statements '}'
// returns ast_subr_body: create_subr_body(ast decs,ast body)
// create_subr_body must be passed:
// . decs: ast_var_decs - the subroutine's local variable declarations
// . body: ast_statements - the statements within the body of the subroutinue
//
ast parse_subr_body()
{
    push_error_context("parse_subr_body()");

    ast decs = nullptr;
    ast body = nullptr;

    // start of parse statement
    mustbe(tk_lcb);
    decs = parse_var_decs();
    body = parse_statements();
    mustbe(tk_rcb);

    pop_error_context();
    ast ret = create_subr_body(decs, body);
    return ret;
}

// var_decs ::= var_dec*
// returns ast_var_decs: create_var_decs(vector<ast> decs)
// create_var_decs must be passed a vector of ast_var_dec
//
ast parse_var_decs()
{
    vector<ast> decs;
    push_error_context("parse_var_decs()");

    // if one of the tokens found
    while (have(tk_var))
    {
        decs.push_back(parse_var_dec());
    }

    pop_error_context();
    ast ret = create_var_decs(decs);
    return ret;
}

// var_dec ::= 'var' type identifier (',' identifier)* ';'
// returns vector of ast_var_dec
//
// ast_var_dec: create_var_dec(string name,string segment,int offset,string type)
// create_var_dec must be passed:
// . name: the variable's name
// . segment: the variable's segment
// . offset: the variable's position in it's segment
// . type: the variable's type
//
ast parse_var_dec()
{
    vector<ast> decs;
    push_error_context("parse_var_dec()");

    mustbe(tk_var);
    string type = token_spelling(parse_type());
    string name = token_spelling(mustbe(tk_identifier));
    string segment = "local";

    st_variable insert = st_variable(name, type, segment, local_offset);

    // insert_variables(func_table, name, insert) ;
    if (!insert_variables(func_table, name, insert))
    {
        fatal_token_context("variable already declared");
    }
    decs.push_back(create_var_dec(name, segment, local_offset, type));

    local_offset++;

    // (',' identifier)*
    while (have(tk_comma))
    {
        mustbe(tk_comma);
        name = token_spelling(mustbe(tk_identifier));
        st_variable insert = st_variable(name, type, segment, local_offset);

        //insert_variables(func_table, name, insert) ;
        if (!insert_variables(func_table, name, insert))
        {
            fatal_token_context("variable already declared");
        }
        decs.push_back(create_var_dec(name, segment, local_offset, type));
        local_offset++;
    }

    mustbe(tk_semi);
    pop_error_context();
    ast ret = create_var_decs(decs);
    return ret;
}

// statements ::= statement*
// create_statements(vector<ast> statements)
// create_statements must be passed a vector of ast_statement
//
ast parse_statements()
{
    vector<ast> statements;
    push_error_context("parse_statements()");

    // if tg_starts statement found
    while (have(tg_starts_statement))
    {
        statements.push_back(parse_statement());
    }

    pop_error_context();
    ast ret = create_statements(statements);
    return ret;
}

// statement ::= let | if | while | do | return
// create_statement(ast statement)
// create_statement initialiser must be one of: ast_let, ast_let_array, ast_if, ast_if_else, ast_while, ast_do, ast_return or ast_return_expr
//
ast parse_statement()
{
    push_error_context("parse_statement()");
    ast stat = nullptr;

    // if either one of the statements found
    switch (token_kind())
    {
    case tk_let:
        stat = parse_let();
        break;

    case tk_if:
        stat = parse_if();
        break;

    case tk_while:
        stat = parse_while();
        break;

    case tk_do:
        stat = parse_do();
        break;

    case tk_return:
        stat = parse_return();
        break;

    default:
        did_not_find(tg_starts_statement);
        fatal_token_context("did not find statement");
        break;
    }

    // return a statement node
    stat = create_statement(stat);
    pop_error_context();
    return stat;
}

// let ::= 'let' identifier index? '=' expr ';'
// return one of: ast_let or ast_let_array
//
// create_let(ast var,ast expr)
// . var: ast_var - the variable being assigned to
// . expr: ast_expr - the variable's new value
//
// create_let_array(ast var,ast index,ast expr)
// . var: ast_var - the variable for the array being assigned to
// . index: ast_expr - the array index
// . expr: ast_expr - the array element's new value
//
ast parse_let()
{
    push_error_context("parse_let()");

    ast var = nullptr;
    ast expr = nullptr;
    ast index = nullptr;
    Token id = nullptr;
    string name = "";
    st_variable st_var;

    mustbe(tk_let);
    id = mustbe(tk_identifier);
    name = token_spelling(id);

    // lookup variable in symbol table
    st_var = lookup_variables(func_table, name);

    // if offset -1 
    if (st_var.offset == -1)
    {
        st_var = lookup_variables(var_table, name);
    }

    // wanted to make them in one if statement, but somehow does not work if they are together.
    if (st_var.offset == -1)
    {
        fatal_token_context("parse_let() variable not declared");
    }

    var = create_var(st_var.name, st_var.segment, st_var.offset, st_var.type);

    if (have(tk_lsb))
    {
        index = parse_index();
        mustbe(tk_eq);
        expr = parse_expr();
        mustbe(tk_semi);
        return create_let_array(var, index, expr);
    }

    mustbe(tk_eq);
    expr = parse_expr();
    mustbe(tk_semi);

    pop_error_context();
    ast ret = create_let(var, expr);
    return ret;
}

// if ::= 'if' '(' expr ')' '{' statements '}' ('else' '{' statements '}')?
// return one of: ast_if or ast_if_else
//
// create_if(ast condition,ast if_true)
// . condition: ast_expr - the if condition
// . if_true: ast_statements - the if true branch
//
// create_if_else(ast condition,ast if_true,ast if_false)
// . condition: ast_expr - the if condition
// . if_true: ast_statements - the if true branch
// . if_false: ast_statements - the if false branch
//
ast parse_if()
{
    push_error_context("parse_if()");
    ast condition = nullptr;
    ast if_true = nullptr;
    ast if_false = nullptr;

    // start of parse statement
    mustbe(tk_if);
    mustbe(tk_lrb);
    condition = parse_expr();
    mustbe(tk_rrb);
    mustbe(tk_lcb);
    if_true = parse_statements();
    mustbe(tk_rcb);

    // if tk_else found
    if (have(tk_else))
    {
        mustbe(tk_else);
        mustbe(tk_lcb);
        if_false = parse_statements();
        mustbe(tk_rcb);
        return create_if_else(condition, if_true, if_false);
    }

    pop_error_context();
    ast ret = create_if(condition, if_true);
    return ret;
}

// while ::= 'while' '(' expr ')' '{' statements '}'
// returns ast_while: create_while(ast condition,ast body)
// . condition: ast_expr - the loop condition
// . body: ast_statements - the loop body
//
ast parse_while()
{
    push_error_context("parse_while()");

    ast condition = nullptr;
    ast body = nullptr;

    // starts with tk_while
    mustbe(tk_while);
    mustbe(tk_lrb);
    
    condition = parse_expr();
    mustbe(tk_rrb);
    mustbe(tk_lcb);

    body = parse_statements();
    mustbe(tk_rcb);

    pop_error_context();
    ast ret = create_while(condition, body);
    return ret;
}

// do ::= 'do' identifier (call | id_call) ';'
// returns ast_do: create_do(ast call)
// create_do must be passed one of: ast_call_as_function or ast_call_as_method
//
// ast_call_as_function: create_call_as_function(string class_name,ast subr_call)
// create_ast_call_as_function must be passed:
// . class_name: name of the function's class
// . subr_call: ast_subr_call - the function's name within it's class and it's explicit arguments
//
// ast_call_as_method: create_call_as_method(string class_name,ast object,ast subr_call)
// create_ast_call_as_method must be passed:
// . class_name: name of the method's class
// . object: ast_expr - the object the method is applied to
// . subr_call: ast_subr_call - the method's name within it's class and it's explicit arguments
//
ast parse_do()
{
    push_error_context("parse_do()");

    ast call = nullptr;
    ast expr_list = nullptr;
    ast subr_call = nullptr;
    ast ast_object = nullptr;
    st_variable object;
    Token id = nullptr;
    string name = "";

    mustbe(tk_do);
    id = mustbe(tk_identifier);
    name = token_spelling(id);

    // if '(' found |  if '.' found
    if (have(tk_lrb))
    {
        expr_list = parse_call();
        subr_call = create_subr_call(name, expr_list);
        call = create_call_as_method(curr_classname, create_this(), subr_call);
    }
    else if (have(tk_stop))
    {
        subr_call = parse_id_call();
        object = lookup_variables(func_table, name);

        if (object.offset == -1)
        {
            object = lookup_variables(var_table, name);
        }
        if (object.offset == -1)
        {
            call = create_call_as_function(name, subr_call);
        }
        else
        {
            ast_object = create_var(object.name, object.segment, object.offset, object.type);
            call = create_call_as_method(object.type, ast_object, subr_call);
        }
    }
    else
    {
        fatal_token_context("no valid parse_do() token");
    }

    mustbe(tk_semi);
    pop_error_context();
    ast ret = create_do(call);
    return ret;
}

// return ::= 'return' expr? ';'
// returns one of: ast_return or ast_return_expr
//
// ast_return: create_return()
//
// ast_return_expr: create_return_expr(ast expr)
// create_return_expr must be passed an ast_expr
//
ast parse_return()
{
    ast return_expr = nullptr;

    push_error_context("parse_return()");
    mustbe(tk_return);

    // if starts a term found
    if (have(tg_starts_term))
    {
        return_expr = create_return_expr(parse_expr());
    }
    else
    {
        return_expr = create_return();
    }

    mustbe(tk_semi);
    pop_error_context();
    ast ret = return_expr;
    return ret;
}

// expr ::= term (infix_op term)*
// returns ast_expr: create_expr(vector<ast> expr)
// the vector passed to create_expr:
// . must be an odd length of at least 1, ie 1, 3, 5, ...
// . must be  a vector alternating between ast_term and ast_infix_op nodes, ending with an ast_term
//
ast parse_expr()
{
    push_error_context("parse_expr()");

    vector<ast> expr;

    expr.push_back(parse_term());

    // if an infix op is found
    while (have(tg_infix_op))
    {
        expr.push_back(parse_infix_op());
        expr.push_back(parse_term());
    }

    pop_error_context();
    ast ret = create_expr(expr);
    return ret;
}

// term ::= integer_constant | string_constant | 'true' | 'false' | 'null' | 'this' | '(' expr ')' | unary_op term | var_term
// returns ast_term: create_term(ast term)
// create_term must be passed one of: ast_int, ast_string, ast_bool, ast_null, ast_this, ast_expr,
//                                    ast_unary_op, ast_var, ast_array_index, ast_call_as_function, ast_call_as_method
//
// ast_int: create_int(int _constant)
// create_int must be passed an integer value in the range 0 to 32767
//
// ast_string: create_string(string _constant)
// create_string must be passed any legal Jack string literal
//
// ast_bool: create_bool(bool t_or_f)
// create_bool must be passed true or false
//
// ast_null: create_null()
//
// ast_this: create_this()
//
// ast_unary_op: create_unary_op(string op,ast term)
// create_unary_op must be passed:
// . op: the unary op
// . term: ast_term
//
ast parse_term()
{
    push_error_context("parse_term()");
    ast term = nullptr;
    ast temp = nullptr;
    Token value = nullptr;

    // if any of the terms found
    if (have(tk_integerConstant))
    {
        value = mustbe(tk_integerConstant);
        term = create_int(token_ivalue(value));
    }
    else if (have(tk_stringConstant))
    {
        value = mustbe(tk_stringConstant);
        term = create_string(token_spelling(value));
    }
    else if (have(tk_true))
    {
        mustbe(tk_true);
        term = create_bool(true);
    }
    else if (have(tk_false))
    {
        mustbe(tk_false);
        term = create_bool(false);
    }
    else if (have(tk_null))
    {
        mustbe(tk_null);
        term = create_null();
    }
    else if (have(tk_this))
    {
        mustbe(tk_this);
        term = create_this();
    }
    else if (have(tk_lrb))
    {
        mustbe(tk_lrb);
        term = parse_expr();
        mustbe(tk_rrb);
    }
    else if (have(tk_sub))
    {
        value = parse_unary_op();
        temp = parse_term();
        term = create_unary_op(token_spelling(value), temp);
    }
    else if (have(tk_not))
    {
        value = parse_unary_op();
        temp = parse_term();
        term = create_unary_op(token_spelling(value), temp);
    }
    else if (have(tk_identifier))
    {
        term = parse_var_term();
    }
    else
    {
        fatal_token_context("did not find a valid parse_term");
    }

    pop_error_context();
    ast ret = create_term(term);
    return ret;
}

// var_term ::= identifier (index | id_call | call)?
// returns one of: ast_var, ast_array_index, ast_call_as_function or ast_call_as_method
//
// ast_var: create_var(string name,string segment,int offset,string type)
// create_ast_var must be passed:
// . name: the variable's name
// . segment: the variable's segment
// . offset: the variable's position in it's segment
// . type: the variable's type
//
// ast_array_index: create_array_index(ast var,ast index)
// create_ast_array_index must be passed:
// . var: ast_var - the array variable
// . index: ast_expr - the index into the array
//
// ast_call_as_function: create_call_as_function(string class_name,ast subr_call)
// create_ast_call_as_function must be passed:
// . class_name: name of the function's class
// . subr_call: ast_subr_call - the function's name within it's class and it's explicit arguments
//
// ast_call_as_method: create_call_as_method(string class_name,ast object,ast subr_call)
// create_ast_call_as_method must be passed:
// . class_name: name of the method's class
// . object: ast_expr - the object the method is applied to - THIS IS WRONG OR CONFLICTS WHAT'S IN THE HEADER FILE
// . subr_call: ast_subr_call - the method's name within it's class and it's explicit arguments
//
ast parse_var_term()
{
    push_error_context("parse_var_term()");

    ast index = nullptr;
    ast subr_call = nullptr;
    ast expr_list = nullptr;
    Token id = nullptr;
    string name = "";

    id = mustbe(tk_identifier);
    name = token_spelling(id);

    // ast_array_index: create_array_index(ast var,ast index)
    // tk_lsb := '['
    if (have(tk_lsb))
    {
        index = parse_index();

        st_variable var_term = lookup_variables(func_table, name);

        if (var_term.offset == -1)
        {
            var_term = lookup_variables(var_table, name);
        }
        if (var_term.offset == -1)
        {
            fatal_token_context("parse_var_term variable not declared");
        }

        ast var = create_var(var_term.name, var_term.segment, var_term.offset, var_term.type);
        return create_array_index(var, index);
    }

    // ast_call_as_function: create_call_as_function(string class_name,ast subr_call)
    // tk_stop := '.'
    else if (have(tk_stop))
    {
        subr_call = parse_id_call();

        st_variable object = lookup_variables(func_table, name);

        if (object.offset == -1)
        {
            object = lookup_variables(var_table, name);
        }

        if (object.offset == -1)
        {
            return create_call_as_function(name, subr_call);
        }
        else if (object.offset >= 0)
        {
            ast ast_object = create_var(object.name, object.segment, object.offset, object.type);
            return create_call_as_method(object.type, ast_object, subr_call);
        }
    }

    // ast_call_as_method: create_call_as_method(string class_name,ast object,ast subr_call)
    // tk_lrb := ')'
    else if (have(tk_lrb))
    {
        expr_list = parse_call();
        subr_call = create_subr_call(name, expr_list);
        return create_call_as_method(curr_classname, create_this(), subr_call);
    }
    // ast_var: create_var(string name,string segment,int offset,string type)
    else
    {
        st_variable var = lookup_variables(func_table, name);

        if (var.offset == -1)
        {
            var = lookup_variables(var_table, name);
        }
        if (var.offset == -1)
        {
            fatal_token_context("parse_var_term variable not declared");
        }

        return create_var(var.name, var.segment, var.offset, var.type);
    }

    pop_error_context();
    ast ret = create_empty();
    return ret;
}

// index ::= '[' expr ']'
// returns ast_expr
ast parse_index()
{
    push_error_context("parse_index()");
    ast expr = nullptr;

    // start of parse statement
    mustbe(tk_lsb);
    expr = parse_expr();
    mustbe(tk_rsb);

    pop_error_context();
    return expr;
}

// id_call ::= '.' identifier call
// returns ast_subr_call: create_subr_call(string subr_name,ast expr_list)
// create_subr_call must be passed:
// . subr_name: the constructor, function or method's name within its class
// . expr_list: ast_expr_list - the explicit arguments to the call
//
ast parse_id_call()
{
    push_error_context("parse_id_call()");

    // start of parse statement
    mustbe(tk_stop);
    Token name = mustbe(tk_identifier);
    string subr_name = token_spelling(name);
    ast exprs = parse_call();

    ast ret = create_subr_call(subr_name, exprs);
    return ret;
}

// call ::= '(' expr_list ')'
// returns ast_expr_list
//
ast parse_call()
{
    push_error_context("parse_call()");

    mustbe(tk_lrb);
    ast expr_list = parse_expr_list();
    mustbe(tk_rrb);

    pop_error_context();
    return expr_list;
}

// expr_list ::= (expr (',' expr)*)?
// returns ast_expr_list: create_expr_list(vector<ast> exprs)
// create_expr_list must be passed: a vector of ast_expr
//
ast parse_expr_list()
{
    push_error_context("parse_expr_list()");
    vector<ast> exprs;

    if (have(tg_starts_term))
    {
        exprs.push_back(parse_expr());
        while (have(tk_comma))
        {
            mustbe(tk_comma);
            exprs.push_back(parse_expr());
        }
    }

    pop_error_context();
    return create_expr_list(exprs);
}

// infix_op ::= '+' | '-' | '*' | '/' | '&' | '|' | '<' | '>' | '='
// returns ast_op: create_infix_op(string infix_op)
// create_infix_op must be passed:
// infix_op: the infix op
//
ast parse_infix_op()
{
    push_error_context("parse_infix_op()");

    Token op = nullptr;

    switch (token_kind())
    {

    case tk_add:
        op = mustbe(tk_add);
        break;

    case tk_sub:
        op = mustbe(tk_sub);
        break;

    case tk_times:
        op = mustbe(tk_times);
        break;

    case tk_divide:
        op = mustbe(tk_divide);
        break;

    case tk_and:
        op = mustbe(tk_and);
        break;

    case tk_or:
        op = mustbe(tk_or);
        break;

    case tk_lt:
        op = mustbe(tk_lt);
        break;

    case tk_gt:
        op = mustbe(tk_gt);
        break;

    case tk_eq:
        op = mustbe(tk_eq);
        break;

    case tk_not:
        op = mustbe(tk_not);
        break;

    default:
        did_not_find(tg_infix_op);
        fatal_token_context("did not find infix op");
        break;
    }

    pop_error_context();
    ast ret = create_infix_op(token_spelling(op));
    return ret;
}

// unary_op ::= '-' | '~'
// returns Token for the unary_op
//
Token parse_unary_op()
{
    push_error_context("parse_unary_op()");

    Token op = nullptr;

    switch (token_kind())
    {
    case tk_sub:
        op = mustbe(tk_sub);
        break;

    case tk_not:
        op = mustbe(tk_not);
        break;

    default:
        did_not_find(tg_unary_op);
        fatal_token_context("did not find unary op");
        break;
    }
    pop_error_context();
    return op;
}

// jack parser and calls create_variables when filling up variable symbol table
ast jack_parser()
{
    var_table = create_variables();
    // read the first token to get the tokeniser initialised
    next_token();

    // construct tree and return as result
    return parse_class();
}

// main program
int main(int argc, char **argv)
{
    // parse a Jack class and print the abstract syntax tree as XML
    ast_print_as_xml(jack_parser(), 4);

    // flush the output and any errors
    print_output();
    print_errors();
}
