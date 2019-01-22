/*
* The MIT License (MIT)
* 
* Copyright (c) 2017 - 2019, CUJO LLC.
* Copyright (c) 2012, Luiz Henrique de Figueiredo, http://webserver2.tecgraf.puc-rio.br/~lhf/
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef _KERNEL
#include <string.h>
#endif

#include "lua.h"
#include "lauxlib.h"

#define MYNAME		"base64"
#define MYVERSION	MYNAME " library for " LUA_VERSION " / Aug 2012"

#define uint unsigned int

static const char code[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void encode(luaL_Buffer *b, uint c1, uint c2, uint c3, int n)
{
 unsigned long tuple=c3+256UL*(c2+256UL*c1);
 int i;
 char s[4];
 for (i=0; i<4; i++) {
  s[3-i] = code[tuple % 64];
  tuple /= 64;
 }
 for (i=n+1; i<4; i++) s[i]='=';
 luaL_addlstring(b,s,4);
}

static int Lencode(lua_State *L)		/** encode(s) */
{
 size_t l;
 const unsigned char *s=(const unsigned char*)luaL_checklstring(L,1,&l);
 luaL_Buffer b;
 int n;
 luaL_buffinit(L,&b);
 for (n=l/3; n--; s+=3) encode(&b,s[0],s[1],s[2],3);
 switch (l%3)
 {
  case 1: encode(&b,s[0],0,0,1);		break;
  case 2: encode(&b,s[0],s[1],0,2);		break;
 }
 luaL_pushresult(&b);
 return 1;
}

static void decode(luaL_Buffer *b, int c1, int c2, int c3, int c4, int n)
{
 unsigned long tuple=c4+64L*(c3+64L*(c2+64L*c1));
 char s[3];
 switch (--n)
 {
  case 3: s[2]=tuple;
  case 2: s[1]=tuple >> 8;
  case 1: s[0]=tuple >> 16;
 }
 luaL_addlstring(b,s,n);
}

static int Ldecode(lua_State *L)		/** decode(s) */
{
 size_t l;
 const char *s=luaL_checklstring(L,1,&l);
 luaL_Buffer b;
 int n=0;
 char t[4];
 luaL_buffinit(L,&b);
 for (;;)
 {
  int c=*s++;
  switch (c)
  {
   const char *p;
   default:
    p=strchr(code,c); if (p==NULL) return 0;
    t[n++]= p-code;
    if (n==4)
    {
     decode(&b,t[0],t[1],t[2],t[3],4);
     n=0;
    }
    break;
   case '=':
    switch (n)
    {
     case 1: decode(&b,t[0],0,0,0,1);		break;
     case 2: decode(&b,t[0],t[1],0,0,2);	break;
     case 3: decode(&b,t[0],t[1],t[2],0,3);	break;
    }
   case 0:
    luaL_pushresult(&b);
    return 1;
   case '\n': case '\r': case '\t': case ' ': case '\f': case '\b':
    break;
  }
 }
 return 0;
}

static const luaL_Reg R[] =
{
	{ "encode",	Lencode	},
	{ "decode",	Ldecode	},
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_base64(lua_State *L)
{
 luaL_newlib(L,R);
 lua_pushliteral(L,"version");			/** version */
 lua_pushliteral(L,MYVERSION);
 lua_settable(L,-3);
 return 1;
}

#if defined(_KERNEL) && defined(__linux__)
#include <linux/module.h>

static int __init modinit(void)
{
 return 0;
}

static void __exit modexit(void)
{
}

module_init(modinit);
module_exit(modexit);
EXPORT_SYMBOL(luaopen_base64);
MODULE_LICENSE("Dual MIT/GPL");
#endif
