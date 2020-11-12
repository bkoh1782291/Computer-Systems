// Brian Koh Lit Yang
// a1782291

#include "iobuffer.h"
#include "symbols.h"
#include "abstract-syntax-tree.h"

// to shorten our code:
using namespace std;
using namespace CS_IO_Buffers;
using namespace Jack_Compiler;

// ***** WHAT TO DO *****
//
// MODIFY the skeleton code below to walk an abstract syntax tree, ast, of a Jack class
//        and pretty print the equivalent Jack source code.
//
// NOTE: the correct segment and offset is recorded with every variable in the ast
//       so the code generator does not need to use any symbol tables
//
// The skeleton code has one function per node in the abstract tree
//  - they each extract all fields into local variables
//  - they each call the appropriate walk_* function to walk any sub-trees
//
// The structure of the skeleton code is just a suggestion
//  - you may want to change the parameters / results to suit your own logic
//  - you can change it as much as you like

// forward declarations of one function per node in the abstract syntax tree
void walk_class(ast t);
void walk_class_var_decs(ast t);
void walk_var_dec(ast t);
void walk_subr_decs(ast t);
void walk_subr(ast t);
void walk_constructor(ast t);
void walk_function(ast t);
void walk_method(ast t);
void walk_param_list(ast t);
void walk_subr_body(ast t);
void walk_var_decs(ast t);
void walk_statements(ast t);
void walk_statement(ast t);
void walk_let(ast t);
void walk_let_array(ast t);
void walk_if(ast t);
void walk_if_else(ast t);
void walk_while(ast t);
void walk_do(ast t);
void walk_return(ast t);
void walk_return_expr(ast t);
void walk_expr(ast t);
void walk_term(ast t);
void walk_int(ast t);
void walk_string(ast t);
void walk_bool(ast t);
void walk_null(ast t);
void walk_this(ast t);
void walk_unary_op(ast t);
void walk_var(ast t);
void walk_array_index(ast t);
void walk_call_as_function(ast t);
void walk_call_as_method(ast t);
void walk_subr_call(ast t);
void walk_expr_list(ast t);
void walk_infix_op(ast t);

// global variable for indentation levels
int indentation = 1;

// walk an ast class node with fields:
// class_name - a string
// var_decs   - ast vector of variable declarations
// subr_decs  - ast vector of subroutine declarations
//
void walk_class(ast t)
{
    string myclassname = get_class_class_name(t);
    ast var_decs = get_class_var_decs(t);
    ast subr_decs = get_class_subr_decs(t);

    // class myclassname { var_decs subr_decs }
    write_to_output("class " + myclassname + "\n");
    write_to_output("{\n");

    walk_class_var_decs(var_decs);
    walk_subr_decs(subr_decs);

    write_to_output("}\n");
}

// walk an ast class var decs node
// it is an ast vector of variable declarations
//
void walk_class_var_decs(ast t)
{
    // class_var_decs ::= (static_var_dec | field_var_dec)*
    int ndecs = size_of_class_var_decs(t);
    vector<ast> statics;
    vector<ast> fields;
    vector<ast>::iterator it;

    // push back all "this" and "static" decs to split them up later
    for (int i = 0; i < ndecs; i++)
    {
        ast next_dec = get_class_var_decs(t, i);

        if (get_var_dec_segment(next_dec) == "this")
        {
            fields.push_back(next_dec);
        }

        else if (get_var_dec_segment(next_dec) == "static")
        {
            statics.push_back(next_dec);
        }
    }

    for (it = statics.begin(); it != statics.end(); ++it)
    {
        walk_var_dec(*it);
    };

    if (fields.size() > 0 && statics.size() > 0)
    {
        write_to_output("\n");
    };

    for (it = fields.begin(); it != fields.end(); ++it)
    {
        walk_var_dec(*it);
    };

    // add newline at the end
    if (ndecs)
    {
        write_to_output("\n");
    }
}

// walk an ast variable declaration with fields
// name - a string
// type - a string - "int", "char", "boolean" or a class name
// segment - a string
// offset - an int
// this is used for statics, fields, parameters and local variables
//
void walk_var_dec(ast t)
{
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    string name = get_var_dec_name(t);
    string type = get_var_dec_type(t);
    string segment = get_var_dec_segment(t);
    int offset = get_var_dec_offset(t);

    string var = "";

    if (segment == "static")
    {
        var = "static ";
    }
    else if (segment == "this")
    {
        var = "field ";
    }
    else
    {
        var = "var ";
    }

    // field_var_dec ::= 'field' type identifier (',' identifier)* ';'
    write_to_output(wspace + var + type + ' ' + name + " ;\n");
}

// walk an ast class var decs node
// it is an ast vector of subroutine declarations
//
void walk_subr_decs(ast t)
{
    int size = size_of_subr_decs(t);

    // subr_decs ::= (constructor | function | method)*
    for (int i = 0; i < size; i++)
    {
        walk_subr(get_subr_decs(t, i));

        if (i != size - 1)
        {
            write_to_output("\n");
        }
    }
}

// walk an ast subroutine node with a single field
// subr - an ast constructor, ast function or ast method node
//
void walk_subr(ast t)
{
    ast subr = get_subr_subr(t);

    // subr_decs ::= (constructor | function | method)*
    switch (ast_node_kind(subr))
    {
    case ast_constructor:
        walk_constructor(subr);
        break;
    case ast_function:
        walk_function(subr);
        break;
    case ast_method:
        walk_method(subr);
        break;
    default:
        fatal_error(0, "Unexpected subroutine kind");
        break;
    }
}

// walk an ast constructor node with fields
// vtype - a string - the name of the class
// name - a string
// param list - an ast vector of variable declarations
// subr body - an ast subr body node
//
void walk_constructor(ast t)
{
    indentation = 1;

    // constructor ::= 'constructor' identifier identifier '(' param_list ')' subr_body
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    string vtype = get_constructor_vtype(t);
    string name = get_constructor_name(t);
    ast param_list = get_constructor_param_list(t);
    ast subr_body = get_constructor_subr_body(t);

    // output for constructor
    write_to_output(wspace + "constructor " + vtype + ' ' + name + '(');
    walk_param_list(param_list);
    write_to_output(")\n");

    write_to_output(wspace + "{\n");
    walk_subr_body(subr_body);
    write_to_output(wspace + "}\n");

    indentation = 1;
}

// walk an ast function node with fields
// vtype - a string - one of "void", "int", "char", "boolean" or the name of a class
// name - a string
// param list - an ast vector of variable declarations
// subr body - an ast subr body node
//
void walk_function(ast t)
{
    indentation = 1;

    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // constructor ::= 'constructor' identifier identifier '(' param_list ')' subr_body
    string vtype = get_function_vtype(t);
    string name = get_function_name(t);
    ast param_list = get_function_param_list(t);
    ast subr_body = get_function_subr_body(t);

    // output for function
    write_to_output(wspace + "function " + vtype + ' ' + name + '(');
    walk_param_list(param_list);
    write_to_output(")\n");

    write_to_output(wspace + "{\n");
    walk_subr_body(subr_body);
    write_to_output(wspace + "}\n");

    indentation = 1;
}

// walk an ast method node with fields
// vtype - a string - one of "void", "int", "char", "boolean" or the name of a class
// name - a string
// param list - an ast vector of variable declarations
// subr body - an ast subr body node
//
void walk_method(ast t)
{
    indentation = 1;

    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // method ::= 'method' vtype identifier '(' param_list ')' subr_body
    string vtype = get_method_vtype(t);
    string name = get_method_name(t);
    ast param_list = get_method_param_list(t);
    ast subr_body = get_method_subr_body(t);

    // output for method
    write_to_output(wspace + "method " + vtype + ' ' + name + '(');
    walk_param_list(param_list);
    write_to_output(")\n");

    write_to_output(wspace + "{\n");
    walk_subr_body(subr_body);
    write_to_output(wspace + "}\n");

    indentation = 1;
}

// walk an ast param list node
// it is an ast vector of variable declarations
//
void walk_param_list(ast t)
{
    // param_list ::= ((type identifier) (',' type identifier)*)?
    int ndecs = size_of_param_list(t);

    for (int i = 0; i < ndecs; i++)
    {
        ast param = get_param_list(t, i);
        string type = get_var_dec_type(param);
        string name = get_var_dec_name(param);

        write_to_output(type + ' ' + name);

        if (i != ndecs - 1)
        {
            write_to_output(",");
        }
    }
}

// walk an ast subr body node with fields
// decs - an ast vector of variable declarations
// body - an ast vector of statement nodes
//
void walk_subr_body(ast t)
{
    ast decs = get_subr_body_decs(t);
    ast body = get_subr_body_body(t);

    // subr_body ::= '{' var_decs statements '}'
    walk_var_decs(decs);
    walk_statements(body);
}

// walk an ast param list node
// it is an ast vector of variable declarations
//
void walk_var_decs(ast t)
{
    // increase and decrease indentation
    ++indentation;

    // var_decs ::= var_dec*
    int ndecs = size_of_var_decs(t);

    for (int i = 0; i < ndecs; i++)
    {
        walk_var_dec(get_var_decs(t, i));
    }

    if (ndecs)
        write_to_output("\n");

    --indentation;
}

// walk an ast statements node
// it is an ast vector of statement nodes
//
void walk_statements(ast t)
{
    int nstatements = size_of_statements(t);
    for (int i = 0; i < nstatements; i++)
    {
        ast statement = get_statements(t, i);
        walk_statement(statement);

        ast statement_statement = get_statement_statement(statement);
        bool insert_newline = false;

        // seperates between while, if and if-else statements
        // prints a new line if they are either one of these
        if (ast_have_kind(statement_statement, ast_while) && i != (nstatements - 1))
        {
            insert_newline = true;
        }
        else if (ast_have_kind(statement_statement, ast_if) && i != (nstatements - 1))
        {
            insert_newline = true;
        }
        else if (ast_have_kind(statement_statement, ast_if_else) && i != (nstatements - 1))
        {
            insert_newline = true;
        }

        if (insert_newline == true)
        {
            write_to_output("\n");
        }
    }
}

// walk an ast statement node with a single field
// statement - one of the following ast nodes, let, let array, if, if else, while, do, return, return expr or statements
//
void walk_statement(ast t)
{
    ++indentation;

    ast statement = get_statement_statement(t);

    // if one of the statement nodes are found
    switch (ast_node_kind(statement))
    {
    case ast_let:
        walk_let(statement);
        break;
    case ast_let_array:
        walk_let_array(statement);
        break;
    case ast_if:
        walk_if(statement);
        break;
    case ast_if_else:
        walk_if_else(statement);
        break;
    case ast_while:
        walk_while(statement);
        break;
    case ast_do:
        walk_do(statement);
        break;
    case ast_return:
        walk_return(statement);
        break;
    case ast_return_expr:
        walk_return_expr(statement);
        break;
    case ast_statements:
        walk_statements(statement);
        break;
    default:
        fatal_error(0, "Unexpected statement kind");
        break;
    }

    --indentation;
}

// walk an ast let node with fields
// var  - an ast variable
// expr - an ast expr node
//
void walk_let(ast t)
{
    // indentation
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    ast var = get_let_var(t);
    ast expr = get_let_expr(t);

    // let ::= 'let' identifier index? '=' expr ';'
    write_to_output(wspace + "let ");
    walk_var(var);
    write_to_output(" = ");
    walk_expr(expr);
    write_to_output(" ;\n");
}

// walk an ast let array node with fields
// var    - an ast variable
// index  - an ast expr node
// expr   - an ast expr node
//
void walk_let_array(ast t)
{
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // walk the let array
    ast var = get_let_array_var(t);
    ast index = get_let_array_index(t);
    ast expr = get_let_array_expr(t);

    // writing to output
    write_to_output(wspace + "let ");
    walk_var(var);

    write_to_output("[");
    walk_expr(index);
    write_to_output("]");

    write_to_output(" = ");
    walk_expr(expr);
    write_to_output(" ;\n");
}

// walk an ast if node with fields
// condition - an ast expr node
// if true   - an ast statements node
//
void walk_if(ast t)
{
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // if ::= 'if' '(' expr ')' '{' statements '}' ('else' '{' statements '}')?
    ast condition = get_if_condition(t);
    ast if_true = get_if_if_true(t);

    // writing to output
    write_to_output(wspace + "if " + "(");
    walk_expr(condition);
    write_to_output(")\n");

    write_to_output(wspace + "{\n");
    walk_statements(if_true);
    write_to_output(wspace + "}\n");
}

// walk an ast if else node with fields
// condition - an ast expr node
// if true   - an ast statements node
// if else   - an ast statements node
//
void walk_if_else(ast t)
{
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    ast condition = get_if_else_condition(t);
    ast if_true = get_if_else_if_true(t);
    ast if_false = get_if_else_if_false(t);

    // writing to output
    write_to_output(wspace + "if " + "(");
    walk_expr(condition);
    write_to_output(")\n");

    write_to_output(wspace + "{\n");
    walk_statements(if_true);
    write_to_output(wspace + "}\n");

    write_to_output(wspace + "else\n");
    write_to_output(wspace + "{\n");
    walk_statements(if_false);
    write_to_output(wspace + "}\n");
}

// walk an ast while node with fields
// condition - an ast expr node
// body      - an ast statements node
//
void walk_while(ast t)
{
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // while ::= 'while' '(' expr ')' '{' statements '}'
    ast condition = get_while_condition(t);
    ast body = get_while_body(t);

    // writing to output
    write_to_output(wspace + "while ");
    write_to_output("(");
    walk_expr(condition);
    write_to_output(")\n");

    write_to_output(wspace + "{\n");
    walk_statements(body);
    write_to_output(wspace + "}\n");
}

// walk an ast do node with a single field
// call - an ast call as function node or an ast call as method node
//
void walk_do(ast t)
{
    // indentation
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // do ::= 'do' identifier (call | id_call) ';'
    ast call = get_do_call(t);
    write_to_output(wspace + "do ");

    switch (ast_node_kind(call))
    {
    case ast_call_as_function:
        walk_call_as_function(call);
        break;
    case ast_call_as_method:
        walk_call_as_method(call);
        break;
    default:
        fatal_error(0, "Unexpected call kind");
        break;
    }

    // writing to output
    write_to_output(" ;\n");
}

// walk an ast return node, it has not fields
//
void walk_return(ast t)
{
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // writing to output
    write_to_output(wspace + "return " + ';' + '\n');
}

// walk an ast return expr node with a single field
// expr - an ast expr node
//
void walk_return_expr(ast t)
{
    string wspace;
    for (int i = 0; i < indentation * 4; i++)
        wspace += ' ';

    // writing to output
    write_to_output(wspace + "return ");

    ast expr = get_return_expr(t);
    walk_expr(expr);
    write_to_output(" ;\n");
}

// walk an ast param list node
// it is an ast vector of ast term and infix op nodes
// it must be of odd length > 0, ie 1, 3, 5, 7, ...
// all elements at even indices are an ast term node
// all elements at odd indices are an ast infix op
//
void walk_expr(ast t)
{
    int term_ops = size_of_expr(t);

    // walks the expression nodes
    for (int i = 0; i < term_ops; i++)
    {
        ast term_op = get_expr(t, i);
        if (i % 2 == 0)
        {
            walk_term(term_op);
        }
        else
        {
            walk_infix_op(term_op);
        }
    }
}

// walk an ast term node with a single field
// term - one of the following ast nodes:
//        int, string, bool, null, this, expr, unary op,
//        variable, array index, call as function or call as method
//
void walk_term(ast t)
{
    ast term = get_term_term(t);

    // if one of the term nodes are found
    switch (ast_node_kind(term))
    {
    case ast_int:
        walk_int(term);
        break;
    case ast_string:
        walk_string(term);
        break;
    case ast_bool:
        walk_bool(term);
        break;
    case ast_null:
        walk_null(term);
        break;
    case ast_this:
        walk_this(term);
        break;
    case ast_expr:
        write_to_output("(");
        walk_expr(term);
        write_to_output(")");
        break;
    case ast_unary_op:
        walk_unary_op(term);
        break;
    case ast_var:
        walk_var(term);
        break;
    case ast_array_index:
        walk_array_index(term);
        break;
    case ast_call_as_function:
        walk_call_as_function(term);
        break;
    case ast_call_as_method:
        walk_call_as_method(term);
        break;
    default:
        fatal_error(0, "Unexpected term kind");
        break;
    }
}

// walk an ast int node with a single field
// constant - an integer in the range -32,768 to 32,767
//
void walk_int(ast t)
{
    // walks the int node
    int _constant = get_int_constant(t);
    write_to_output(to_string(_constant));
}

// walk an ast string node with a single field
// constant - a string
//
void walk_string(ast t)
{
    // walks the string node
    string _constant = get_string_constant(t);
    write_to_output('\"' + _constant + '\"');
}

// walk an ast bool node with a single field
// constant - either true or false
//
void walk_bool(ast t)
{
    bool _constant = get_bool_t_or_f(t);
    string cond = "";

    // if constant is true
    if (_constant)
    {
        cond = "true";
    }
    else
    {
        cond = "false";
    }

    // writing to output
    write_to_output(cond);
}

// walk an ast null node, it has not fields
//
void walk_null(ast t)
{
    // walks the null node
    write_to_output("null");
}

// walk an ast this node, it has not fields
//
void walk_this(ast t)
{
    // walks the this node
    write_to_output("this");
}

// walk an ast unary op node with fields
// op   - a string, either "-" or "~"
// term - an ast term node
//        int, string, bool, null, this, expr, unary op,
//        variable, array index, call as function or call as method
//
void walk_unary_op(ast t)
{
    string uop = get_unary_op_op(t);
    ast term = get_unary_op_term(t);

    // output either one of the unary ops
    write_to_output(uop);
    walk_term(term);
}

// walk an ast variable node with fields
// name - a string
// type - a string - "int", "char", "boolean" or a class name
// segment - a string
// offset - an int
//
void walk_var(ast t)
{
    string name = get_var_name(t);
    string type = get_var_type(t);
    string segment = get_var_segment(t);
    int offset = get_var_offset(t);

    // output the name
    write_to_output(name);
}

// walk an ast array index node with fields
// var   - an ast variable node
// index - an ast expr node
//
void walk_array_index(ast t)
{
    ast var = get_array_index_var(t);
    ast index = get_array_index_index(t);

    // writing to output
    walk_var(var);
    write_to_output("[");
    walk_expr(index);
    write_to_output("]");
}

// walk an ast subr call as method with fields
// class name - a string
// call       - an ast subr call node
//
void walk_call_as_function(ast t)
{
    string class_name = get_call_as_function_class_name(t);
    ast subr_call = get_call_as_function_subr_call(t);
    string subr_name = get_subr_call_subr_name(subr_call);

    // write to function
    write_to_output(class_name + '.' + subr_name + '(');
    walk_subr_call(subr_call);
    write_to_output(")");
}

// walk an ast subr call as method with fields
// class name - a string
// var        - an ast variable or ast this node, hidden first parameter of the call
// call       - an ast subr call node
//
void walk_call_as_method(ast t)
{
    string class_name = get_call_as_method_class_name(t);
    ast var = get_call_as_method_var(t);
    ast subr_call = get_call_as_method_subr_call(t);
    string subr_name = get_subr_call_subr_name(subr_call);

    // if either node found
    switch (ast_node_kind(var))
    {
    case ast_this:
        break;
    case ast_var:
        walk_var(var);
        write_to_output(".");
        break;
    default:
        fatal_error(0, "Expected var or this");
        break;
    }

    // walking subr_call
    write_to_output(subr_name + '(');
    walk_subr_call(subr_call);
    write_to_output(")");
}

// walk an ast subr call node with fields
// name      - a string
// expr list - a vector of ast expr nodes
//
void walk_subr_call(ast t)
{
    string subr_name = get_subr_call_subr_name(t);
    ast expr_list = get_subr_call_expr_list(t);

    // walk the subr_call node
    walk_expr_list(expr_list);
}

// walk an ast expr list node
// it is an ast vector of ast expr nodes
//
void walk_expr_list(ast t)
{
    int nexpressions = size_of_expr_list(t);

    // walk the expr list node
    for (int i = 0; i < nexpressions; i++)
    {
        walk_expr(get_expr_list(t, i));

        if (i != nexpressions - 1)
        {
            write_to_output(",");
        }
    }
}

// walk an ast infix op node with a single field
// op - a string - one of "+", "-", "*", "/", "&", "|", "<", ">", "="
//
void walk_infix_op(ast t)
{
    string op = get_infix_op_op(t);

    // writing to output of one of the infix ops
    write_to_output(' ' + op + ' ');
}

// main program
int main(int argc, char **argv)
{
    // walk an AST parsed from XML and pretty print equivalent Jack code
    walk_class(ast_parse_xml());

    // flush the output and any errors
    print_output();
    print_errors();
}
