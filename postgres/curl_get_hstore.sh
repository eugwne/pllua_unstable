pgremotepath=https://raw.githubusercontent.com/postgres/postgres/REL9_4_STABLE

mkdir -p ./contrib/hstore
curl -s $pgremotepath/contrib/hstore/hstore.h > ./contrib/hstore/hstore.h
