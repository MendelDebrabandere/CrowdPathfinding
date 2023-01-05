# GPP-Exam-research-topic

I will try to create a vector field / flow field pathfinding algorithm.

![image](https://user-images.githubusercontent.com/95921047/209066811-861d3889-4ede-4782-af1f-c0c44a4b9c25.png)

This can be usefeull when the target never / rarely moves, and a lot of agents need to go there.
The calculation is larger than a normal Astar pathfinding algorithm, but once computed, the vector field is created and all the agents can find a path using the field without computing anything.




How i will approach this problem:
First create a distance heatmap

![image](https://user-images.githubusercontent.com/95921047/209067461-0bd8f538-add2-496d-9658-9fbbacaad785.png)

Then create a vector for every cell using the heatmap.

![image](https://user-images.githubusercontent.com/95921047/209067738-6549cffe-1434-4c63-8516-00d1eef93b5e.png)





