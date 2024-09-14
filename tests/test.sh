cc client.c -o client
for i in {1..200}; do
    curl http://localhost:8080 &
done
rm client
wait
