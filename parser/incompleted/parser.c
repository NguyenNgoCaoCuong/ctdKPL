/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>
#include <stdio.h>
#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

Token *currentToken;
Token *lookAhead;

void scan(void) {
    Token* tmp = currentToken;
    currentToken = lookAhead;
    lookAhead = getValidToken();
    free(tmp);
}

void eat(TokenType tokenType) {
    if (lookAhead->tokenType == tokenType) {
        printToken(lookAhead);
        scan();
    } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
    assert("Parsing a Program ....");
    eat(KW_PROGRAM);
    eat(TK_IDENT);
    eat(SB_SEMICOLON);
    compileBlock();
    eat(SB_PERIOD);
    assert("Program parsed!");
}

void compileBlock(void) {
    assert("Parsing a Block ....");
    if (lookAhead->tokenType == KW_CONST) {
        eat(KW_CONST);
        compileConstDecl();
        compileConstDecls();
        compileBlock2();
    }
    else compileBlock2();
    assert("Block parsed!");
}

void compileBlock2(void) {
    if (lookAhead->tokenType == KW_TYPE) {
        eat(KW_TYPE);
        compileTypeDecl();
        compileTypeDecls();
        compileBlock3();
    }
    else compileBlock3();
}

void compileBlock3(void) {
    if (lookAhead->tokenType == KW_VAR) {
        eat(KW_VAR);
        compileVarDecl();
        compileVarDecls();
        compileBlock4();
    }
    else compileBlock4();
}

void compileBlock4(void) {
    compileSubDecls();
    compileBlock5();
}

void compileBlock5(void) {
    eat(KW_BEGIN);
    compileStatements();
    eat(KW_END);
}

void compileConstDecls(void) {
    assert("Parsing subconstants...");
    while (lookAhead->tokenType == TK_IDENT)
        compileConstDecl();
    assert("Subconstants parsed!");
}

void compileConstDecl(void) {
    assert("Parsing constant...");
    eat(TK_IDENT);
    eat(SB_EQ);
    compileConstant();
    eat(SB_SEMICOLON);
    assert("Constant parsed!");
}

void compileTypeDecls(void) {
    assert("Parsing subconstants...");
    while (lookAhead->tokenType == TK_IDENT)
        compileConstDecl();
    assert("Subconstants parsed!");
}

void compileTypeDecl(void) {
    assert("Parsing constant...");
    eat(TK_IDENT);
    eat(SB_EQ);
    compileType();
    eat(SB_SEMICOLON);
    assert("Constant parsed!");
}

void compileVarDecls(void) {
    while(lookAhead->tokenType == TK_IDENT)
        compileVarDecl();
}

void compileVarDecl(void) {
    eat(TK_IDENT);
    eat(SB_COLON);
    compileType();
    eat(SB_SEMICOLON);
}

void compileSubDecls(void) {
    assert("Parsing subtoutines ....");
    if(lookAhead->tokenType == KW_FUNCTION){
        compileFuncDecl();
        compileSubDecls();
    } else if(lookAhead->tokenType == KW_PROCEDURE){
        compileProcDecl();
        compileSubDecls();
    }
    assert("Subtoutines parsed ....");
}

void compileFuncDecl(void) {
    assert("Parsing a function ....");
    eat(KW_FUNCTION);
    eat(TK_IDENT);
    compileParams();
    eat(SB_COLON);
    compileBasicType();
    eat(SB_SEMICOLON);
    compileBlock();
    eat(SB_SEMICOLON);
    assert("Function parsed ....");
}

void compileProcDecl(void) {
    assert("Parsing a procedure ....");
    eat(KW_PROCEDURE);
    eat(TK_IDENT);
    compileParams();
    eat(SB_SEMICOLON);
    compileBlock();
    eat(SB_SEMICOLON);
    assert("Procedure parsed ....");
}

void compileUnsignedConstant(void) {
    switch (lookAhead->tokenType) {
        case TK_NUMBER:
            eat(TK_NUMBER);
            break;
        case TK_IDENT:
            eat(TK_IDENT);
            break;
        case TK_CHAR:
            eat(TK_CHAR);
            break;
        default:
            error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileConstant(void) {
    switch(lookAhead->tokenType){
        case SB_PLUS:
            eat(SB_PLUS);
            compileConstant2();
            break;
        case SB_MINUS:
            eat(SB_MINUS);
            compileConstant2();
            break;
        case TK_CHAR:
            eat(TK_CHAR);
            break;
        default:
            compileConstant2();
            break;
    }
}

void compileConstant2(void) {
    switch (lookAhead->tokenType) {
        case TK_IDENT:
            eat(TK_IDENT);
            break;
        case TK_NUMBER:
            eat(TK_NUMBER);
            break;
        default:
            error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileType(void) {
    switch (lookAhead->tokenType) {
        case KW_INTEGER:
            eat(KW_INTEGER);
            break;
        case KW_CHAR:
            eat(KW_CHAR);
            break;
        case TK_IDENT:
            eat(TK_IDENT);
            break;
        case KW_ARRAY:
            eat(KW_ARRAY);
            eat(SB_LSEL);
            eat(TK_NUMBER);
            eat(SB_RSEL);
            eat(KW_OF);
            compileType();
            break;
        default:
            error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileBasicType(void) {
    switch (lookAhead->tokenType) {
        case KW_INTEGER:
            eat(KW_INTEGER);
            break;
        case KW_CHAR:
            eat(KW_CHAR);
            break;
        default:
            error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileParams(void) {

    switch (lookAhead->tokenType) {
        case SB_LPAR:
            eat(SB_LPAR);
            compileParam();
            compileParams2();
            eat(SB_RPAR);
            break;
        case SB_COLON:
        case SB_SEMICOLON:
            break;
        default:

            error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileParams2(void) {
    switch (lookAhead->tokenType) {
        case SB_SEMICOLON:
            eat(SB_SEMICOLON);
            compileParam();
            compileParams2();
            break;
        case SB_RPAR:
            break;
        default:
            error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileParam(void) {
    switch (lookAhead->tokenType) {
        case TK_IDENT:
            eat(TK_IDENT);
            eat(SB_COLON);
            compileBasicType();
            break;
        case KW_VAR:
            eat(KW_VAR);
            eat(TK_IDENT);
            eat(SB_COLON);
            compileBasicType();
            break;
        default:
            error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileStatements(void) {
    compileStatement();
    compileStatements2();
}

void compileStatements2(void) {
    switch (lookAhead->tokenType) {
        case SB_SEMICOLON:
            eat(SB_SEMICOLON);
            compileStatement();
            compileStatements2();
            break;
        case KW_END:
            break;
        default:
            error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileStatement(void) {
    switch (lookAhead->tokenType) {
        case TK_IDENT:
            compileAssignSt();
            break;
        case KW_CALL:
            compileCallSt();
            break;
        case KW_BEGIN:
            compileGroupSt();
            break;
        case KW_IF:
            compileIfSt();
            break;
        case KW_WHILE:
            compileWhileSt();
            break;
        case KW_FOR:
            compileForSt();
            break;
            // EmptySt needs to check FOLLOW tokens
        case SB_SEMICOLON:
        case KW_END:
        case KW_ELSE:
            break;
            // Error occurs
        default:
            error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileAssignSt(void) {
    assert("Parsing an assign statement ....");
    eat(TK_IDENT);
    if (lookAhead->tokenType == SB_LSEL) {
        compileIndexes();
    }
    eat(SB_ASSIGN);
    compileExpression();
    assert("Assign statement parsed ....");
}

void compileCallSt(void) {
    assert("Parsing a call statement ....");
    eat(KW_CALL);
    eat(TK_IDENT);
    compileArguments();
    assert("Call statement parsed ....");
}

void compileGroupSt(void) {
    assert("Parsing a group statement ....");
    eat(KW_BEGIN);
    compileStatements();
    eat(KW_END);
    assert("Group statement parsed ....");
}

void compileIfSt(void) {
    assert("Parsing an if statement ....");
    eat(KW_IF);
    compileCondition();
    eat(KW_THEN);
    compileStatement();
    if (lookAhead->tokenType == KW_ELSE)
        compileElseSt();
    assert("If statement parsed ....");
}

void compileElseSt(void) {
    eat(KW_ELSE);
    compileStatement();
}

void compileWhileSt(void) {
    assert("Parsing a while statement ....");
    eat(KW_WHILE);
    compileCondition();
    eat(KW_DO);
    compileStatement();
    assert("While statement pased ....");
}

void compileForSt(void) {
    assert("Parsing a for statement ....");
    eat(KW_FOR);
    eat(TK_IDENT);
    eat(SB_ASSIGN);
    compileExpression();
    eat(KW_TO);
    compileExpression();
    eat(KW_DO);
    compileStatement();
    assert("For statement parsed ....");
}

// void compileArguments(void) {
//   switch (lookAhead->tokenType) {
//   // case SB_LPAR:
//   //     eat(SB_LPAR);
//   //     compileExpression();
//   //     compileArguments2();
//   //     eat(SB_RPAR);
//   //     break;

//   // Follow - same as call statement as statement:

//   case SB_SEMICOLON:
//   case KW_END:
//   case KW_ELSE:
//   // Follow - term2
//   case SB_TIMES:
//   case SB_SLASH:
//   // Follow - expression3
//   // Follow (For statement)
//   case KW_TO:
//   case KW_DO:
//   // Follow (arguments2)
//   case SB_COMMA:
//   // Follow (condition2)
//   case SB_EQ:
//   case SB_NEQ:
//   case SB_LE:
//   case SB_LT:
//   case SB_GE:
//   case SB_GT:
//   // Follow (factor)
//   case SB_RPAR:
//   // Follow (indexes)
//   case SB_RSEL:
//   // Follow (if statement)
//   case KW_THEN:
//       break;
//   // Error
//   default:
//       error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
//       break;
//   }
//   switch(lookAhead->token == tokenType){
//     case SB_PLUS:
//     case SB_MINUS:
//     // factor
//     case TK_NUMBER:
//     case KW_CONST:
//     case KW_CONST:
//     // variable
//     case variable:
//   }
// }

void compileArguments(void) {
    printf("errrrrrr compileArgu\n");
    switch (lookAhead->tokenType) {
        case SB_MINUS:
        case SB_PLUS:
        case TK_NUMBER:
        case TK_CHAR:
        case TK_IDENT:
        case SB_LPAR:
            compileExpression();
            compileArguments2();
            break;
        case SB_SEMICOLON: //58 46 47
        case KW_END: //58 50 46(48) 59
        case SB_TIMES://91 88 83
        case SB_SLASH://91 99 94
        case KW_TO://91 88  82(85) 78(81) 75  64
        case KW_DO://91 88  82(85) 78(81) 75  64
        case SB_COMMA://91 88  82(85) 78(81) 75  67
        case SB_EQ://91 88  82(85) 78(81) 75  68 69
        case SB_NEQ://91 88  82(85) 78(81) 75  68 70
        case SB_LE://91 88  82(85) 78(81) 75  68 71
        case SB_LT://91 88  82(85) 78(81) 75  68 72
        case SB_GE://91 88  82(85) 78(81) 75  68 73
        case SB_GT://91 88  82(85) 78(81) 75  68 74
        case SB_RPAR://91 88 89
        case SB_ASSIGN://91 88 87  90(93) 56
        case KW_THEN://91 88 82(85) 78(81) 75 69 68 60
            return;
        default:
            error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileArguments2(void) {
    printf("erooooooooooooooo\n");
    switch (lookAhead->tokenType) {
        case SB_COMMA:
            eat(SB_COMMA);
            compileExpression();
            compileArguments2();
            break;
        case SB_SEMICOLON: //58 46 47
        case KW_END: //58 50 46(48) 59
        case SB_TIMES://91 88 83
        case SB_SLASH://91 99 94
        case KW_TO://91 88  82(85) 78(81) 75  64
        case KW_DO://91 88  82(85) 78(81) 75  64
        case SB_EQ://91 88  82(85) 78(81) 75  68 69
        case SB_NEQ://91 88  82(85) 78(81) 75  68 70
        case SB_LE://91 88  82(85) 78(81) 75  68 71
        case SB_LT://91 88  82(85) 78(81) 75  68 72
        case SB_GE://91 88  82(85) 78(81) 75  68 73
        case SB_GT://91 88  82(85) 78(81) 75  68 74
        case SB_RPAR://91 88 89
        case SB_ASSIGN://91 88 87  90(93) 56
        case KW_THEN://91 88 82(85) 78(81) 75 69 68 60
            break;
        default:
            error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
    }
}

// void compileArguments2(void) {
//   switch (lookAhead->tokenType) {
//   case SB_COMMA:
//       eat(SB_COMMA);
//       compileExpression();
//       compileArguments2();
//       break;
//   // Follow
//   case SB_RPAR:
//       break;
//   // Error:
//   default:
//       error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
//       break;
//   }
// }

void compileCondition(void) {
    compileExpression();
    compileCondition2();
}

void compileCondition2(void) {
    switch (lookAhead->tokenType) {
        case SB_EQ:
            eat(SB_EQ);
            compileExpression();
            break;
        case SB_NEQ:
            eat(SB_NEQ);
            compileExpression();
            break;
        case SB_LE:
            eat(SB_LE);
            compileExpression();
            break;
        case SB_LT:
            eat(SB_LT);
            compileExpression();
            break;
        case SB_GE:
            eat(SB_GE);
            compileExpression();
            break;
        case SB_GT:
            eat(SB_GT);
            compileExpression();
            break;
        default:
            error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileExpression(void) {
    assert("Parsing an expression");
    switch (lookAhead->tokenType) {
        case SB_PLUS:
            eat(SB_PLUS);
            compileExpression2();
            break;
        case SB_MINUS:
            eat(SB_MINUS);
            compileExpression2();
            break;
        default:
            compileExpression2();
            break;
    }
    assert("Expression parsed");
}

void compileExpression2(void) {
    compileTerm();
    compileExpression3();
}


void compileExpression3(void) {
    switch(lookAhead->tokenType) {
        case SB_PLUS:
            eat(SB_PLUS);
            compileTerm();
            compileExpression3();
            break;
        case SB_MINUS:
            eat(SB_MINUS);
            compileTerm();
            compileExpression3();
            break;
            // Follow (statement)
        case SB_SEMICOLON:
        case KW_END:
        case KW_ELSE:
            // Follow (For statement)
        case KW_TO:
        case KW_DO:
            // Follow (arguments2)
        case SB_COMMA:
            // Follow (condition2)
        case SB_EQ:
        case SB_NEQ:
        case SB_LE:
        case SB_LT:
        case SB_GE:
        case SB_GT:
            // Follow (factor)
        case SB_RPAR:
            // Follow (indexes)
        case SB_RSEL:
            // Follow (if statement)
        case KW_THEN:
            break;
            // Error
        default:
            error(ERR_INVALIDEXPRESSION, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileTerm(void) {
    compileFactor();
    compileTerm2();
}

void compileTerm2(void) {
    switch (lookAhead->tokenType) {
        case SB_TIMES:
            eat(SB_TIMES);
            compileFactor();
            compileTerm2();
            break;
        case SB_SLASH:
            eat(SB_SLASH);
            compileFactor();
            compileTerm2();
            break;
            // Follow - same as expression3
        case SB_PLUS:
        case SB_MINUS:
            // Follow (statement)
        case SB_SEMICOLON:
        case KW_END:
        case KW_ELSE:
            // Follow (For statement)
        case KW_TO:
        case KW_DO:
            // Follow (arguments2)
        case SB_COMMA:
            // Follow (condition2)
        case SB_EQ:
        case SB_NEQ:
        case SB_LE:
        case SB_LT:
        case SB_GE:
        case SB_GT:
            // Follow (factor)
        case SB_RPAR:
            // Follow (indexes)
        case SB_RSEL:
            // Follow (if statement)
        case KW_THEN:
            break;
        default:
            error(ERR_INVALIDTERM, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileFactor(void) {
    switch (lookAhead->tokenType) {
        case TK_NUMBER:
        case TK_CHAR:
            compileUnsignedConstant();
            break;
        case SB_LPAR:
            eat(SB_LPAR);
            compileExpression();
            eat(SB_RPAR);
            break;
        case TK_IDENT:
            eat(TK_IDENT);
            switch(lookAhead->tokenType) {
                case SB_LSEL:
                    compileIndexes();
                    break;
                case SB_LPAR:
                    compileArguments();
                    break;
                default:
                    break;
            }
            break;
        default:
            error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileIndexes(void) {
    if (lookAhead->tokenType == SB_LSEL) {
        eat(SB_LSEL);
        compileExpression();
        eat(SB_RSEL);
        compileIndexes();
    }
}

int compile(char *fileName) {
    if (openInputStream(fileName) == IO_ERROR)
        return IO_ERROR;

    currentToken = NULL;
    lookAhead = getValidToken();

    compileProgram();

    free(currentToken);
    free(lookAhead);
    closeInputStream();
    return IO_SUCCESS;

}