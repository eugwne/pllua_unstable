pllua unstable
=====

[PL/Lua] (https://github.com/pllua/pllua) fork for implementing new PL/Lua features, not properly tested. 

Requirements: Postgres 9.4, lua 5.1. 

### changes: 

####hstore type support  

example:

```LUA
do $$
  register_type('hstore')
  local hs = fromstring('hstore','"aa"=>"bb"') 
  hs = hstore.delete(fromstring('hstore','a=>1 , b=>2, c=>3'), 'd')
  print(hs)
  hs.a = "foo"
  hs.f = "bar"
  print(hs)
$$ LANGUAGE pllua;
```
####pgfunc function

**pgfunc** might be used to bind postgres functions(sql/internal functions with IN arguments)

example:

```LUA
do $$
local quote_ident = pgfunc("quote_ident(text)")
print(quote_ident("int"))
$$ LANGUAGE pllua;
```

wrap sql function:
```SQL
CREATE OR REPLACE FUNCTION add_em(integer, integer)
  RETURNS integer AS
$BODY$
    SELECT $1 + $2;
$BODY$  LANGUAGE sql;
```

```LUA
do $$
local add_em = pgfunc("add_em(integer, integer)")
print  (add_em(5,10));
$$ LANGUAGE pllua;
```

pgfunc might be used as chunk loader for pllua functions with signature **function(internal) internal** :

at first we need to register module as postgres function:
```LUA
CREATE OR REPLACE FUNCTION i_void(internal)
  RETURNS internal AS
$BODY$
-- mymodule.lua
local M = {} -- public interface

-- private
local x = 1
local function baz() print 'test' end

function M.foo() print("foo", x) end

function M.bar()
  M.foo()
  baz()
  print "bar"
end

return M

$BODY$
  LANGUAGE pllua
```

load and use this code:
```LUA
do $$
local m_void = pgfunc("i_void(internal)")
m_void.bar()
$$ LANGUAGE pllua;
```
####subtransactions

example:
```LUA
do $$
local f = function() 
  local p = server.prepare("UPDATE accounts SET balance = balance + $2 WHERE account_name = $1", {"text","int4"})
  p:execute{'joe', 50}
  p:execute{'mary',-50}
  return true
end  
local status, err = subtransaction(f)
print(status, err)
$$ LANGUAGE pllua;
```

note: all plans used in subtransaction must be created inside subtransaction function, otherwise it will crash

### License

Copyright (c) 2008 Luis Carvalho

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
