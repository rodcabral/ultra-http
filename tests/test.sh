cc client.c -o client
for i in {1..100}; do
    ./client 8080 &
done
rm client
wait
