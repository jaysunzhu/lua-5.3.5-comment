/*
** $Id: lobject.h,v 2.117.1.1 2017/04/19 17:39:34 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/*
** Extra tags for non-values
*/
#define LUA_TPROTO	LUA_NUMTAGS		/* function prototypes */
#define LUA_TDEADKEY	(LUA_NUMTAGS+1)		/* removed keys in tables */

/* 原始的数据类型只要用低四位就可以表示了，因为原始数据类型不超过16种 */

/*
** number of all possible tags (including LUA_TNONE but excluding DEADKEY)
*/
#define LUA_TOTALTAGS	(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* value)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/
/*
** 对于打了标签的Value对象，其包所含的内部类型有如下定义：
** 低四位(bits 0-3)用于表示基本类型
** 中间两位(bits 4-5)用于表示变种类型
** 第6位用于表示一个value对象是不是可以回收的。
*/


/*
** LUA_TFUNCTION variants:
** 0 - Lua function
** 1 - light C function
** 2 - regular C function (closure)
*/
/*
** lua中的函数总共有三种类型，一种是lua闭包，一种是没有upvalue的C函数，
** 一种是有upvalue的C函数，即C闭包。
*/

/* Variant tags for functions */
/*
** LUA_TFUNCTION宏的值是6，即00000110b，那么依下面的定义有：
** lua闭包（有Upval）的类型为00000110b
** light C函数（无Upval）的类型为00010110b
** C闭包（有Upval）的类型为00100110b
*/
#define LUA_TLCL	(LUA_TFUNCTION | (0 << 4))  /* Lua closure 6*/ 
/* light C function是指没有upvalue的函数 */
#define LUA_TLCF	(LUA_TFUNCTION | (1 << 4))  /* light C function 22*/  
#define LUA_TCCL	(LUA_TFUNCTION | (2 << 4))  /* C closure 38*/ 


/* Variant tags for strings */
/*
** 字符串类型的变种标记，由字符串原始类型按位或变种掩码组成，
** 其中0表示短字符串变种掩码，0x10表示长字符串变种掩码
*/
#define LUA_TSHRSTR	(LUA_TSTRING | (0 << 4))  /* short strings 4*/ //
#define LUA_TLNGSTR	(LUA_TSTRING | (1 << 4))  /* long strings 20*/ //


/* Variant tags for numbers */
/*
** 数据类型的变种标记，由数据原始类型按位或变种掩码组成，
** 其中0表示浮点型变种掩码，0x10表示整型变种掩码
** LUA_TNUMFLT是表示单精度浮点数，LUA_TNUMINT表示整型
*/

#define LUA_TNUMFLT	(LUA_TNUMBER | (0 << 4))  /* float numbers 3*/ //
#define LUA_TNUMINT	(LUA_TNUMBER | (1 << 4))  /* integer numbers 19*/ //


/* Bit mark for collectable types */
/* 垃圾回收的掩码（0x40） */
#define BIT_ISCOLLECTABLE	(1 << 6)

/* mark a tag as collectable */
/* 给需要进行垃圾回收操作的数据类型打需要进行垃圾回收的标记位 */
#define ctb(t)			((t) | BIT_ISCOLLECTABLE)


/*
** Common type for all collectable objects
*/
typedef struct GCObject GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
// next：在allgc链表中，指定下一个gc对象的指针
// tt：记录gc对象的类型，不同的gc对象在propagate阶段有不同的处理逻辑
// marked：用来标记gc对象颜色用的，默认是currentwhite
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


/*
** Common type has only the common header
*/
struct GCObject {
  CommonHeader;
};




/*
** Tagged Values. This is the basic representation of values in Lua,
** an actual value plus a tag with its type.
*/

/*
** Union of all Lua values
*/
/*
** lua中所有可能值的联合体，通过包含GCObject *类型的成员gc，我们就可以将所有
** 需要进行垃圾回收（GC）的类型也包含进Value中，进而lua中所有类型的值都可以用Value来表示。
** lua中的数据分为值类型和引用类型，引用类型需要有GC来维护其生命周期，Value中的gc成员则
** 代表了所有的引用类型；Value中的其余成员就是值类型，直接存放在Value中，
*/
// Lua 中的数据可以这样分为两类：值类型和引用类型。值类型可以被任意复制，而引用类型共享一份数
// 据，由 GC 负责维护生命期
typedef union Value {
  GCObject *gc;    /* collectable objects */
  void *p;         /* light userdata */ //这种值在C中，需要我们自行管理内存
  int b;           /* booleans */
  lua_CFunction f; /* light C functions */
  lua_Integer i;   /* integer numbers */
  lua_Number n;    /* float numbers */
} Value;


/*
** TValue的成员定义，tt_表示值的原始数据类型，
** 原始数据类型由GC标记位、变种类型标记位和基本类型标记位组成。
** 如果不需要进行GC，则GC标记位为0，如果无变种类型，则变种标记位为0.
*/
//tt_ 对于table的 为key的TValue，tt_还有一个表示LUA_TDEADKEY，表示keyvalue已经失效
#define TValuefields	Value value_; int tt_

/*
** TValua的定义
** TValue是lua中所有值的基本表示，由具体的值和一个关联的原始数据类型组成。
** 数据类型存放在TValuefields中的tt_成员中。lua内部的任何数据都用TValue来表示。
*/
typedef struct lua_TValue {
  TValuefields;
} TValue;



/* macro defining a nil value */
#define NILCONSTANT	{NULL}, LUA_TNIL

/* val_宏用于获取具体的值内容 */
#define val_(o)		((o)->value_)


/* raw type tag of a TValue */
/* TValue的原始类型 */
#define rttype(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
/* 获取x的基本类型，不含变种类型和GC标记位 */
#define novariant(x)	((x) & 0x0F)

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */
/*
** TValua的类型标记，包括基本类型标记和变种类型标记，变种类型标记由第4，5位表示，
** 基本类型标记由低四位表示。基本类型标记其实就是lua.h中的那些类型宏。
*/
#define ttype(o)	(rttype(o) & 0x3F)

/* type tag of a TValue with no variants (bits 0-3) */
/* 获取Value对象内部值对应的基本类型标记。基本类型标记其实就是lua.h中的那些类型宏。 */
#define ttnov(o)	(novariant(rttype(o)))


/* Macros to test type */
#define checktag(o,t)		(rttype(o) == (t))
#define checktype(o,t)		(ttnov(o) == (t))
#define ttisnumber(o)		checktype((o), LUA_TNUMBER)
#define ttisfloat(o)		checktag((o), LUA_TNUMFLT)
#define ttisinteger(o)		checktag((o), LUA_TNUMINT)
#define ttisnil(o)		checktag((o), LUA_TNIL)
#define ttisboolean(o)		checktag((o), LUA_TBOOLEAN)
#define ttislightuserdata(o)	checktag((o), LUA_TLIGHTUSERDATA)
#define ttisstring(o)		checktype((o), LUA_TSTRING)
#define ttisshrstring(o)	checktag((o), ctb(LUA_TSHRSTR))
#define ttislngstring(o)	checktag((o), ctb(LUA_TLNGSTR))
#define ttistable(o)		checktag((o), ctb(LUA_TTABLE))
#define ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#define ttisclosure(o)		((rttype(o) & 0x1F) == LUA_TFUNCTION)
#define ttisCclosure(o)		checktag((o), ctb(LUA_TCCL))
#define ttisLclosure(o)		checktag((o), ctb(LUA_TLCL))
#define ttislcf(o)		checktag((o), LUA_TLCF)
#define ttisfulluserdata(o)	checktag((o), ctb(LUA_TUSERDATA))
#define ttisthread(o)		checktag((o), ctb(LUA_TTHREAD))
#define ttisdeadkey(o)		checktag((o), LUA_TDEADKEY)


/* Macros to access values */
//获取TValue中的参数，基础类型和GC类型
#define ivalue(o)	check_exp(ttisinteger(o), val_(o).i)
#define fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
//数字类型分为整形和浮点，nvalue返回数字类型
#define nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_num(ivalue(o)) : fltvalue(o)))
#define gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)
//lightuserdata
#define pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)

//tsvalue用于获取UTString对象
#define tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))
#define uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))
#define clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
//Lclosure
#define clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
//Cclosure
#define clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))
//light C function
#define fvalue(o)	check_exp(ttislcf(o), val_(o).f)

#define hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))
#define bvalue(o)	check_exp(ttisboolean(o), val_(o).b)
#define thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))
/* a dead value may get the 'gc' field, but cannot access its contents */
#define deadvalue(o)	check_exp(ttisdeadkey(o), cast(void *, val_(o).gc))

//lua 判断语句是否为false。包含nil或者false
#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0))

/*
** 检查给定类型是否需要进行垃圾回收，其实LUA_TSTRING之后的数据类型
** 都要进行垃圾回收。
*/
#define iscollectable(o)	(rttype(o) & BIT_ISCOLLECTABLE)


/* Macros for internal tests */
#define righttt(obj)		(ttype(obj) == gcvalue(obj)->tt)

#define checkliveness(L,obj) \
	lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == NULL || !isdead(G(L),gcvalue(obj)))))


/* Macros to set values */
/* 设置值的原始类型 */
#define settt_(o,t)	((o)->tt_=(t))

/* 设置TValue中的lua_Number类型成员f的值 */
#define setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_TNUMFLT); }

/* 修改TValue中的lua_Number类型成员f的值 */
#define chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

/* 设置TValue中的lua_Integer类型成员i的值 */
#define setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_TNUMINT); }

/* 修改TValue中的lua_Integer类型成员i的值 */
#define chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

#define setnilvalue(obj) settt_(obj, LUA_TNIL)

/* 设置TValue中的lua_CFunction类型成员f的值 */
#define setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_TLCF); }

/* 设置TValue中的void *类型成员p的值 */
#define setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_TLIGHTUSERDATA); }

/* 设置TValue中的int类型成员b的值 */
#define setbvalue(obj,x) \
  { TValue *io=(obj); val_(io).b=(x); settt_(io, LUA_TBOOLEAN); }

/* 设置TValue中的GCObject* 类型成员gc的值 */
#define setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

#define setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

#define setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TUSERDATA)); \
    checkliveness(L,io); }

#define setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTHREAD)); \
    checkliveness(L,io); }

#define setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TLCL)); \
    checkliveness(L,io); }

#define setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TCCL)); \
    checkliveness(L,io); }

#define sethvalue(L,obj,x) \
  { TValue *io = (obj); Table *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTABLE)); \
    checkliveness(L,io); }

//设置为deadkey。table value为nil时，key对象特有状态。
#define setdeadvalue(obj)	settt_(obj, LUA_TDEADKEY)


/* from stack to (same) stack */
//from obj2 to obj1
#define setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); *io1 = *(obj2); \
	  (void)L; checkliveness(L,io1); }


/*
** different types of assignments, according to destination
*/

/* from stack to (same) stack */
//其它寄存器到寄存器
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

/* to table (define it as an expression to be used in macros) */
#define setobj2t(L,o1,o2)  ((void)L, *(o1)=*(o2), checkliveness(L,(o1)))




/*
** {======================================================
** types and prototypes
** =======================================================
*/

/* 
** 栈指针类型，表明栈单元存放的是TValue类型的数据，由于TValue可以表示lua中所有类型的数据，
** 因此栈中可以存放所有类型的数据
*/
typedef TValue *StkId;  /* index to stack elements */

/*
**
*/


/*
** Header for string value; string bytes follow the end of this structure
** (aligned according to 'UTString'; see next).
*/
/*
** TString类型是lua中string类型的头部，string类型的具体内容紧跟在头部之后，
** 并且在内存中需要按照8字节对齐。所有的短字符串均存放在全局状态信息(global_State)
** 的strt成员中，该成员是一个hash表。相同的短字符串在lua只有唯一一份，
** 而长字符串则独立存放，从外部压入一个长字符串时，并不立刻计算其hash值，而是标记
** 一下TString中的中的extra成员，表示暂未计算hash值，直到需要对长字符串做键匹配时，
** 才计算其hash值。
*/
typedef struct TString {
  CommonHeader;		/* 所有类型的公共头部 */
  /*
  ** extra成员对于长字符串来说，是用于表明长字符串是否已经计算了hash值的标志位，1表示已计算hash值。
  ** extra成员对于短字符串来说，用于标记一个字符串是否是lua保留的字符串，如关键字等。
  ** 这种情况下，extra成员存放的就是紧跟在头部后面的字符串在luaX_tokens数组中的下标。
  ** 如果一个短字符串不是保留字符串，那么extra就位0。
  */
//   TString为长字符串时：当extra=0表示该字符串未进行hash运算；当extra=1时表示该字符串已经进行过hash运算。
//   TString为短字符串时：当extra=0时表示它需要被gc托管；当extra=1时表示该字符串不会被gc回收。
  lu_byte extra;  /* reserved words for short strings; "has hash" for longs */
  /* 由于Lua不以'\0'来识别字符串的长度，因此需要显示保存字符串的长度。 */
  lu_byte shrlen;  /* length for short strings */ /* 短字符串的长度 */
  unsigned int hash;	/* 该string对应的hash值，由string进行hash后得到 */
  union {
    size_t lnglen;  /* length for long strings */ /* 长字符串的长度 */
    /*
    ** 只有短字符串才有用lua是用散列桶来存放string的，hnext用于指向下一个具有相同hash值的string，
    ** 即具有相同hash值的字符串是用链表来串接起来的。（这个成员由短字符串对象使用。）
    */
    struct TString *hnext;  /* linked list for hash table */
  } u;
  //内存对象模型中，TString后长度为len+1的空间就是字符串所在内存
  char *temp;
} TString;


/*
** Ensures that address after this type is always fully aligned.
*/
/* 这个联合体用于确保string的起始地址肯定是8字节对齐的。 */
typedef union UTString {
  L_Umaxalign dummy;  /* ensures maximum alignment for strings */
  TString tsv;
} UTString;


/*
** Get the actual string (array of bytes) from a 'TString'.
** (Access to 'extra' ensures that value is really a 'TString'.)
*/
/*
** lua中的string内容是紧跟在string头部之后的，所以这里返回的是内容的起始地址。
** cast(char *, (ts))是获取字符串对象的起始地址。
*/
#define getstr(ts)  \
  check_exp(sizeof((ts)->extra), cast(char *, (ts)) + sizeof(UTString))


/* get the actual string (array of bytes) from a Lua value */
/* 从一个value对象中获取UTString对象中的实际字符串内容 */
#define svalue(o)       getstr(tsvalue(o))

/* get string length from 'TString *s' */
/* 获取字符串对象的长度，短字符串和长字符串中存放长度的地方不一样 */
#define tsslen(s)	((s)->tt == LUA_TSHRSTR ? (s)->shrlen : (s)->u.lnglen)

/* get string length from 'TValue *o' */
/* 从一个value对象中获取UTString对象中实际字符串的长度 */
#define vslen(o)	tsslen(tsvalue(o))


/*
** Header for userdata; memory area follows the end of this structure
** (aligned according to 'UUdata'; see next).
*/
/* userdata类型的头部，而userdata类型的实际内容（用户数据）则紧跟在头部之后 */
typedef struct Udata {
  CommonHeader;
  //Lua API debug.setuservalue或者C API lua_getuservalue设置 
  lu_byte ttuv_;  /* user value's tag */
  /* userdata对象的元表 */
  struct Table *metatable;
  /* 用于数据的长度 */
  size_t len;  /* number of bytes */
  //Lua API debug.setuservalue或者C API lua_getuservalue设置 
  union Value user_;  /* user value */

  /* 对于LUA_TUSERDATA类型而言，从这里开始就是用户的数据，内存长度为len字节 */
} Udata;


/*
** Ensures that address after this type is always fully aligned.
*/
typedef union UUdata {
  L_Umaxalign dummy;  /* ensures maximum alignment for 'local' udata */
  Udata uv;
} UUdata;


/*
**  Get the address of memory block inside 'Udata'.
** (Access to 'ttuv_' ensures that value is really a 'Udata'.)
*/
/* 返回userdata中存放用户数据的内存起始地址 */
#define getudatamem(u)  \
  check_exp(sizeof((u)->ttuv_), (cast(char*, (u)) + sizeof(UUdata)))

#define setuservalue(L,u,o) \
	{ const TValue *io=(o); Udata *iu = (u); \
	  iu->user_ = io->value_; iu->ttuv_ = rttype(io); \
	  checkliveness(L,io); }


#define getuservalue(L,u,o) \
	{ TValue *io=(o); const Udata *iu = (u); \
	  io->value_ = iu->user_; settt_(io, iu->ttuv_); \
	  checkliveness(L,io); }


/*
** Description of an upvalue for function prototypes
*/
/* lua函数upvalue描述信息 */
//proto负数内容Upvaldesc，在编译时候已经确定
typedef struct Upvaldesc {
  //当外层函数并没有退出时，我们调用刚生成的闭包，这个时候闭包更像一个普通的内嵌函数。外层函数
  // 的局部变量只是数据栈上的一个普通变量，虚拟机用一个数据栈上的索引映射局部变量，内嵌函数可以通过
  // 数据栈自由访问它。而一旦外层函数返回，数据栈空间收缩，原有的局部变量消失了。这个时候闭包需要用
  // 其它方式访问这些 upvalue 。
  
  TString *name;  /* upvalue name (for debug information) */
  //是否在stack上。chunk的closure中_ENV是在栈上，其他closure _ENV不在栈上
  lu_byte instack;  /* whether it is in stack (register) */
  //instack==true, 在数据栈上base+idx地址获得引用，可参考pushclosure或getcached
  //instack==false, 在上级调用函数的upvalue中获取引用，可参考pushclosure或getcached
  //instack==true效率更低，可参考luaF_findupval
  lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
/* lua函数中定义的本地变量 */
//debug使用
typedef struct LocVar {
  TString *varname;
  //记录active在proto的pc区间[startpc，endpc)
  int startpc;  /* first point where variable is active */
  int endpc;    /* first point where variable is dead */
} LocVar;


/*
** Function Prototypes
*/
/*
** Proto结构体用于存放函数的原型信息
** 注意，整个lua代码文件解析完成之后，也是生成这么一个Proto的对象，即整个代码文件也当做是一个函数。
*/
//使用https://www.luac.nl/s/14fbea26ac382adff23862142a(最佳)或者luac查看原型
typedef struct Proto {
  CommonHeader;

  /* 函数原型中固定参数的数量 */
  lu_byte numparams;  /* number of fixed parameters */

  // 这可能是由一次 open call ，所调用的函数返回了不定数量的参数；
  // 也可以是在 Lua 中用 ... 引用不定数量的参数，它会生成 VARARG 这个操作的字节码
  lu_byte is_vararg;  
  
  //数据栈slot数量，包含了此proto的参数、局部变量以及proto中所有call函数中最多的实参和对于1个cl，不含upvalue。空函数情况下为2
  lu_byte maxstacksize;  /* number of registers needed by this function */

  /* cl对应的Upvalue的个数。 */
  int sizeupvalues;  /* size of 'upvalues' */
  
  //常量个数
  int sizek;  /* size of 'k' */

  /* 字节码占用的数组大小 */
  int sizecode;
  //lineinfo的数组大小
  int sizelineinfo;
  
  //该函数（原型）内定义了函数的的数量
  int sizep;  /* size of 'p' */
  
  //包含局部变量（注意：含闭包function定义）和参数(注意:语法糖中的self)
  int sizelocvars;
  
  int linedefined;  /* debug information  */
  int lastlinedefined;  /* debug information  */

  /* 存放该函数使用的所有常量的数组，k为数组首地址 */
  TValue *k;  /* constants used by the function */

  /* code指向的是该函数对应的字节码存放的地址，该地址指向的内存不在虚拟栈中，而是动态申请的 */
  Instruction *code;  /* opcodes */

  /* p指向的是在这个函数内定义的其他函数所对应的Proto结构体指针数组 */
  struct Proto **p;  /* functions defined inside the function */
  int *lineinfo;  /* map from opcodes to source lines (debug information) */

  /* locvars存放了该函数使用到的本地变量（即在函数内部定义的局部变量）信息（仅用于debug） */
  LocVar *locvars;  /* information about local variables (debug information) */
  //那局部变量放在哪里？在L的数据栈上
  
  //由于proto的upvalue可能不一样，所以数据放到LClosure
  Upvaldesc *upvalues;  /* upvalue information */
  
  //只保留最近的一个Lua Closure，正常proto和cl是一一对应的，cache一个就够。只有cl中upvalue地址不一样（即upvalue是closed状态），才可能有一个proto有多个cl
  struct LClosure *cache;  /* last-created closure with this prototype */
  TString  *source;  /* used for debug information */
  GCObject *gclist;//参考linkgclist
} Proto;



/*
** Lua Upvalues
*/
typedef struct UpVal UpVal;


/*
** Closures
*/
/* 闭包头的定义，nupvalues是闭包所含有的upvalue的个数 */
//参考linkgclist
#define ClosureHeader \
	CommonHeader; lu_byte nupvalues; GCObject *gclist

/* C Closure对象 */
typedef struct CClosure {
  ClosureHeader;
  /* 函数指针 */
  lua_CFunction f;
  /* C Closure对应的upvalue列表 */
  //和 Lua 闭包不同，在 C 函数中不会去引用外层函数中的局部变量。 所以，C 闭包中的 upval 天生就是关闭状态的
  TValue upvalue[1];  /* list of upvalues */
} CClosure;


/* Lua Closure对象 */
typedef struct LClosure {
  ClosureHeader;
  struct Proto *p; /* 存放lua闭包对应的函数原型信息 */
    
  //Lua Closure对象的upval有两种来源，
  //第一、Lua 闭包一般在虚拟机运行的过程中被动态构造出来的。这时，闭包需引用的 upval 都在当前
  //的数据栈上, 利用 luaF_findupval 可以把数据栈上的值转换为 upval
  //第二种生成 Lua 闭包的方式是加载一段 Lua 代码。每段 Lua 代码都会被编译成一个函数原型，但 Lua
  //的外部 API 是不返回函数proto的，而是把这个函数原型转换为一个 Lua closure。
  
  //闭包只是对upvalue的引用，因为多个闭包可以用相同的upvalue的引用
  UpVal *upvals[1];  /* list of upvalues */
} LClosure;


/* 闭包联合体，包括lua闭包和C闭包 */
typedef union Closure {
  CClosure c;
  LClosure l;
} Closure;


/* 判断value对象o是不是一个lua闭包 */
#define isLfunction(o)	ttisLclosure(o)

#define getproto(o)	(clLvalue(o)->p)


/*
** Tables
*/
/*
** Lua使用表来表示lua中的一切数据，Lua表分为数组和散列表部分，其中数组的索引是从1开始，
** 散列表部分可以存放任何不能存储在数组中的数据，唯一的要求是散列表的键值不能是nil。
** 虽然lua表内部分为了数组和散列表部分，但其对于使用者来说是透明的，表提供的操作方法
** 对数组和散列表来说是通用的。
*/

/*
** 散列桶中存放的Node节点所对应的key节点，TKey是一个联合体。一般情况下，如果看到一个
** 数据类型是union，那么这个数据类型是想以一种更省内存的方式来表示多种用途。但某一时刻
** 这个数据类型只会是其中的某一种用途。当TKey表示的是nk的时候，那么nk中的TValuefields
** 用于存放key节点的具体内容及其内容对应的类型，而nk中的next则用于串联同属于一个散列桶
** 中的其他Node节点；当TKey表示tvk的时候，那么此时key节点只存放key的内容及其数据类型。
*/
typedef union TKey {
  struct {
    TValuefields;

    /* next保存的是Node节点中相同mainpostion的下标偏移，即索引值 */
    //故当前Tkey*指针+next,就可以获得下个Tkey*指针。默认是0，表示没有下一个
    
    int next;  /* for chaining (offset for next node) */ 
  } nk;
  TValue tvk;
} TKey;


/* copy a value into a key without messing up field 'next' */
#define setnodekey(L,key,obj) \
	{ TKey *k_=(key); const TValue *io_=(obj); \
	  k_->nk.value_ = io_->value_; k_->nk.tt_ = io_->tt_; \
	  (void)L; checkliveness(L,io_); }

/*
** 散列桶中存放的节点，包括key节点和value节点，其中value节点是通用数据
** 类型TValue，而key类型则为TKey。
*/
typedef struct Node {
  TValue i_val;
  TKey i_key;
} Node;


typedef struct Table {
  CommonHeader; /* 公共头部 */
  /*
  ** flags其实是一个unsigned char类型的数据，用于表示这个表提供了哪些元方法，
  ** 当查找一次之后，如果该表中存在某个元方法，那么
  ** 将该元方法对应的bit置位，下一次查找时只要比较这个bit就行了。
  */
  //默认值是cast_byte(~0)，即255。只有常用的6个元方法才会使用。  TM_INDEX,TM_NEWINDEX,TM_GC,TM_MODE,TM_LEN,TM_EQ
  lu_byte flags;  /* 1<<p means tagmethod(p) is not present */

  /* 散列表中散列桶数组大小,其中大小做了2的对数。即大小必须符合2的对数 */
  lu_byte lsizenode;  /* log2 of size of 'node' array */

  /* 数组部分的大小 */
  unsigned int sizearray;  /* size of 'array' array */

  /* 指向数组部分的指针 */
  TValue *array;  /* array part */

  /*
  ** 指向该表的散列桶数组起始位置的指针，从lsizenode的定义可以看出散列桶数组
  ** 的大小必须是2的整数次幂，利用lisizenode求散列桶数组大小的时候可以直接通过
  ** 1 << lsizenode来实现。
  */
  Node *node;

  /* 指向该表的散列桶数组的最后一个位置的指针 */
  Node *lastfree;  /* any free position is before this position */

  /* 存放该表的元表 */
  struct Table *metatable;

  //参考linkgclist
  GCObject *gclist;
} Table;



/*
** 'module' operation for hashing (size is always a power of 2)
*/
//对超过范围进行求mod，符合合法性校验。由于是稀疏数组会放到散列桶数，同时字符串hash值等情况都会超过map的size
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
//散列表中散列桶数组大小
#define sizenode(t)	(twoto((t)->lsizenode)) 


/*
** (address of) a fixed nil value
*/
#define luaO_nilobject		(&luaO_nilobject_)


LUAI_DDEC const TValue luaO_nilobject_;

/* size of buffer for 'luaO_utf8esc' function */
#define UTF8BUFFSZ	8

LUAI_FUNC int luaO_int2fb (unsigned int x);
LUAI_FUNC int luaO_fb2int (int x);
LUAI_FUNC int luaO_utf8esc (char *buff, unsigned long x);
LUAI_FUNC int luaO_ceillog2 (unsigned int x);
LUAI_FUNC void luaO_arith (lua_State *L, int op, const TValue *p1,
                           const TValue *p2, TValue *res);
LUAI_FUNC size_t luaO_str2num (const char *s, TValue *o);
LUAI_FUNC int luaO_hexavalue (int c);
LUAI_FUNC void luaO_tostring (lua_State *L, StkId obj);
LUAI_FUNC const char *luaO_pushvfstring (lua_State *L, const char *fmt,
                                                       va_list argp);
LUAI_FUNC const char *luaO_pushfstring (lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid (char *out, const char *source, size_t len);


#endif

