/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "error.h"

extern SymTab *symtab;
extern Token *currentToken;

Object *lookupObject(char *name) {
    if (name == NULL || symtab->currentScope == NULL) return NULL;
    Scope *scope = symtab->currentScope;
    Object *obj = NULL;
    while (scope != NULL) {
        obj = findObject(scope->objList, name);
        if (obj != NULL) return obj;
        scope = scope->outer;// Duyệt ra bên ngoài
    }
    //Tìm kiếm phạm vi global
    obj = findObject(symtab->globalObjectList, name);
    return obj;
}

//Kiểm tra tên hợp lệ
void checkFreshIdent(char *name) {
    if (name == NULL || symtab->currentScope == NULL) return;
    if (findObject(symtab->currentScope->objList, name) != NULL)
        error(ERR_DUPLICATE_IDENT, currentToken->lineNo, currentToken->lineNo);
}

//Kiểm tra một dịnh danh được khai báo hay chưa
Object *checkDeclaredIdent(char *name) {
    Object *obj = lookupObject(name);
    if (obj == NULL) error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
    return obj;
}

//Kiểm tra hằng số đã được khai báo hay chưa
Object *checkDeclaredConstant(char *name) {
    Object *obj = lookupObject(name);
    if (obj == NULL) error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
    if (obj->kind != OBJ_CONSTANT) error(ERR_INVALID_CONSTANT, currentToken->lineNo, currentToken->colNo);
    return obj;
}

Object *checkDeclaredType(char *name) {
    Object *obj = lookupObject(name);
    if (obj == NULL) error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
    if (obj->kind != OBJ_TYPE) error(ERR_INVALID_TYPE, currentToken->lineNo, currentToken->colNo);
    return obj;
}

Object *checkDeclaredVariable(char *name) {
    Object *obj = lookupObject(name);
    if (obj == NULL) error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
    if (obj->kind != OBJ_VARIABLE) error(ERR_INVALID_VARIABLE, currentToken->lineNo, currentToken->colNo);
    return obj;
}

Object *checkDeclaredFunction(char *name) {
    Object *obj = lookupObject(name);
    if (obj == NULL) error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
    if (obj->kind != OBJ_FUNCTION) error(ERR_INVALID_FUNCTION, currentToken->lineNo, currentToken->colNo);
    return obj;
}

Object *checkDeclaredProcedure(char *name) {
    Object *obj = lookupObject(name);
    if (obj == NULL) error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
    if (obj->kind != OBJ_PROCEDURE) error(ERR_INVALID_PROCEDURE, currentToken->lineNo, currentToken->colNo);
    return obj;
}

//Kiểm tra một định danh trước phép gán là một biến một tham số hay là một tên hàm, nếu nó là tên hàm thì kiểm tra xem có phải tên hàm của chính nó không ??
Object *checkDeclaredLValueIdent(char *name) {
    Object *obj = lookupObject(name);
    if (obj == NULL) error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
    switch (obj->kind) {
        case OBJ_VARIABLE:
        case OBJ_PARAMETER:
            break;
        case OBJ_FUNCTION:
            if(obj == symtab->currentScope->owner) return obj;
        default:
            error(ERR_INVALID_IDENT,currentToken->lineNo,currentToken->colNo);
            break;
    }
    return obj;
}

