# lua-comment
### 虚拟机运转的核心功能
| 模块名   	| 函数前缀  | 模块说明 | 文件模块头说明                                            |
| -------- 	| --------  | -------- 	| ---------------------------------------------- 			|
| lua.h | lua_ 		| C 语言接口 |供外部程序使用的 API 则使用 lua_ 的命名风格													|
| lctype | 无 | C标准库中ctype相关实现 | 'ctype' functions for Lua |
| ldebug 	| luaG_ 	| Debug接口 	| Debug Interface 											|
| ldo 		| luaD_(Do) | 函数调用以及栈管理 		| Stack and Call structure of Lua				 			|
| lfunc 	| luaF_ 	| 函数原型及闭包管理 	| Auxiliary functions to manipulate prototypes and closures |
| lgc 		| luaC_ 	| 垃圾回收  		| Garbage Collector 										|
| lmem 		| luaM_ 	| 内存管理接口 		| Interface to Memory Manager 								|
| lobject 	| luaO_ 	| 对象操作的一些函数 	| Type definitions for Lua objects 							|
| lopcodes 	| luaP_ 	| 虚拟机的字节码定义 	| Opcodes for Lua virtual machine 							|
| lstate 	| luaE_ 	| 全局状态机 	|Global State												|
| lstring 	| luaS_ 	| 字符串池 	|String table (keep all strings handled by Lua)				|
| ltable 	| luaH_(Hash) | 表类型的相关操作 	|Lua tables (hash)											|
| ltm 		| luaT_ 	| 元方法 		|Tag methods												|
| lvm 		| luaV_ 	| 虚拟机 		|Lua virtual machine										|
| lzio 		| luaZ_ 	| 输入流接口  		|Buffered streams											|


### 源代码解析以及预编译字节码
| 模块名   	| 函数前缀  | 模块说明 | 文件模块头说明                         |
| -------- 	| --------  | -------- 	| ---------------------------------------------- 			|
| lcode    	| luaK_     | 代码生成器    	| Code generator for Lua                         			|
| ldump 	| luaU_ 	| 序列化预编译的字节码 	| save precompiled Lua chunks 								|
| llex 		| luaX_ 	| 词法分析器 		| Lexical Analyzer 											|
| lparser 	| luaY_(yacc 的含义) | 解析器	| Lua Parser 												|
| lundump 	| luaU_ 	| 还原预编译的字节码 	| load precompiled Lua chunks 								|

### 内嵌库
| 模块名   	| 函数前缀  | 模块说明 | 文件模块头说明                         |
| -------- 	| --------  | -------- 	| ---------------------------------------------- 			|
| lauxlib  	| luaL_     | 库编写用到的辅助函数库  	| Auxiliary functions for building Lua libraries 			|
| lbaselib 	| luaB_     | 基础库 	| Basic library                                  			|
| lbitlib | 无 | 位操作库 | Standard library for bitwise operations |
| lcorolib 	| luaB_ 	| 协程库 	| Coroutine Library 										|
| ldblib | db_ | Debug 库 | Interface from Lua to its debug API |
| linit 	| luaL_ 	| 内嵌库的初始化 | Initialization of libraries for lua.c and other clients 	|
| liolib | f_ 和 io_ | IO 库 | Standard I/O (and system) library |
| llimits | 无 | 一些类型和限制定义 | Limits, basic types, and some other 'installation-dependent' definitions |
| lmathlib | math_ | 数学库 | Standard mathematical library |
| loadlib | ll_ | 动态扩展库管理 | Dynamic library loader for Lua |
| loslib | os_ | OS 库 | Standard Operating System library |
| lstrlib | str_ | 字符串库 | Standard library for string operations and pattern-matching |
| ltablib | 无 | 表处理 库 | Library for Table Manipulation |
### 可执行的解析器，字节码编译器
| 模块名   	| 函数前缀  | 模块说明 | 文件模块头说明                         |
| -------- 	| --------  | -------- 	| ---------------------------------------------- 			|
| lua.c |  		| 解释器 | Lua stand-alone interpreter |
| luac |      | 字节码编译器 | Lua compiler (saves bytecodes to files; also lists bytecodes) |

### tt_和tt类型，1字节
| 类型   	| 类型说明  | tt_数值 | tt数值 | 代码                         |
| -------- 	| --------  | -------- 	| ---------------------------------------------- 			 | ---------------------------------------------- 			 |
| LUA_TNONE | 无类型 | -1 | -1 |      |
| LUA_TNIL             | 空类型 | 0 | 0 |      |
| LUA_TBOOLEAN         | 布尔类型 | 1 | 1 |      |
| LUA_TLIGHTUSERDATA   | 指针类型(void *) | 2 | 2 |      |
| LUA_TNUMBER          | 数字 | 3 | 3 |      |
| LUA_TNUMFLT   | float | 3 | 3 | (LUA_TNUMBER \| (0 << 4)) |
| LUA_TNUMINT | integer | 19 | 19 | (LUA_TNUMBER \| (1 << 4)) |
| LUA_TSTRING          | 字符串类型 | 68 | 4 |      |
| LUA_TSHRSTR          | 短字符串 | 68 | 4 | (LUA_TSTRING \| (0 << 4)) |
| LUA_TLNGSTR          | 长字符串 | 84 | 20 | (LUA_TSTRING \| (1 << 4)) |
| LUA_TTABLE         | 表类型           | 69  | 5    |                             |
| LUA_TFUNCTION      | 函数类型 | 70 | 6 |      |
| LUA_TLCL           | Lua closure      | 70  | 6    | (LUA_TFUNCTION \| (0 << 4)) |
| LUA_TLCF           | light C function | 84 | 22   | (LUA_TFUNCTION \| (1 << 4)) |
| LUA_TCCL           | C closure        | 102 | 38   | (LUA_TFUNCTION \| (2 << 4)) |
| LUA_TUSERDATA      | 指针类型(void *) | 71  | 7    |                             |
| LUA_TTHREAD        | Lua虚拟机、协程  | 72  | 8    |                             |



### marked类型，1字节
| 类型   	| 位移  | 数值 | 代码                         |
| -------- 	| --------  | -------- 	| ---------------------------------------------- 			|
| WHITE0BIT | 0 | 1 | 由于mark是分步原因，需要分出while和otherwhile |
| WHITE1BIT    | 1 | 2 | 由于mark是分步原因，需要分出while和otherwhile |
| WHITEBITS | - | 3 |      |
| BLACKBIT | 2 | 4 |      |
| FINALIZEDBIT | 3 | 8 |      |
| gray | 低三位不为0 | 低三位不为0 | neither white nor black |
|              |             |             |                                               |