// Brian Koh Lit Yang
// a1782291

#include "tokeniser-extras.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
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
    static string temp_spel;

    // keeps track of the current and starting rows of tokens
    // helps with token_context() output
    static int currentColumn = 0;
    static int currentRow = 0;

    static int startColumn = 1;
    static int startRow = 1;

    static char tempstr = EOF;

    vector<string> contxt;
    vector<int> contxtrow;
    vector<int> contxtcol;

    vector<int> line_control;
    int x=0;
    int idx=0;

    vector<int> rows_per_col;
    int numofcols_perline=0;

    vector<int> tokenindexes;
    int tok_idx = 0;
    
    // create a new token using characters tempstr since the last token was created
    // in the final submission tests new_token() will require the correct line and column numbers
    // this will require keeping an entire history of the input, not just the latest spelling
    // NOTE: the first character in the input is in column 1 of line 1
    Token new_token(TokenKind kind)
    {
        // create a new token object and reset spelling
        Token token = new_token(kind,spelling,startRow,startColumn) ;

        contxt.push_back(token_original(token));
        idx++;
        tok_idx++;
        numofcols_perline++;

        if(token_original(token) == "\n"){
            line_control.push_back(idx);
            rows_per_col.push_back(numofcols_perline);
            numofcols_perline = 0;
        }
        // reset the spelling
        spelling = "" ;

        startRow = currentRow;
        startColumn = currentColumn;

        contxtrow.push_back(startRow);
        contxtcol.push_back(startColumn);

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
    
    int i=0;
    int tab_spacing = 8;
    int check = 0;
    int check1 = 0;
    
    string token_context(Token token)
    {
        int token_offset = token_column(token);
        int tokenidx = 0;
        int row = token_line(token);
        int cur_row = row-1;
        int prev_row = row-2;

        string print_token; 
        string return_string;
        string cur_line = std::to_string(row);
        string prev_line = std::to_string(row-1);
        
        if (check == 0){                                                                // append the leading 0 for the token line
            tokenindexes.push_back(0);
            //reverse(tokenindexes.begin(), tokenindexes.end());
            check++;
        } else {
            check++;
        }

        if(check1 == 0){                                                                // push back the token line values
            for ( int i = idx - 1 ; i > 0 ; i -= 5 )
            {
                tokenindexes.push_back(i);
            }
            check1++;
        } else{
            check1++;
        }

        // for(int j=0; j < tokenindexes.size(); j++){
        //     cout << "token-context " << j << ": " << tokenindexes.at(j) << endl; 
        // }
        //cout << "token-context " << check << ": " << tokenindexes.at(check-1) << endl; 
        tokenidx = tokenindexes.at(check-1);

        //cout << "tokenidx: " << tokenidx << endl;

        if (row < 2){
            return_string += "   ";
            return_string += cur_line + ": ";
            
            for(i = line_control[prev_row]; i < line_control[cur_row]; i++){            // for loop from the start of row to end of row
                
                if (tokenidx == 0){                                                      
                    return_string += " ";
                }
                if (i == tokenidx){                                                 // if reached tokenidx break for loop
                    break;
                }
                if (contxt[i] != "\n"){
                    return_string += contxt[i];
                }
                else if (contxt[i] == "\n"){
                    return_string += "$";                                               // if new line found print "$"
                    break;
                }
            }
            return_string += "\n";                                                      // end of line 2,
        } 
        else // if (more than 2 rows in tokens)
        {
            // FIRST LINE
            return_string += "   ";
            return_string += prev_line + ": ";                                          // current line being displayed

            for(i = line_control[prev_row-1]; i < line_control[cur_row-1]; i++){        // print tokens
                if (contxt[i] != "\n"){
                    return_string += contxt[i];
                }
                else if (contxt[i] == "\n"){
                    return_string += "$";                                               // if new line found print "$"
                    break;
                }
            } 

            return_string += "\n";                                                      // end of line 1,

            //SECOND LINE
            return_string += "   ";
            return_string += cur_line + ": ";

            for(i = line_control[prev_row]; i < line_control[cur_row]; i++){
                
                if (i == tokenidx)
                    break;

                if (contxt[i] != "\n"){
                    return_string += contxt[i];
                }
                else if (contxt[i] == "\n"){
                    return_string += "$";                                               // if new line found print "$"
                    break;
                }
            }

            return_string += "\n";                                                      // end of line 2,
        }

        // add indentation
        return_string += "      ";

        // add spaces
        for(i = 1; i < token_offset; i++){
            return_string += " ";
        }

        // add the arrow
        return_string += "^";

        // newline
        return_string += "\n";

        // for (int j=0; j < contxt.size(); j++){
        //     cout << "token " << j << ": " << contxt[j] << endl;
        // }
        return return_string;
    }

    // read next character if not at the end of input and update the line and column numbers
    // additional code will be required here to handle preprocessing of '\t' and '\r'
    // in some cases you may wish to remember a character to use next time instead of calling read_char()
    void nextch()
    {
        if ( ch == EOF ) return ;           // stop reading once we have read EOF

        temp_spel += ch;
        //contxt.push_back(spelling);

        if (temp_spel == " "){
            temp_spel = "";
        }

        if (ch == '\n'){
            currentRow++;
            currentColumn=0;
        }

        spelling += ch ;                    // remember the old ch, it is part of the current token being parsed
        

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

        // ch = read_char() ;                  // read the next character
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
