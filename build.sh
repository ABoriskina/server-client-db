cd postgres && ./gen.sh # WARNING! you nedd to chmod+x before build

cd ..

mkdir -p  practice1_client/build
cd practice1_client/build && cmake .. && cmake --build . # build client

cd ../..

mkdir -p  practice1_server/build
cd practice1_server/build && cmake .. && cmake --build . # build server

cd .. && ./gen.sh # generate keys for server