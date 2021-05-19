/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

void freeObject(Object *obj);

void freeScope(Scope *scope);

void freeObjectList(ObjectNode *objList);

void freeReferenceList(ObjectNode *objList);

SymTab *symtab;
Type *intType;
Type *charType;

/******************* Type utilities ******************************/

Type *makeIntType(void) {
    Type *type = (Type *) malloc(sizeof(Type));
    type->typeClass = TP_INT;
    return type;
}

Type *makeCharType(void) {
    Type *type = (Type *) malloc(sizeof(Type));
    type->typeClass = TP_CHAR;
    return type;
}

Type *makeArrayType(int arraySize, Type *elementType) {
    Type *type = (Type *) malloc(sizeof(Type));
    type->typeClass = TP_ARRAY;
    type->arraySize = arraySize;
    type->elementType = elementType;
    return type;
}

Type *duplicateType(Type *type) {
    if (type == NULL) return NULL;
    Type *type1 = (Type *) malloc(sizeof(type));
    type1->arraySize = type->arraySize;
    type1->typeClass = type->typeClass;
    if (type->elementType != NULL) {
        type1->elementType = duplicateType(type->elementType);
    }
    return type1;
}

int compareType(Type *type1, Type *type2) {
    if(type1->typeClass == type2->typeClass){
        if(type1->typeClass == TP_ARRAY){
            if(!(type1->arraySize == type2->arraySize && compareType(type1->elementType,type2->elementType)))
                return 0;
        }
        return 1;
    }
    return 0;
}

void freeType(Type *type) {
    if (type == NULL) return;
    if (type->elementType != NULL) freeType(type->elementType);
    free(type);
}

/******************* Constant utility ******************************/

ConstantValue *makeIntConstant(int i) {
    ConstantValue *constantValue = (ConstantValue *) malloc(sizeof(ConstantValue));
    constantValue->type = TP_INT;
    constantValue->intValue = i;
    return constantValue;
}

ConstantValue *makeCharConstant(char ch) {
    ConstantValue *constantValue = (ConstantValue *) malloc(sizeof(ConstantValue));
    constantValue->type = TP_CHAR;
    constantValue->charValue = ch;
    return constantValue;
}

ConstantValue *duplicateConstantValue(ConstantValue *v) {
    if (v == NULL) return NULL;
    ConstantValue *constantValue = (ConstantValue *) malloc(sizeof(ConstantValue));
    if (v->type == TP_CHAR) {
        constantValue->charValue = v->charValue;
    } else constantValue->intValue = v->intValue;
    return constantValue;
}

/******************* Object utilities ******************************/

Scope *createScope(Object *owner, Scope *outer) {
    Scope *scope = (Scope *) malloc(sizeof(Scope));
    scope->objList = NULL;
    scope->owner = owner;
    scope->outer = outer;
    return scope;
}

Object *createProgramObject(char *programName) {
    Object *program = (Object *) malloc(sizeof(Object));
    strcpy(program->name, programName);
    program->kind = OBJ_PROGRAM;
    program->progAttrs = (ProgramAttributes *) malloc(sizeof(ProgramAttributes));
    program->progAttrs->scope = createScope(program, NULL);
    symtab->program = program;

    return program;
}

Object *createConstantObject(char *name) {
    Object *object = (Object *) malloc(sizeof(Object));
    strcpy(object->name, name);
    object->kind = OBJ_CONSTANT;
    object->constAttrs = (ConstantAttributes *) malloc(sizeof(ConstantAttributes));
    object->constAttrs->value = NULL;
    return object;
}

Object *createTypeObject(char *name) {
    Object *object = (Object *) malloc(sizeof(Object));
    strcpy(object->name, name);
    object->kind = OBJ_TYPE;
    object->typeAttrs = (TypeAttributes *) malloc(sizeof(TypeAttributes));
    object->typeAttrs->actualType = NULL;
    return object;
}

Object *createVariableObject(char *name) {
    Object *object = (Object *) malloc(sizeof(Object));
    strcpy(object->name, name);
    object->kind = OBJ_VARIABLE;
    object->varAttrs = (VariableAttributes *) malloc(sizeof(VariableAttributes));
    object->varAttrs->type = NULL;
    object->varAttrs->scope = symtab->currentScope;
    return object;
}

Object *createFunctionObject(char *name) {
    Object *object = (Object *) malloc(sizeof(Object));
    strcpy(object->name, name);
    object->kind = OBJ_FUNCTION;
    object->funcAttrs = (FunctionAttributes *) malloc(sizeof(FunctionAttributes));
    object->funcAttrs->scope = symtab->currentScope;
    object->funcAttrs->paramList = NULL;
    object->funcAttrs->returnType = NULL;
    return object;
}

Object *createProcedureObject(char *name) {
    Object *object = (Object *) malloc(sizeof(Object));
    strcpy(object->name, name);
    object->kind = OBJ_PROCEDURE;
    object->procAttrs = (ProcedureAttributes *) malloc(sizeof(ProcedureAttributes));
    object->procAttrs->paramList = NULL;
    object->procAttrs->scope = symtab->currentScope;
    return object;
}

Object *createParameterObject(char *name, enum ParamKind kind, Object *owner) {
    Object *object = (Object *) malloc(sizeof(Object));
    strcpy(object->name, name);
    object->kind = OBJ_PARAMETER;
    object->paramAttrs = (ParameterAttributes *) malloc(sizeof(ParameterAttributes));
    object->paramAttrs->type = NULL;
    object->paramAttrs->kind = kind;
    object->paramAttrs->function = owner;
    return object;
}

void freeObject(Object *obj) {
    if (obj == NULL) return;
    switch (obj->kind) {
        case OBJ_PARAMETER:
            freeObject(obj->paramAttrs->function);
            freeType(obj->paramAttrs->type);
            break;
        case OBJ_PROCEDURE:
            freeScope(obj->procAttrs->scope);
            freeObjectList(obj->procAttrs->paramList);//??????????????
            break;
        case OBJ_FUNCTION:
            freeObjectList(obj->funcAttrs->paramList);
            freeScope(obj->funcAttrs->scope);
            freeType(obj->funcAttrs->returnType);
            break;
        case OBJ_VARIABLE:
            freeScope(obj->varAttrs->scope);
            freeType(obj->varAttrs->type);
            break;
        case OBJ_TYPE:
            freeType(obj->typeAttrs->actualType);
            break;
        case OBJ_CONSTANT:
            if (obj->constAttrs->value != NULL) free(obj->constAttrs->value);
            break;
        case OBJ_PROGRAM:
            freeScope(obj->progAttrs->scope);
            break;
    }
    free(obj);
}

void freeScope(Scope *scope) {
    if (scope == NULL) return;
    freeObject(scope->owner);
    freeObjectList(scope->objList);
    freeScope(scope->outer);
    free(scope);
}

void freeObjectList(ObjectNode *objList) {
    if (objList == NULL) return;
    freeObject(objList->object);
    freeObjectList(objList->next);
    free(objList);
}

void freeReferenceList(ObjectNode *objList) {
    if (objList == NULL) return;
    freeObject(objList->object);
    freeObjectList(objList->next);
    free(objList);
}

void addObject(ObjectNode **objList, Object *obj) {
    ObjectNode *node = (ObjectNode *) malloc(sizeof(ObjectNode));
    node->object = obj;
    node->next = NULL;
    if ((*objList) == NULL)
        *objList = node;
    else {
        ObjectNode *n = *objList;
        while (n->next != NULL)
            n = n->next;
        n->next = node;
    }
}

Object *findObject(ObjectNode *objList, char *name) {
    if(objList == NULL ||name == NULL ) return NULL;
    ObjectNode *crr = objList;
    while (crr != NULL){
        if(strcmp(crr->object->name,name) == 0) return crr;
        crr = crr->next;
    }
    return NULL;
}

/******************* others ******************************/

void initSymTab(void) {
    Object *obj;
    Object *param;

    symtab = (SymTab *) malloc(sizeof(SymTab));
    symtab->globalObjectList = NULL;

    obj = createFunctionObject("READC");
    obj->funcAttrs->returnType = makeCharType();
    addObject(&(symtab->globalObjectList), obj);

    obj = createFunctionObject("READI");
    obj->funcAttrs->returnType = makeIntType();
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITEI");
    param = createParameterObject("i", PARAM_VALUE, obj);
    param->paramAttrs->type = makeIntType();
    addObject(&(obj->procAttrs->paramList), param);
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITEC");
    param = createParameterObject("ch", PARAM_VALUE, obj);
    param->paramAttrs->type = makeCharType();
    addObject(&(obj->procAttrs->paramList), param);
    addObject(&(symtab->globalObjectList), obj);

    obj = createProcedureObject("WRITELN");
    addObject(&(symtab->globalObjectList), obj);

    intType = makeIntType();
    charType = makeCharType();
}

void cleanSymTab(void) {
    freeObject(symtab->program);
    freeObjectList(symtab->globalObjectList);
    free(symtab);
    freeType(intType);
    freeType(charType);
}

//Mỗi khi dịch một hàm hay thủ tục, phải cập nhật giá trị của currentScope
void enterBlock(Scope *scope) {
    symtab->currentScope = scope;
}

//Mỗi khi kết thúc duyệt một hàm hay thủ tục phải chuyển lại currentScope ra block bên ngoài
void exitBlock(void) {
    symtab->currentScope = symtab->currentScope->outer;
}

//Thực hiện đăng ký một đối tượng vào block hiện tại
void declareObject(Object *obj) {
    if (obj->kind == OBJ_PARAMETER) {
        Object *owner = symtab->currentScope->owner;
        switch (owner->kind) {
            case OBJ_FUNCTION:
                addObject(&(owner->funcAttrs->paramList), obj);
                break;
            case OBJ_PROCEDURE:
                addObject(&(owner->procAttrs->paramList), obj);
                break;
            default:
                break;
        }
    }

    addObject(&(symtab->currentScope->objList), obj);
}


