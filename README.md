ASSUMPTIONS:

g++ Pricer.cpp -o main
./main <target-size> < <log-file>

For example:

g++ Pricer.cpp -o main
./main 200 < sample.in



Things I would like to improve:

1. Improve efficiency
2. Make code comments more helpful
3. Remove duplicated code
4. Add error checking


Questions:

1. How did you choose your implementation language?

	I am most comfortable working with C++ when I do OOP. I knew I had to implement a class structure and use some complex data structures, so I chose a language where I'm most familiar with types of data structures and their complexities. 

2. What is the time complexity for processing an Add Order message?

	If n is the number of strings stored in the bidMap or askMap, then:

		findMaxOffer: O(n) 
		findMinOffer: O(n)

	which makes:

		addOrder: O(n) 

3. What is the time complexity for processing a Reduce Order message?

	reduceOrder: O(logn) best case, O(n) worst case scenario

4. If your implementation were put into production and found to be too slow, what ideas would you try out to improve its performance? (Other than reimplementing it in a different language such as C or C++.)

	I would try working with different data structures or using another algorithm. Perhaps implementing a hash map myself (instead of using the one from the standard library) would allow me to get a best case O(1) lookup time. The most costly functions I have implemented are findMaxOffer and findMinOffer of O(n), since they go through each item in the map already stored looking for a max or min value. Maybe I would have to create a new data structure to store the prices of each stock order in an ordered data structure, to reduce lookup time. 