/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */
#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "semantics.h"
#include "error.h"
#include "debug.h"

Token *currentToken;
Token *lookAhead;

extern Type *intType;
extern Type *charType;
extern SymTab *symtab;

void scan(void) {
    Token *tmp = currentToken;
    currentToken = lookAhead;
    lookAhead = getValidToken();
    free(tmp);
}

void eat(TokenType tokenType) {
    if (lookAhead->tokenType == tokenType) {
        scan();
    } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
    Object *program;

    eat(KW_PROGRAM);
    eat(TK_IDENT);

    program = createProgramObject(currentToken->string);
    enterBlock(program->progAttrs->scope);

    eat(SB_SEMICOLON);

    compileBlock();
    eat(SB_PERIOD);

    exitBlock();
}

void compileBlock(void) {
    Object *constObj;
    ConstantValue *constValue;

    if (lookAhead->tokenType == KW_CONST) {
        eat(KW_CONST);

        do {
            eat(TK_IDENT);

            checkFreshIdent(currentToken->string);
            constObj = createConstantObject(currentToken->string);

            eat(SB_EQ);
            constValue = compileConstant();

            constObj->constAttrs->value = constValue;
            declareObject(constObj);

            eat(SB_SEMICOLON);
        } while (lookAhead->tokenType == TK_IDENT);

        compileBlock2();
    } else compileBlock2();
}

void compileBlock2(void) {
    Object *typeObj;
    Type *actualType;

    if (lookAhead->tokenType == KW_TYPE) {
        eat(KW_TYPE);

        do {
            eat(TK_IDENT);

            checkFreshIdent(currentToken->string);
            typeObj = createTypeObject(currentToken->string);

            eat(SB_EQ);
            actualType = compileType();

            typeObj->typeAttrs->actualType = actualType;
            declareObject(typeObj);

            eat(SB_SEMICOLON);
        } while (lookAhead->tokenType == TK_IDENT);

        compileBlock3();
    } else compileBlock3();
}

void compileBlock3(void) {
    Object *varObj;
    Type *varType;

    if (lookAhead->tokenType == KW_VAR) {
        eat(KW_VAR);

        do {
            eat(TK_IDENT);

            checkFreshIdent(currentToken->string);
            varObj = createVariableObject(currentToken->string);

            eat(SB_COLON);
            varType = compileType();

            varObj->varAttrs->type = varType;
            declareObject(varObj);

            eat(SB_SEMICOLON);
        } while (lookAhead->tokenType == TK_IDENT);

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

void compileSubDecls(void) {
    while ((lookAhead->tokenType == KW_FUNCTION) || (lookAhead->tokenType == KW_PROCEDURE)) {
        if (lookAhead->tokenType == KW_FUNCTION)
            compileFuncDecl();
        else compileProcDecl();
    }
}

void compileFuncDecl(void) {
    Object *funcObj;
    Type *returnType;

    eat(KW_FUNCTION);
    eat(TK_IDENT);

    checkFreshIdent(currentToken->string);
    funcObj = createFunctionObject(currentToken->string);
    declareObject(funcObj);

    enterBlock(funcObj->funcAttrs->scope);

    compileParams();

    eat(SB_COLON);
    returnType = compileBasicType();
    funcObj->funcAttrs->returnType = returnType;

    eat(SB_SEMICOLON);
    compileBlock();
    eat(SB_SEMICOLON);

    exitBlock();
}

void compileProcDecl(void) {
    Object *procObj;

    eat(KW_PROCEDURE);
    eat(TK_IDENT);

    checkFreshIdent(currentToken->string);
    procObj = createProcedureObject(currentToken->string);
    declareObject(procObj);

    enterBlock(procObj->procAttrs->scope);

    compileParams();

    eat(SB_SEMICOLON);
    compileBlock();
    eat(SB_SEMICOLON);

    exitBlock();
}

ConstantValue *compileUnsignedConstant(void) {
    ConstantValue *constValue;
    Object *obj;

    switch (lookAhead->tokenType) {
        case TK_NUMBER:
            eat(TK_NUMBER);
            constValue = makeIntConstant(currentToken->value);
            break;
        case TK_IDENT:
            eat(TK_IDENT);

            obj = checkDeclaredConstant(currentToken->string);
            constValue = duplicateConstantValue(obj->constAttrs->value);

            break;
        case TK_CHAR:
            eat(TK_CHAR);
            constValue = makeCharConstant(currentToken->string[0]);
            break;
        default:
            error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
    return constValue;
}

ConstantValue *compileConstant(void) {
    ConstantValue *constValue;

    switch (lookAhead->tokenType) {
        case SB_PLUS:
            eat(SB_PLUS);
            constValue = compileConstant2();
            break;
        case SB_MINUS:
            eat(SB_MINUS);
            constValue = compileConstant2();
            constValue->intValue = -constValue->intValue;
            break;
        case TK_CHAR:
            eat(TK_CHAR);
            constValue = makeCharConstant(currentToken->string[0]);
            break;
        default:
            constValue = compileConstant2();
            break;
    }
    return constValue;
}

ConstantValue *compileConstant2(void) {
    ConstantValue *constValue;
    Object *obj;

    switch (lookAhead->tokenType) {
        case TK_NUMBER:
            eat(TK_NUMBER);
            constValue = makeIntConstant(currentToken->value);
            break;
        case TK_IDENT:
            eat(TK_IDENT);
            obj = checkDeclaredConstant(currentToken->string);
            if (obj->constAttrs->value->type == TP_INT)
                constValue = duplicateConstantValue(obj->constAttrs->value);
            else
                error(ERR_UNDECLARED_INT_CONSTANT, currentToken->lineNo, currentToken->colNo);
            break;
        default:
            error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
    return constValue;
}

Type *compileType(void) {
    Type *type;
    Type *elementType;
    int arraySize;
    Object *obj;

    switch (lookAhead->tokenType) {
        case KW_INTEGER:
            eat(KW_INTEGER);
            type = makeIntType();
            break;
        case KW_CHAR:
            eat(KW_CHAR);
            type = makeCharType();
            break;
        case KW_STRING:
            eat(KW_STRING);
            return makeStringType();
        case KW_DOUBLE:
            eat(KW_DOUBLE);
            type = makeDoubleType();
            break;
        case KW_ARRAY:
            eat(KW_ARRAY);
            eat(SB_LSEL);
            eat(TK_NUMBER);

            arraySize = currentToken->value;

            eat(SB_RSEL);
            eat(KW_OF);
            elementType = compileType();
            type = makeArrayType(arraySize, elementType);
            break;
        case TK_IDENT:
            eat(TK_IDENT);
            obj = checkDeclaredType(currentToken->string);
            type = duplicateType(obj->typeAttrs->actualType);
            break;
        default:
            error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
    return type;
}

Type *compileBasicType(void) {
    Type *type;

    switch (lookAhead->tokenType) {
        case KW_INTEGER:
            eat(KW_INTEGER);
            type = makeIntType();
            break;
        case KW_CHAR:
            eat(KW_CHAR);
            type = makeCharType();
            break;
        default:
            error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
    return type;
}

void compileParams(void) {
    if (lookAhead->tokenType == SB_LPAR) {
        eat(SB_LPAR);
        compileParam();
        while (lookAhead->tokenType == SB_SEMICOLON) {
            eat(SB_SEMICOLON);
            compileParam();
        }
        eat(SB_RPAR);
    }
}

void compileParam(void) {
    Object *param;
    Type *type;
    enum ParamKind paramKind;

    switch (lookAhead->tokenType) {
        case TK_IDENT:
            paramKind = PARAM_VALUE;
            break;
        case KW_VAR:
            eat(KW_VAR);
            paramKind = PARAM_REFERENCE;
            break;
        default:
            error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
            break;
    }

    eat(TK_IDENT);
    checkFreshIdent(currentToken->string);
    param = createParameterObject(currentToken->string, paramKind, symtab->currentScope->owner);
    eat(SB_COLON);
    type = compileBasicType();
    param->paramAttrs->type = type;
    declareObject(param);
}

void compileStatements(void) {
    compileStatement();
    while (lookAhead->tokenType == SB_SEMICOLON) {
        eat(SB_SEMICOLON);
        compileStatement();
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
        case KW_SWITCH:
            compileSwitchSt();
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
        case TK_NUMBER:
            compileExpression();
            break;
            // EmptySt needs to check FOLLOW tokens
        case SB_SEMICOLON:
        case KW_END:
        case KW_ELSE:
        case KW_CASE:
            break;
            // Error occurs
        default:
            error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

Type *compileLValue(void) {
    Object *obj;
    Type *type = NULL;

    eat(TK_IDENT);
    // check if the identifier is a function identifier, or a variable identifier, or a parameter
    obj = checkDeclaredLValueIdent(currentToken->string);
    if (obj->kind == OBJ_VARIABLE) {
        if (obj->varAttrs->type->typeClass == TP_ARRAY) type = compileIndexes(obj->varAttrs->type);
        else type = obj->varAttrs->type;
    } else if (obj->kind == OBJ_FUNCTION)
        type = obj->funcAttrs->returnType;
    else type = obj->paramAttrs->type;
    return type;
}

void compileAssignSt(void) {

    Type *t1 = compileLValue();
    eat(SB_ASSIGN);
    checkTypeEquality(t1, compileExpression());
}

void compileCallSt(void) {
    Object *proc;

    eat(KW_CALL);
    eat(TK_IDENT);
    proc = checkDeclaredProcedure(currentToken->string);
    compileArguments(proc->procAttrs->paramList);
}

void compileSwitchSt(void) {
    eat(KW_SWITCH);
    compileExpression();
    eat(KW_BEGIN);
    while (lookAhead->tokenType == KW_CASE){
        eat(KW_CASE);
        compileConstant();
        eat(SB_COLON);
        compileStatements();
        if(lookAhead->tokenType == KW_BREAK)
            eat(KW_BREAK);
    }
    if(lookAhead->tokenType == KW_DEFAULT){
        eat(KW_DEFAULT);
        eat(SB_COLON);
        compileStatement();
    }
    eat(KW_END);
}

void compileGroupSt(void) {
    eat(KW_BEGIN);
    compileStatements();
    eat(KW_END);
}

void compileIfSt(void) {
    eat(KW_IF);
    compileCondition();
    eat(KW_THEN);
    compileStatement();
    if (lookAhead->tokenType == KW_ELSE)
        compileElseSt();
}

void compileElseSt(void) {
    eat(KW_ELSE);
    compileStatement();
}

void compileWhileSt(void) {
    eat(KW_WHILE);
    compileCondition();
    eat(KW_DO);
    compileStatement();
}

void compileForSt(void) {

    eat(KW_FOR);
    eat(TK_IDENT);

    // check if the identifier is a variable
    Object *var = checkDeclaredVariable(currentToken->string);
    //  checkBasicType(var->varAttrs->type);
    Type *t1 = var->varAttrs->type;
    eat(SB_ASSIGN);
    checkTypeEquality(t1, compileExpression());
    eat(KW_TO);
    checkTypeEquality(t1, compileExpression());
    eat(KW_DO);
    compileStatement();
}

void compileArgument(Object *param) {
    if (param->paramAttrs->kind == PARAM_REFERENCE) {
        if (lookAhead->tokenType == TK_IDENT) {
            checkDeclaredLValueIdent(lookAhead->string);
        } else {
            error(ERR_TYPE_INCONSISTENCY, lookAhead->lineNo, lookAhead->colNo);
        }
    }
    checkTypeEquality(compileExpression(), param->paramAttrs->type);
}

void compileArguments(ObjectNode *paramList) {
    //   printf("goi t %d\n",currentToken->lineNo);
    int t = 1;
    if (paramList != NULL && paramList->object != NULL) t = 2;
    switch (lookAhead->tokenType) {
        case SB_LPAR:
            if (paramList == NULL)
                error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
            eat(SB_LPAR);
            ObjectNode *param = paramList;
            compileArgument(param->object);
            while (lookAhead->tokenType == SB_COMMA) {
                eat(SB_COMMA);
                param = param->next;
                if (param == NULL)
                    error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
                else compileArgument(param->object);
            }

            if (param->next != NULL)
                error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
            eat(SB_RPAR);
            return;
            // Check FOLLOW set
        case SB_TIMES:
        case SB_SLASH:
        case SB_PLUS:
        case SB_MINUS:
        case KW_TO:
        case KW_DO:
        case SB_RPAR:
        case SB_COMMA:
        case SB_EQ:
        case SB_NEQ:
        case SB_LE:
        case SB_LT:
        case SB_GE:
        case SB_GT:
        case SB_RSEL:
        case SB_SEMICOLON:
        case KW_END:
        case KW_ELSE:
        case KW_THEN:
        case KW_CASE:
            if (t == 2) error(ERR_PARAMETERS_ARGUMENTS_INCONSISTENCY, currentToken->lineNo, currentToken->colNo);
            break;
        default:
            error(ERR_INVALID_ARGUMENTS, lookAhead->lineNo, lookAhead->colNo);
    }

}

void compileCondition(void) {
    Type *type = compileExpression();

    switch (lookAhead->tokenType) {
        case SB_EQ:
            eat(SB_EQ);
            break;
        case SB_NEQ:
            eat(SB_NEQ);
            break;
        case SB_LE:
            eat(SB_LE);
            break;
        case SB_LT:
            eat(SB_LT);
            break;
        case SB_GE:
            eat(SB_GE);
            break;
        case SB_GT:
            eat(SB_GT);
            break;
        default:
            error(ERR_INVALID_COMPARATOR, lookAhead->lineNo, lookAhead->colNo);
    }

    checkTypeEquality(compileExpression(), type);
}

Type *compileExpression(void) {
    Type *type;

    switch (lookAhead->tokenType) {
        case SB_PLUS:
            eat(SB_PLUS);
            type = compileExpression2();
            break;
        case SB_MINUS:
            eat(SB_MINUS);
            type = compileExpression2();
            break;
        default:
            type = compileExpression2();
    }
    return type;
}

Type *compileExpression2(void) {
    Type *type;

    type = compileTerm();
    compileExpression3(type);

    return type;
}


void compileExpression3(Type *t) {
    Type *type;

    switch (lookAhead->tokenType) {
        case SB_PLUS:
            eat(SB_PLUS);
            checkTypeEquality(t,compileTerm());
            compileExpression3(t);
            break;
        case SB_MINUS:
            eat(SB_MINUS);
            checkTypeEquality(t,compileTerm());
            compileExpression3(t);
            break;
            // check the FOLLOW set
        case KW_TO:
        case KW_DO:
        case SB_RPAR:
        case SB_COMMA:
        case SB_EQ:
        case SB_NEQ:
        case SB_LE:
        case SB_LT:
        case SB_GE:
        case SB_GT:
        case SB_RSEL:
        case SB_SEMICOLON:
        case KW_END:
        case KW_ELSE:
        case KW_THEN:
        case KW_CASE:
        case KW_BEGIN:
        case KW_DEFAULT:
        case KW_BREAK:
            break;
        default:
            error(ERR_INVALID_EXPRESSION, lookAhead->lineNo, lookAhead->colNo);
    }
}

Type *compileTerm(void) {
    Type *type;

    type = compileFactor();
    compileTerm2();

    return type;
}

void compileTerm2(void) {
    Type *type;

    switch (lookAhead->tokenType) {
        case SB_TIMES:
            eat(SB_TIMES);
            type = compileFactor();
            checkIntType(type);
            compileTerm2();
            break;
        case SB_SLASH:
            eat(SB_SLASH);
            type = compileFactor();
            checkIntType(type);
            compileTerm2();
            break;
        case SB_POW:
            eat(SB_POW);
            type = compileFactor();
            checkIntType(type);
            compileTerm2();
            break;
            // check the FOLLOW set
        case SB_PLUS:
        case SB_MINUS:
        case KW_TO:
        case KW_DO:
        case SB_RPAR:
        case SB_COMMA:
        case SB_EQ:
        case SB_NEQ:
        case SB_LE:
        case SB_LT:
        case SB_GE:
        case SB_GT:
        case SB_RSEL:
        case SB_SEMICOLON:
        case KW_END:
        case KW_ELSE:
        case KW_THEN:
        case KW_CASE:
        case KW_BEGIN:
        case KW_DEFAULT:
        case KW_BREAK:
            break;
        default:
            error(ERR_INVALID_TERM, lookAhead->lineNo, lookAhead->colNo);
    }
}

Type *compileFactor(void) {

    Object *obj;
    Type *type;

    switch (lookAhead->tokenType) {
        case TK_NUMBER:
            eat(TK_NUMBER);
            return makeIntType();
        case TK_CHAR:
            eat(TK_CHAR);
            return makeCharType();
        case TK_STRING:
            eat(TK_STRING);
            return makeStringType();
        case TK_DOUBLE:
            eat(TK_DOUBLE);
            return makeDoubleType();
        case TK_IDENT:
            eat(TK_IDENT);
            // check if the identifier is declared
            obj = checkDeclaredIdent(currentToken->string);

            switch (obj->kind) {
                case OBJ_CONSTANT:
                    switch (obj->constAttrs->value->type) {
                        case TP_INT:
                            return makeIntType();

                        case TP_CHAR:
                            return makeCharType();
                    }
                    break;
                case OBJ_VARIABLE:
                    if (obj->varAttrs->type->typeClass != TP_ARRAY) type = obj->varAttrs->type;
                    else return compileIndexes(obj->varAttrs->type);
                    break;
                case OBJ_PARAMETER:
                    type = obj->paramAttrs->type;
                    break;
                case OBJ_FUNCTION:
                    type = obj->funcAttrs->returnType;
                    compileArguments(obj->funcAttrs->paramList);
                    break;
                default:
                    error(ERR_INVALID_FACTOR, currentToken->lineNo, currentToken->colNo);
                    break;
            }
            break;
        default:
            error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
    }

    return type;
}

Type *compileIndexes(Type *arrayType) {
    Type *type = arrayType;
    while (lookAhead->tokenType == SB_LSEL) {
        eat(SB_LSEL);
        checkIntType(compileExpression());
        arrayType = arrayType->elementType;
        eat(SB_RSEL);
        type = arrayType;
        if (type->typeClass != TP_ARRAY) {
            break;
        } else if (lookAhead->tokenType != SB_LSEL)
            error(ERR_DIMENSIONAL_OF_ARRAY, currentToken->lineNo, currentToken->colNo);
    }
    if (lookAhead->tokenType == SB_LSEL) error(ERR_DIMENSIONAL_OF_ARRAY, currentToken->lineNo, currentToken->colNo);
    return type;
}

int compile(char *fileName) {
    if (openInputStream(fileName) == IO_ERROR)
        return IO_ERROR;

    currentToken = NULL;
    lookAhead = getValidToken();

    initSymTab();

    compileProgram();

    printObject(symtab->program, 0);

    cleanSymTab();

    free(currentToken);
    free(lookAhead);
    closeInputStream();
    return IO_SUCCESS;

}