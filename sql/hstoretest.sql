CREATE EXTENSION  IF NOT EXISTS  hstore;
CREATE OR REPLACE FUNCTION pg_temp.hstore_test()
RETURNS SETOF text AS $$
  register_type('hstore')
  local hs = fromstring('hstore','"aa"=>"bb"') 
  coroutine.yield(tostring(hs))
  hs = fromstring('hstore','aa=>null')
  coroutine.yield(tostring(hs))
  hs = fromstring('hstore',[[\=a=>q=w]])
  coroutine.yield(tostring(hs))
  hs = fromstring('hstore','aa=>b, c=>d , b=>16')
  coroutine.yield(hs.c)
  coroutine.yield(hs.aa)
  --coroutine.yield(hs.gg); yield(nil) == break.....
  coroutine.yield(tostring(hs.gg))
  hs = fromstring('hstore','aa=>"NULL", c=>d , b=>16')
  coroutine.yield(tostring(hs.aa))
  -- hstore.delete --
  hs = hstore.delete(fromstring('hstore','a=>null , b=>2, c=>3'), 'a')
  coroutine.yield(tostring(hs))
  hs = hstore.delete(fromstring('hstore','a=>1 , b=>2, c=>3'), 'b')
  coroutine.yield(tostring(hs))
  hs = hstore.delete(fromstring('hstore','a=>1 , b=>2, c=>3'), 'd')
  coroutine.yield(tostring(hs))
  hs.a = "foo"
  hs.f = "bar"
  coroutine.yield(tostring(hs))
$$ LANGUAGE pllua;
select pg_temp.hstore_test();

