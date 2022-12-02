/*
** $Id: lgc.h,v 2.91.1.1 2017/04/19 17:39:34 roberto Exp $
** Garbage Collector
** See Copyright Notice in lua.h
*/

#ifndef lgc_h
#define lgc_h


#include "lobject.h"
#include "lstate.h"

/*
** Collectable objects may have one of three colors: white, which
** means the object is not marked; gray, which means the
** object is marked, but its references may be not marked; and
** black, which means that the object and all its references are marked.
** The main invariant of the garbage collector, while marking objects,
** is that a black object can never point to a white one. Moreover,
** any gray object must be in a "gray list" (gray, grayagain, weak,
** allweak, ephemeron) so that it can be visited again before finishing
** the collection cycle. These lists have no meaning when the invariant
** is not being enforced (e.g., sweep phase).
*/
// Invariants：
//
// 所有被根集引用的对象要么是黑色，要么是灰色的。
//
// 黑色的对象不可能指向白色的。


/* how much to allocate before next GC step */
#if !defined(GCSTEPSIZE)
/* ~100 small strings */
//100短字符串 32位 2000Byte。
#define GCSTEPSIZE (cast_int(100 * sizeof(TString)))
#endif


/*
** Possible states of the Garbage Collector
*/
#define GCSpropagate	0	//扫描，mark阶段，属于Incremental Mark
#define GCSatomic	1		//原子的mark阶段
#define GCSswpallgc	2		//sweep "regular" objects
//参考g->finobj
#define GCSswpfinobj	3	//sweep objects with finalizers
//参考g->tobefnz
#define GCSswptobefnz	4	//sweep objects to be finalized
#define GCSswpend	5		//finish sweeps
//sweep阶段也会进行runafewfinalizers
//GCScallfin是将剩余部分进行清理
#define GCScallfin	6		//call remaining finalizers 
#define GCSpause	7		//


//sweep阶段，包含GCSswpallgc、GCSswpfinobj、GCSswptobefnz、GCSswpend
#define issweepphase(g)  \
	(GCSswpallgc <= (g)->gcstate && (g)->gcstate <= GCSswpend)


/*
** macro to tell when main invariant (white objects cannot point to black
** ones) must be kept. During a collection, the sweep
** phase may break the invariant, as objects turned white may point to
** still-black objects. The invariant is restored when sweep ends and
** all objects are white again.
*/

#define keepinvariant(g)	((g)->gcstate <= GCSatomic)


/*
** some useful bit tricks
*/
//去掉多个位,eg:resetbits(7,3) => 4
#define resetbits(x,m)		((x) &= cast(lu_byte, ~(m)))
//设置多个位(x) = (x)|(m) ,eg:setbits(8,3) => 11
#define setbits(x,m)		((x) |= (m))
//判断多个位
#define testbits(x,m)		((x) & (m))
//位左移
#define bitmask(b)		(1<<(b))
#define bit2mask(b1,b2)		(bitmask(b1) | bitmask(b2))
//设置单个位
#define l_setbit(x,b)		setbits(x, bitmask(b))
// 去掉单个位
#define resetbit(x,b)		resetbits(x, bitmask(b))
//判断单个位
#define testbit(x,b)		testbits(x, bitmask(b))


/* Layout for bit use in 'marked' field: */
//lua5.3中的gc，白色有两种，一种是白色，还有一种是另一种白（otherwhite），
//这两种白色用于不同gc轮回之间的乒乓切换，比如，如果当前gc轮是white0作为白色标记，
//那么在扫描阶段结束后，新创建的对象就会以white1标记，这样在清除阶段的时候，
//就只清除被标记为white0的白色对象，而下一轮gc则刚好反过来
//
#define WHITE0BIT	0  /* object is white (type 0) */
#define WHITE1BIT	1  /* object is white (type 1) */
#define BLACKBIT	2  /* object is black */
//table和UserData设置元表的情况下，多一种状态
#define FINALIZEDBIT	3  /* object has been marked for finalization */
/* bit 7 is currently used by tests (luaL_checkmemory) */

#define WHITEBITS	bit2mask(WHITE0BIT, WHITE1BIT)


#define iswhite(x)      testbits((x)->marked, WHITEBITS)
#define isblack(x)      testbit((x)->marked, BLACKBIT)
#define isgray(x)  /* neither white nor black */  \
	(!testbits((x)->marked, WHITEBITS | bitmask(BLACKBIT)))
#define tofinalize(x)	testbit((x)->marked, FINALIZEDBIT)

//获取当前状态的otherwhite。eg，当前为1，otherwhite为0
#define otherwhite(g)	((g)->currentwhite ^ WHITEBITS)
//ow为otherwhite的意思。m是marked参数
#define isdeadm(ow,m)	(!(((m) ^ WHITEBITS) & (ow)))
#define isdead(g,v)	isdeadm(otherwhite(g), (v)->marked)

//while0到while1或者while1到while0。使用异或。(x)->marked = (x)->marked ^ WHITEBITS
#define changewhite(x)	((x)->marked ^= WHITEBITS)
//去掉白色，即进入灰色。见propagatemark
#define gray2black(x)	l_setbit((x)->marked, BLACKBIT)

//返回currentwhite，0或者1
#define luaC_white(g)	cast(lu_byte, (g)->currentwhite & WHITEBITS)


/*
** Does one step of collection when debt becomes positive. 'pre'/'pos'
** allows some adjustments to be done only when needed. macro
** 'condchangemem' is used only for heavy tests (forcing a full
** GC cycle on every opportunity)
*/
//只有当GCdebt的值大于0的时候，才能触发gc运作机制
#define luaC_condGC(L,pre,pos) \
	{ if (G(L)->GCdebt > 0) { pre; luaC_step(L); pos;}; \
	  condchangemem(L,pre,pos); }

//一个对象被创建以后，在合适的时机去进行gc检查和处理
/* more often than not, 'pre'/'pos' are empty */
#define luaC_checkGC(L)		luaC_condGC(L,(void)0,(void)0)

//gc在propagate阶段是可以被中断的，也就是说，在中断的过程中，可能会有新的对象被创建，
//并且被已经被标记为黑色的对象引用，这种gc算法，是不能将黑色的对象，直接引用白色的对象的，
//因为黑色的对象已经标记和扫描完毕，本轮gc不会再进行扫描，这样被其引用的白色对象也不会被标记和扫描到，
//到了sweep阶段，因为新创建的对象未被标记和扫描，因此会被当做不可达的对象被清除掉，造成不可挽回的后果

//barrier分为两种，一种是向前设置barrier,还有一种则是向后设置barrier。
//向前barrier的情况，适用于已被标记为黑色的对象类型，为不会频繁改变引用关系的数据类型，如lua的proto结构。
//而向后barrier的情况，适合被标记为黑色的对象类型，为会出现频繁改变引用关系情况的数据类型，如lua的table结构，

//也就是说，在两次调用gc功能的过程中，table中的同一个key，可能被赋值多个value，如果把这些value对象均标记为灰色，
//并放入gray列表，那么将会造成许多无谓的标记和扫描操作，因为这些value很可能不再被引用，需要被回收，
//因此，只要把已经被标记为黑色的table，重新设置为灰色，是避开这个性能问题的良好方式。
//而如果我们直接把从黑色重新标记为灰色的table对象，放入gray列表的话，如上所述，table的key和value的引用关系变化频繁，
//这个table很可能在黑色和灰色之间来回切换，进行很多重复的扫描，为了提高效率，则将他放在grayagain列表中，在atomic阶段，一次性标记和扫描完。
//

//参考luaC_barrier_，v表示TValue
#define luaC_barrier(L,p,v) (  \
	(iscollectable(v) && isblack(p) && iswhite(gcvalue(v))) ?  \
	luaC_barrier_(L,obj2gco(p),gcvalue(v)) : cast_void(0))

//加入限定条件。参考luaC_barrierback_
#define luaC_barrierback(L,p,v) (  \
	(iscollectable(v) && isblack(p) && iswhite(gcvalue(v))) ? \
	luaC_barrierback_(L,p) : cast_void(0))

//参考luaC_barrier_，o表示gcobject
#define luaC_objbarrier(L,p,o) (  \
	(isblack(p) && iswhite(o)) ? \
	luaC_barrier_(L,obj2gco(p),obj2gco(o)) : cast_void(0))

//参考luaC_upvalbarrier_
#define luaC_upvalbarrier(L,uv) ( \
	(iscollectable((uv)->v) && !upisopen(uv)) ? \
         luaC_upvalbarrier_(L,uv) : cast_void(0))

LUAI_FUNC void luaC_fix (lua_State *L, GCObject *o);
LUAI_FUNC void luaC_freeallobjects (lua_State *L);
LUAI_FUNC void luaC_step (lua_State *L);
LUAI_FUNC void luaC_runtilstate (lua_State *L, int statesmask);
LUAI_FUNC void luaC_fullgc (lua_State *L, int isemergency);
LUAI_FUNC GCObject *luaC_newobj (lua_State *L, int tt, size_t sz);
LUAI_FUNC void luaC_barrier_ (lua_State *L, GCObject *o, GCObject *v);
LUAI_FUNC void luaC_barrierback_ (lua_State *L, Table *o);
LUAI_FUNC void luaC_upvalbarrier_ (lua_State *L, UpVal *uv);
LUAI_FUNC void luaC_checkfinalizer (lua_State *L, GCObject *o, Table *mt);
LUAI_FUNC void luaC_upvdeccount (lua_State *L, UpVal *uv);


#endif
