// Brian Koh Lit Yang
// a1782291

#include "tokeniser-extras.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// to shorten the code
using namespace std ;

/////////////////////////////////////////////////////////////////////////

namespace Assignment_Tokeniser
{
    // the current input character
    // the eof marker can be confused with a legal character but not one accepted by our tokeniser
    char ch ;

    // to create a new token we need the characters read since the last token was created
    // the tokeniser always reads one character past a token so ch is never part of a new token
    // when nextch() is called the old ch value must be part of the token currently being parsed
    // this minimal implementation just remembers the spelling of the current token
    static string spelling ;

    // keeps track of the current and starting rows of tokens
    // helps with token_context() output
    static int currentColumn = 0;
    static int currentRow = 0;

    static int startColumn = 1;
    static int startRow = 1;

    static char tempstr = EOF;

    vector<string> spelling_history;
    int line_index=1;
    int column_index=1;
    
    // create a new token using characters tempstr since the last token was created
    // in the final submission tests new_token() will require the correct line and column numbers
    // this will require keeping an entire history of the input, not just the latest spelling
    // NOTE: the first character in the input is in column 1 of line 1
    Token new_token(TokenKind kind)
    {
        // create a new token object and reset spelling
        Token token = new_token(kind,spelling,startRow,startColumn) ;

        //cout << "tempstr: " << tempstr << endl;
        //cout << "spelling: " << spelling << endl;

        // for (int i = 0; i < spelling.length(); i++)
        // {
        //     if (spelling[i] == '\n')
        //     {
        //         line_index++;
            
        //         column_index = 1;
                
        //         spelling_history[spelling_history.size() - 1] += '$';

        //         spelling_history.push_back("");
        //     }
        //     else
        //     {
        //         column_index++;
        //         spelling_history[spelling_history.size() - 1] += spelling[i];
        //     }
        // }

        // reset the spelling
        spelling = "" ;

        startRow = currentRow;
        startColumn = currentColumn;

        return token ;
    }

    // generate a context string for the given token
    // all lines are shown after the preprocssing of special characters
    // it shows the line before the token, with a '$' marking the end of line,
    // the line containing the token up to the end of the token, and
    // a line of spaces ending with a ^ under the token's first character to mark its position
    // the lines are prefixed by line numbers, right justified in four spaces, followed by ": ", eg "  45: "
    // NOTE: if a token includes newline character(s), only display the token before the first newline followed by a '$'
    // NOTE: this function is not required for the milestone submission
    // NOTE: it is a good idea to build a data structure to remember where each line starts
    string token_context(Token token)
    {
        string content = "";
        string line_number;

        //The code inside this if-statement ensures that the lines are formatted correctly when there is more than 1 line
        if(token_line(token) > 1)
        {
            //First line number
            line_number = std::to_string(token_line(token) - 1);

            //Prepend spaces before the line number
            do
            {
                line_number = " " + line_number;
            } while (line_number.length() < 4);

            //#: abcdef/n
            content = line_number + ": " + spelling_history[token_line(token) - 2] + "\n";
        }

        //Second line number if there has already been one, else this would be the first line number
        line_number = std::to_string(token_line(token));

        //Prepend spaces before the line number
        do
        {
            line_number = " " + line_number;
        } while (line_number.length() < 4);

        //Ensures that the line is to the correct index of the spelling
        content = content + line_number + ": " + spelling_history[token_line(token) - 1].substr(0, token_column(token) - 1 + token_original(token).length()) + "\n";

        //Prepend spaces to ensure that the up arrow is pointing to the correct character
        for (int i = 0; i < 5+token_column(token); i++)
        {
            content += " ";
        }

        //Add in the arrow that will point to the correct character
        content += "^\n";

        return content;
    }

    // read next character if not at the end of input and update the line and column numbers
    // additional code will be required here to handle preprocessing of '\t' and '\r'
    // in some cases you may wish to remember a character to use next time instead of calling read_char()

    int tab_space =0;
    void nextch()
    {
        if ( ch == EOF ) return ;

        //converts a tab character to the appropriate amount of spaces
        if (tab_space > 0)
        {
            spelling = " ";
            ch = ' ';
            tab_space -= 1;
        } 
        else if (ch == '\t') 
        {
            tab_space = 2;
            spelling = " ";
        }
        else if(ch == '\r')
        {
            ch = read_char();
        }
        else
        {
            // remember the old ch, it is part of the current token being parsed
            // read the next character
            spelling += ch ;
            ch = read_char() ;
        }

        if (ch == '\n'){
            currentRow++;
            currentColumn=0;
        }

        if (tempstr == EOF){                // checks for EOF
            ch = read_char();
        }
        else {
            ch = tempstr;
            tempstr = EOF;
        }

        if (ch == '\t'){                            // if \t is found
            if ((4 - currentColumn % 4) > 1){
                tempstr = '\t';
            }
            ch = ' ';
        }
        else if (ch == '\r')                        // if '\r' is found
        {
            ch = read_char();
            if ( ch != '\n')
            {
                tempstr = ch;
                ch = '\n';
            }
        }
        currentColumn++;
        //ch = read_char() ;                  // read the next character
    }

    // initialise the tokeniser
    void initialise_tokeniser()
    {
                                            // add any other initialisation code you need here
                                            // ...
        ch = '\n' ;                         // initialise ch to avoid accidents
        nextch() ;                          // make first call to nextch to initialise ch using the input
        spelling = "" ;                     // discard the initial '\n', it is not part of the input
    }
}
