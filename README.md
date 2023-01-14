# GPP-Exam-research-topic

I created a goal based vector field / flow field pathfinding algorithm.

![Infinite_Examplegif](https://user-images.githubusercontent.com/95921047/212049584-7a818c75-f573-4051-ad22-eb996eeab01d.gif)


## World Layout
The world is broken up into individual sectors containing grid squares. Each sector contains 10 x 10 grid squares.
There are also portal windows, where each portal window crosses a sector boundary. Portal windows begin and end at walls on either side of sector boundaries. There is one portal for each window side.


Sectors: ![image](https://user-images.githubusercontent.com/95921047/211923820-b74d7c02-2c2c-43e0-a924-60603f6ad4e6.png)
Portals: ![image](https://user-images.githubusercontent.com/95921047/211923879-3c4c400c-16e0-41d7-8897-bcbaee9fec44.png)

## Data field types
For each sector there are 3 different 10 x 10 arrays/fields of data. These three types are
1. Type field: Contains data over what the cell is. ex: wall / inaccessible / walkable
2. Heat field: A field containing the distance from the goal for each cell.
3. Flow field: A field containing 'path to goal' directions.

## Creating a path
If a valid goal position is made and there are agents in the world, pathes will be created. The algorithm will first run A* pathfinding on the sectors and portals and
find the most efficient path on a sector level. This process happens for all agents unless there is already a path to the sector of the agent or a previous path passed
the sector the agent.  
When this stage is completed you have the sectors that will be used by the agents to go to the goal so you dont have to calculate a flow field
for the whole world. And since the sectors are large, the A* pathes are 'short' and little CPU is used compaired A* on a larger scale.

Example of a generated path on a sector level: ![image](https://user-images.githubusercontent.com/95921047/211927171-ec0a552b-abc1-40ed-8263-8e2fbcb9c3f2.png)
  
    
## Creating a Heat field
After the path generation, a heat field is generated.  
How to generate a distance based heat field:  
We first create an empty list called the open list, and to this open list we will append the goal tile. This tile will be called the current node, and all the nodes arround it are the neighbor nodes. Now for each neighbor node we execute the next sequence:    
First we will look if the selected node is a wall or not, if not we will look if the node has been visited before. If it has not, we give it the distance value of the current node + 1 and add it to the open list.  
After doing this to each neighbor node, the current node is removed from the open list.  
And this whole process is repeated untill the open list is empty. (by hitting walls or ends of sectors)  
When done, each node will have been visited and assigned a distance from the goal node.  

Example of heat field generation: ![image](https://user-images.githubusercontent.com/95921047/211928745-2d88d51a-c864-4b92-ae6b-221b53cd5968.png)  
(In this image the shade of red is rescaled to the sector it is in, making it look weird.)

## Creating a flow field
The last part is the creation of the flow field based on the heat field.  
To create a vector we use a simplified version of a process called 'Kernel convolution' (A convolution Kernel will take information about it's neighbors and use it to modify its own information):  
For each tile of the grid we look at the heat field values of the neighbor tiles and create a vector pointing to the lowest value.

The vector points to the lowest distance value: ![image](https://user-images.githubusercontent.com/95921047/211929190-626b33ee-15a0-4f88-9ea5-a4cc8bed5226.png)

Example of flow field generation: ![image](https://user-images.githubusercontent.com/95921047/211928845-68f27aab-da72-4042-85ec-237e513cb06a.png)


## Conclusion
Now everything is done and all that is left is to set the agent's velocity to that of the vector of the cell that it is in and we have created flow field that can handle loads of agents in large worlds.  
  
sources:  
https://www.youtube.com/watch?v=ZJZu3zLMYAc  
http://www.gameaipro.com/GameAIPro/GameAIPro_Chapter23_Crowd_Pathfinding_and_Steering_Using_Flow_Field_Tiles.pdf
