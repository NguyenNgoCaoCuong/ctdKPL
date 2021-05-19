/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#define MAX_IDENT_LEN 15

//Kiểu dữ liệu
enum TypeClass {
  TP_INT,
  TP_CHAR,
  TP_ARRAY
};

//Phân loại ký hiệu
enum ObjectKind {
  OBJ_CONSTANT,
  OBJ_VARIABLE,
  OBJ_TYPE,
  OBJ_FUNCTION,
  OBJ_PROCEDURE,
  OBJ_PARAMETER,
  OBJ_PROGRAM
};

enum ParamKind {
  PARAM_VALUE,
  PARAM_REFERENCE
};

struct Type_ {
  enum TypeClass typeClass;
  //Chỉ sử dụng cho kiểu mảng
  int arraySize;
  struct Type_ *elementType;
};

typedef struct Type_ Type;
typedef struct Type_ BasicType;


struct ConstantValue_ {
  enum TypeClass type;
  union {
    int intValue;
    char charValue;
  };
};

typedef struct ConstantValue_ ConstantValue;

struct Scope_;
struct ObjectNode_;
struct Object_;
//Hằng số
struct ConstantAttributes_ {
  ConstantValue* value;
};
//Biến
struct VariableAttributes_ {
  Type *type;
  //Phạm vi của biến (được sử dụng cho pha sinh mã)
  struct Scope_ *scope;
};
//Kiểu
struct TypeAttributes_ {
  Type *actualType;
};
//Thủ tục
struct ProcedureAttributes_ {
  struct ObjectNode_ *paramList;//Danh sách các object trong thủ tục
  struct Scope_* scope;//Phạm vi của thủ tục
};

struct FunctionAttributes_ {
  struct ObjectNode_ *paramList;//Danh sách các object trong hàm
  Type* returnType;//Kiểu trả về của hàm
  struct Scope_ *scope;//Phạm vi hàm
};
//Chương trình
struct ProgramAttributes_ {
  struct Scope_ *scope;
};
//Biến truyền vào
struct ParameterAttributes_ {
  enum ParamKind kind;//Kiểu truyền biến tham chiếu hay tham trị
  Type* type;//Kiểu của biến truyền vào
  struct Object_ *function;//Hàm chứa
};
/**
*Lưu ý: các đối tượng tham số hình thức vừa được đăng ký trong danh
*sách tham số (paramList), vừa được đăng ký trong danh sách các đối
*tượng được định nghĩa trong block (scope->objList)
 */
typedef struct ConstantAttributes_ ConstantAttributes;
typedef struct TypeAttributes_ TypeAttributes;
typedef struct VariableAttributes_ VariableAttributes;
typedef struct FunctionAttributes_ FunctionAttributes;
typedef struct ProcedureAttributes_ ProcedureAttributes;
typedef struct ProgramAttributes_ ProgramAttributes;
typedef struct ParameterAttributes_ ParameterAttributes;
//Thuộc tính của đối tượng trên bảng ký hiệu
struct Object_ {
  char name[MAX_IDENT_LEN];//Tên đối tượng
  enum ObjectKind kind;//Kiểu ký hiệu
  union {
    ConstantAttributes* constAttrs;
    VariableAttributes* varAttrs;
    TypeAttributes* typeAttrs;
    FunctionAttributes* funcAttrs;
    ProcedureAttributes* procAttrs;
    ProgramAttributes* progAttrs;
    ParameterAttributes* paramAttrs;
  };
};

typedef struct Object_ Object;

struct ObjectNode_ {
  Object *object;
  struct ObjectNode_ *next;
};

typedef struct ObjectNode_ ObjectNode;
//Phạm vi của một khối (block)
struct Scope_ {
  ObjectNode *objList;//Danh sách các đối tượng trong block
  Object *owner;//Hàm thủ tục chương trình ứng với block
  struct Scope_ *outer;//Phạm vi bên ngoài
};

typedef struct Scope_ Scope;
//bảng ký hiệu
struct SymTab_ {
  Object* program;// Chương trình chính
  Scope* currentScope;//Phạm vi hiện tại(ghi nhớ block hiện đang duyệt trong biến current Scope)
  ObjectNode *globalObjectList;//Các đối tượng toàn cục như hàm WRITEI, WRITEC,...
};

typedef struct SymTab_ SymTab;

Type* makeIntType(void);
Type* makeCharType(void);
Type* makeArrayType(int arraySize, Type* elementType);
Type* duplicateType(Type* type);
int compareType(Type* type1, Type* type2);
void freeType(Type* type);

ConstantValue* makeIntConstant(int i);
ConstantValue* makeCharConstant(char ch);
ConstantValue* duplicateConstantValue(ConstantValue* v);

Scope* createScope(Object* owner, Scope* outer);

Object* createProgramObject(char *programName);
Object* createConstantObject(char *name);
Object* createTypeObject(char *name);
Object* createVariableObject(char *name);
Object* createFunctionObject(char *name);
Object* createProcedureObject(char *name);
Object* createParameterObject(char *name, enum ParamKind kind, Object* owner);

Object* findObject(ObjectNode *objList, char *name);

void initSymTab(void);
void cleanSymTab(void);
void enterBlock(Scope* scope);
void exitBlock(void);
void declareObject(Object* obj);

#endif
