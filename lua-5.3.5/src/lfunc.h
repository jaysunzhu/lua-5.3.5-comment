/*
** $Id: lfunc.h,v 2.15.1.1 2017/04/19 17:39:34 roberto Exp $
** Auxiliary functions to manipulate prototypes and closures
** See Copyright Notice in lua.h
*/

#ifndef lfunc_h
#define lfunc_h

//函数、闭包、local变量、upval、常量。可以通过一下编译成字节码学习
// https://www.luac.nl/s/14fbea26ac382adff23862142a
#include "lobject.h"

// struct LocVar、struct Upvaldesc用来辅助描述


#define sizeCclosure(n)	(cast(int, sizeof(CClosure)) + \
                         cast(int, sizeof(TValue)*((n)-1)))

#define sizeLclosure(n)	(cast(int, sizeof(LClosure)) + \
                         cast(int, sizeof(TValue *)*((n)-1)))


/* test whether thread is in 'twups' list */
#define isintwups(L)	(L->twups != L)


/*
** maximum number of upvalues in a closure (both C and Lua). (Value
** must fit in a VM register.)
*/
#define MAXUPVAL	255


/*
** Upvalues for Lua closures
*/
/* lua闭包中所使用的upvalue信息 */
struct UpVal {
// 它的 upvalue 从实现上来讲，更像是 C 语言中的指针。
// 它引用了另一个对象。多个闭包可以共享同一个 upvalue ，有如 C 语言中，可以有多份指针指向同
// 一个结构体。
//open,当被引用的变量还在数据栈上时，这个指针直接指向栈上的地址。这个 upvalue 被称为开放的
//close，就是当 upvalue 引用的数据栈上的数据不再存在于栈上时（通常是由申请局部变量的函数返回引起的），需要
// 从open到close，就是把 upvalue 从openupval链表中拿掉，并把其引用的数据栈上的变量值换一个安全的地方存放。这个安全所在就是UpVal 结构体内
  
  
  //closed，v指向value。opend，v指向stack上slot
  TValue *v;  /* points to stack or to its own value */

  //引用计数，为了做GC，无论Upvaldesc.instack是open还是close。0 no references
  //新建LClosure引用upvalue和lua_upvaluejoin的fn2 upvalue +1，反之，LClosure释放和lua_upvaluejoin的fn1 upvalue -1
  lu_mem refcount;  /* reference counter */ 
  union {
    struct {  /* (when open) */
      UpVal *next;  /* linked list */
      //是否延迟mark标志。1为需要延迟标志，参考remarkupvals
      int touched;  /* mark to avoid cycles with dead threads */
    } open;
    TValue value;  /* the value (when closed) */
  } u;

    //说明：
    //引用计数，为了控制close Upvalue是否需要GC
    //touched，是open的upvalue，是否需要mark其引用的资源
};

#define upisopen(up)	((up)->v != &(up)->u.value)


LUAI_FUNC Proto *luaF_newproto (lua_State *L);
LUAI_FUNC CClosure *luaF_newCclosure (lua_State *L, int nelems);
LUAI_FUNC LClosure *luaF_newLclosure (lua_State *L, int nelems);
LUAI_FUNC void luaF_initupvals (lua_State *L, LClosure *cl);
LUAI_FUNC UpVal *luaF_findupval (lua_State *L, StkId level);
LUAI_FUNC void luaF_close (lua_State *L, StkId level);
LUAI_FUNC void luaF_freeproto (lua_State *L, Proto *f);
LUAI_FUNC const char *luaF_getlocalname (const Proto *func, int local_number,
                                         int pc);


#endif
