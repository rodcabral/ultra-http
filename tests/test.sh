for i in {1..500}; do
    curl http://localhost:8080 &
done
wait
