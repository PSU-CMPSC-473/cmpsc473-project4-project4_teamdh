Deep , dph5402
Arpit , abs6339




@Test1: 1, 1000, 6080000
@Test2: 2, 1000, 8310000
@Test3: 4, 1000, 12750000
@Test4: 8, 1000, 20220000
@Test5: 16, 1000, 29570000
@Test6: 32, 1000, 51780000
@Test7: 8, 100, 20160000
@Test8: 8, 1000, 20260000
@Test9: 8, 10000, 20400000


Explanation on the impact of performance for the relationship between the number of mappers vs the buffer size:

Test 1-6: As seen from the real times mentioned in microseconds for each test case, we can observe that as the number of mappers are increased we see an overall increase in real time. This can explained by the fact that during context-switching between threads, the CPU first needs to save the program pointer and local registers of the current thread and then load the program pointer and local registers of the next thread which consumes time and is affecting the performance of multi-threaded program as observed. 

Test 7-9: Because of the style of our multi-threaded program, one or more mappers against only one reducers, the program spends more time in recieving and on reducer side while the mappers are waiting to send further information into our buffer which would explain why changing of buffer size for same number of threads doesn't cause much difference in performance when comparing in terms of seconds.
