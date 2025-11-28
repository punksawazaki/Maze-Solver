# Maze Solver

Maze Solver is a simple web-based visualization tool that lets you create mazes, generate random mazes, and run different pathfinding algorithms to see how they explore and solve the maze.

The project is built with C++ (using Crow for the backend) and HTML/CSS/JavaScript for the frontend.

You can:
- Create your own maze by clicking on the grid  
- Set the Start (S) and Goal (G) positions  
- Generate a random solvable maze  
- Run BFS, DFS, Greedy Best-First Search, and A*  
- Watch the algorithm step-by-step as it visits cells  
- View the final path, visited nodes, and basic statistics  

The random maze generator uses a DFS backtracking algorithm, which always creates a connected and solvable maze.

To run the project:
1. Build the C++ server (Crow + CMake)
2. Run the executable
3. Open `http://localhost:18080` in your browser

The project was created as a final assignment and demonstrates basic maze generation, pathfinding, and web-based visualization using C++.
