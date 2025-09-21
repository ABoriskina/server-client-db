mkdir -p build/certs
openssl req -x509 -newkey rsa:2048 -nodes \
    -keyout build/certs/server.key -out build/certs/server.crt -days 365 \
    -subj "/CN=localhost"

mkdir -p ../practice1_client/build/certs
cp build/certs/server.crt ../practice1_client/build/certs/

