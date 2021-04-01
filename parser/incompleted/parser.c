/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include  <stdlib.h>
#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include <stdio.h>
Token *currentToken;
Token *lookAhead;

void scan(void) {
    Token *tmp = currentToken;
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
    } else compileBlock2();
    assert("Block parsed!");
}

void compileBlock2(void) {
    if (lookAhead->tokenType == KW_TYPE) {
        eat(KW_TYPE);
        compileTypeDecl();
        compileTypeDecls();
        compileBlock3();
    } else compileBlock3();
}

void compileBlock3(void) {
    if (lookAhead->tokenType == KW_VAR) {
        eat(KW_VAR);
        compileVarDecl();
        compileVarDecls();
        compileBlock4();
    } else compileBlock4();
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
    while (lookAhead->tokenType == TK_IDENT)
        compileConstDecl();
}

void compileConstDecl(void) {
    eat(TK_IDENT);
    eat(SB_EQ);
    compileConstant();
    eat(SB_SEMICOLON);
}

void compileTypeDecls(void) {
    while (lookAhead->tokenType == TK_IDENT)
        compileTypeDecl();
}

void compileTypeDecl(void) {
    eat(TK_IDENT);
    eat(SB_EQ);
    compileType();
    eat(SB_SEMICOLON);
}

void compileVarDecls(void) {
    while (lookAhead->tokenType == TK_IDENT)
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
    if(lookAhead->tokenType == KW_PROCEDURE){
        compileProcDecl();
        compileSubDecls();
    }
    if(lookAhead->tokenType == KW_FUNCTION){
        compileFuncDecl();
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
    if (lookAhead->tokenType == TK_NUMBER)
        eat(TK_NUMBER);
    else if (lookAhead->tokenType == TK_IDENT)
        eat(TK_IDENT);
    else if (lookAhead->tokenType == TK_CHAR)
        eat(TK_CHAR);
    else error(ERR_INVALIDCHARCONSTANT, lookAhead->lineNo, lookAhead->colNo);
}

void compileConstant(void) {
    if (lookAhead->tokenType == SB_PLUS) {
        eat(SB_PLUS);
        compileConstant2();
    } else if (lookAhead->tokenType == SB_MINUS) {
        eat(SB_MINUS);
        compileConstant2();
    } else if (lookAhead->tokenType == TK_CHAR) {
        eat(TK_CHAR);
    } else {
        compileConstant2();
    }
}

void compileConstant2(void) {
    if (lookAhead->tokenType == TK_IDENT){
        printf("TK_ID\n");
        eat(TK_IDENT);
    }
    else if (lookAhead->tokenType == TK_NUMBER) {
        printf("TK_NB\n");
        eat(TK_NUMBER);
    } else{

        error(ERR_INVALIDCHARCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileType(void) {
    if (lookAhead->tokenType == KW_INTEGER)
        eat(KW_INTEGER);
    else if (lookAhead->tokenType == KW_CHAR)
        eat(KW_CHAR);
    else if (lookAhead->tokenType == TK_IDENT)
        eat(TK_IDENT);
    else if (lookAhead->tokenType == KW_ARRAY) {
        eat(KW_ARRAY);
        eat(SB_LSEL);
        eat(TK_NUMBER);
        eat(SB_RSEL);
        eat(KW_OF);
        compileType();
    } else error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
}

void compileBasicType(void) {
    if (lookAhead->tokenType == KW_INTEGER)
        eat(KW_INTEGER);
    else if (lookAhead->tokenType == KW_CHAR)
        eat(KW_CHAR);
    else error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
}

void compileParams(void) {
    if (lookAhead->tokenType == SB_LPAR) {
        eat(SB_LPAR);
        compileParam();
        compileParams2();
        eat(SB_RPAR);
        return;
    } else if (lookAhead->tokenType == SB_COLON || lookAhead->tokenType == SB_SEMICOLON) return;//Luat 22 23
    error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);

}

void compileParams2(void) {
   if(lookAhead->tokenType == SB_SEMICOLON){
       eat(SB_SEMICOLON);
       compileParam();
       compileParams2();
       return;
   }else if(lookAhead->tokenType == SB_RPAR) return; //24
   error(ERR_INVALIDPARAM,lookAhead->lineNo,lookAhead->colNo);
}

void compileParam(void) {
    if (lookAhead->tokenType == TK_IDENT) {
        eat(TK_IDENT);
        eat(SB_COLON);
        compileBasicType();
    } else if (lookAhead->tokenType == KW_VAR) {
        eat(KW_VAR);
        eat(TK_IDENT);
        eat(SB_COLON);
        compileBasicType();
    } else error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
}

void compileStatements(void) {
    compileStatement();
    compileStatements2();
}

void compileStatements2(void) {
   if(lookAhead->tokenType == SB_SEMICOLON){
       eat(SB_SEMICOLON);
       compileStatements();
       compileStatements2();
       return;
   }else if(lookAhead->tokenType == KW_END) //59
       return;
   error(ERR_INVALIDSTATEMENT,lookAhead->lineNo,lookAhead->colNo);
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
        case SB_SEMICOLON:// 47
        case KW_END://59
        case KW_ELSE://60
            break;
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
    compileElseSt();
    assert("If statement parsed ....");
}

void compileElseSt(void) {
    if (lookAhead->tokenType == KW_ELSE) {
        eat(KW_ELSE);
        compileStatement();
    }
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

void compileArguments(void) {
    switch (lookAhead->tokenType) {
        case SB_LPAR:
            eat(SB_LPAR);
            compileExpression();
            compileArguments2();
            eat(SB_RPAR);
            break;
        case SB_SEMICOLON: //58 46 47
        case KW_END: //58 50 46(48) 59
        case SB_TIMES://91 88 83
        case SB_SLASH://91 99 94
        case KW_TO://91 88  82(85) 78(81) 75  64
        case KW_DO://91 88  82(85) 78(81) 75  64
        case SB_PLUS://91 88  82(85) 78
        case SB_MINUS://91 88  82(85) 78
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
            break;
        default:
            error(ERR_INVALIDARGUMENTS,lookAhead->lineNo,lookAhead->colNo);
    }

}

void compileArguments2(void) {
    if (lookAhead->tokenType == SB_COMMA) {
        eat(SB_COMMA);
        compileExpression();
        compileArguments2();
        return;
    } else if (lookAhead->tokenType == SB_RPAR) return;//67 65
    error(ERR_INVALIDARGUMENTS, lookAhead->lineNo, lookAhead->colNo);

}

void compileCondition(void) {
    compileExpression();
    compileCondition2();
}

void compileCondition2(void) {
    if (lookAhead->tokenType == SB_EQ) {
        eat(SB_EQ);
        compileExpression();
    } else if (lookAhead->tokenType == SB_NEQ) {
        eat(SB_NEQ);
        compileExpression();
    } else if (lookAhead->tokenType == SB_LE) {
        eat(SB_LE);
        compileExpression();
    } else if (lookAhead->tokenType == SB_GE) {
        eat(SB_GE);
        compileExpression();
    } else if (lookAhead->tokenType == SB_GT) {
        compileExpression();
        eat(SB_GT);
    } else error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
}

void compileExpression(void) {
    assert("Parsing an expression");
    if (lookAhead->tokenType == SB_PLUS)
        eat(SB_PLUS);
    else if (lookAhead->tokenType == SB_MINUS)
        eat(SB_MINUS);
    compileExpression2();

    assert("Expression parsed");
}

void compileExpression2(void) {
    compileTerm();
    compileExpression3();
}

void compileExpression3(void) {
    printf("Nguyen Ngo Cao Cuong\n");
    switch(lookAhead->tokenType){
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
        case KW_TO://79(81) 78 75 64
        case KW_DO://79(81) 78 75 64
        case SB_EQ://79(81) 78 75 68 69
        case SB_NEQ://79(81) 78 75 68 69 70
        case SB_LE://79(81) 78 75 68 69 71
        case SB_LT://79(81) 78 75 68 69 72
        case SB_GE://79(81) 78 75 68 69 73
        case SB_GT://79(81) 78 75 68 69 74
        case SB_COMMA://79(81) 78 75 67
        case SB_RPAR://79(81) 78 75 65(66)
        case SB_SEMICOLON://79(81) 82(85) ->Follow(Facto)
        case KW_END://79(81) 82(85) ->Follow(Facto)
        case KW_ELSE://79(81) 82(85) ->Follow(Facto)
        case KW_THEN://79(81) 82(85) ->Follow(Facto)
            break;
        default:
            error(ERR_INVALIDEXPRESSION,lookAhead->lineNo,lookAhead->colNo);
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
        case KW_TO: //82(85) -> Follow(Facto)
        case KW_DO: //82(85) -> Follow(Facto)
        case SB_EQ: //82(85) -> Follow(Facto)
        case SB_NEQ: //82(85) -> Follow(Facto)
        case SB_LE: //82(85) -> Follow(Facto)
        case SB_LT: //82(85) -> Follow(Facto)
        case SB_GE: //82(85) -> Follow(Facto)
        case SB_GT: //82(85) -> Follow(Facto)
        case SB_COMMA: //82(85) -> Follow(Facto)
        case SB_RPAR: //82(85) -> Follow(Facto)
        case SB_SEMICOLON: //82(85) -> Follow(Facto)
        case KW_END: //82(85) -> Follow(Facto)
        case KW_ELSE: //82(85) -> Follow(Facto)
        case KW_THEN: //82(85) -> Follow(Facto)
        case SB_PLUS: //82 79
        case SB_MINUS: //82 80
            break;
        default:
            error(ERR_INVALIDTERM,lookAhead->lineNo,lookAhead->colNo);
    }

}

void compileFactor(void) {

    switch (lookAhead->tokenType) {
        case SB_LPAR:
            eat(SB_LPAR);
            compileExpression();
            eat(SB_RPAR);
            break;
        case TK_IDENT:
            eat(TK_IDENT);
            if(lookAhead->tokenType == SB_LSEL){
                compileIndexes();
            }else{
                compileArguments();
            }
            break;
        case TK_CHAR:
            eat(TK_CHAR);
            break;
        case TK_NUMBER:
            eat(TK_NUMBER);
            break;
        default:
            error(ERR_INVALIDFACTOR,lookAhead->lineNo,lookAhead->colNo);
    }
}

void compileIndexes(void) {
    if(lookAhead->tokenType ==SB_LSEL){
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
