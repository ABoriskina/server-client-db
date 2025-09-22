mkdir -p certs
openssl req -new -text -passout pass:abcd -subj /CN=localhost -out certs/server.req -keyout certs/server.key
openssl rsa -in certs/server.key -passin pass:abcd -out certs/server.key
openssl req -x509 -in certs/server.req -text -key certs/server.key -out certs/server.crt

sudo chown 999:999 certs/server.key
chmod 600 certs/server.key
chmod 644 certs/server.crt

cp certs/server.crt ../practice1_server/build/certs/server_postgres.crt