// Path-Finding solution by Breese Sherman
// August 7, 2020

#include <iostream>
#include <vector>

int FindPath(const int nStartX, const int nStartY,
             const int nTargetX, const int nTargetY,
             const unsigned char* pMap, const int nMapWidth, const int nMapHeight,
             int* pOutBuffer, const int nOutBufferSize)
{

  // Trivial case: the start and target cell are the same.
  if (nStartX == nTargetX && nStartY == nTargetY) {
    return 0;
  }

  // Using pMap, nMapWidth, and nMapHeight, generate a graph representing the map.
  // Our graph is an array of integers called "adj" that is four times the size of
  // the number of cells in the map. Every four cells in "adj" represents the
  // neighboring cells of the n'th cell, i.e., the n'th group of four. If there's
  // a -1 in a group of four, that means there is no neighbor there... i.e., it's
  // an untraversable cell, or there's no cell at all.

  int nMapSize = nMapWidth * nMapHeight;
  int* adj = new int[nMapSize * 4];
  for (int i = 0; i < nMapSize * 4; i++) {
    adj[i] = -1;
  }

  // We build "adj" in four steps: by looking in each direction for each cell.
  // To account for edge cases, we only check to the right if the cell in question
  // isn't on the right border, we only check below if the cell isn't on the
  // bottom... so on and so forth.

  for (int i = 0; i < nMapSize; i++) {
    if (pMap[i] == 1) {
      // Rightwards
      if ((i + 1) % nMapWidth != 0) {
        if (pMap[i + 1] == 1)
          adj[4*i] = i + 1;
      }
      // Upwards
      if (i >= nMapWidth) {
        if (pMap[i - nMapWidth] == 1)
          adj[4*i + 1] = i - nMapWidth;
      }
      // Leftwards
      if (i % nMapWidth != 0) {
        if (pMap[i - 1] == 1)
          adj[4*i + 2] = i - 1;
      }
      // Downwards
      if (i < nMapSize - nMapWidth) {
        if (pMap[i + nMapWidth] == 1)
          adj[4*i + 3] = i + nMapWidth;
      }
    }
  }

  // Our graph is done. To find the shortest path between two cells, we use a
  // two-way breadth-first search. In the following code, we "fan out" from both
  // the starting cell and the target cell until the two "fans" meet. Imagine
  // dropping two stones in a pond. The point where the ripples meet is on the
  // shortest path between them. Even if we add walls in the pond, it still works.

  int* start_level = new int[nMapSize];     // Distance of each cell from start
  int* start_parent = new int[nMapSize];    // Parent of each cell (leading to start)
  int* target_level = new int[nMapSize];    // Distance of each cell from target
  int* target_parent = new int[nMapSize];   // Parent of each cell (leading to target)

  for (int i = 0; i < nMapSize; i++) {
    start_level[i] = -1;
    start_parent[i] = -1;
    target_level[i] = -1;
    target_parent[i] = -1;
  }

  // The starting cell is 0 steps away from the starting cell, so we set
  // start_level(starting cell position) to 0. Likewise with the target cell.

  start_level[nStartX + nStartY * nMapWidth] = 0;
  target_level[nTargetX + nTargetY * nMapWidth] = 0;

  int l = 1;                     // How many steps away we are from both the start
                                 // and the target. We'll increment this as we fan
                                 // out.

  // The "frontiers" are dynamic arrays that represent the "edges" of our "fans."
  // The "next" arrays represent the unvisited cells that lie one step further.

  std::vector<int> start_frontier;
  std::vector<int> target_frontier;
  std::vector<int> start_next;
  std::vector<int> target_next;

  // Our first frontiers are the start and target cells themselves.

  start_frontier.push_back(nStartX + nStartY * nMapWidth);
  target_frontier.push_back(nTargetX + nTargetY * nMapWidth);

  // As long as our frontiers exist, we still have more cells to visit. (Unless
  // our frontiers connect, but we'll get to that later.)

  while (start_frontier.size() > 0 && target_frontier.size() > 0) {

    // How to expand a frontier: First, clear out the "next" array.
    start_next.clear();

    // Then, for all cells in the frontier, look at their unvisited neighbors.
    for (int u = 0; u < start_frontier.size(); u++) {
      for (int v = 0; v < 4; v++) {
        int n = adj[4*start_frontier[u] + v];
        if (n != -1) {
          // Remember that if a cell hasn't been visited, its "level" is -1. So,
          // if this is the case, add it to our "next" array, and set its level
          // and parent.
          if (start_level[n] == -1) {
            start_level[n] = l;
            start_parent[n] = start_frontier[u];
            start_next.push_back(n);
          }
        }
      }
    }

    // Then we set our frontier to "next" so it reaches one step further.
    start_frontier.clear();
    for (int u = 0; u < start_next.size(); u++) {
      start_frontier.push_back(start_next[u]);
    }

    // That was just the frontier fanning out from the starting cell... now, we
    // have to do it again for the frontier coming from our target cell. It's
    // pretty much the same.

    target_next.clear();

    for (int u = 0; u < target_frontier.size(); u++) {

      // Except for this: our breakout case. If the cell in the target frontier
      // that we're looking at has already been visited by the start frontier, our
      // frontiers have just met at a cell on the shortest path.

      if (start_level[target_frontier[u]] != -1) {

        // Determine the "ancestral" paths we took to get to this "meeting" cell.
        std::vector<int> start_path;
        std::vector<int> target_path;
        int p = target_frontier[u];
        while (target_parent[p] != -1) {
          target_path.push_back(p);
          p = target_parent[p];
        }
        target_path.push_back(nTargetX + nTargetY * nMapWidth);
        p = start_parent[target_frontier[u]];
        while (start_parent[p] != -1) {
          start_path.push_back(p);
          p = start_parent[p];
        }

        // Get the lengths of those paths.
        int sp = start_path.size();   // Starting cell to meeting cell
        int tp = target_path.size();  // Meeting cell to target cell

        // Don't bother with pOutBuffer if the path length is longer than
        // nOutBufferSize.
        if (sp + tp <= nOutBufferSize) {
          for (int i = 0; i < sp; i++) {
            // Have to fill this in backwards since it's an ancestral path.
            pOutBuffer[i] = start_path[sp - 1 - i];
          }
          for (int i = 0; i < tp; i++) {
            // An ancestral path going backwards is a double-negative of sorts,
            // so we can fill in the rest of it forwards.
            pOutBuffer[i + sp] = target_path[i];
          }
        }

        // Clean up...
        delete[] start_level;
        delete[] start_parent;
        delete[] target_level;
        delete[] target_parent;
        delete[] adj;

        // Return the total length of the path, i.e., the length of the path from
        // the starting cell to the meeting cell plus the length of the path from
        // the meeting cell to the ending cell. All done!
        return sp + tp;
      }

      for (int v = 0; v < 4; v++) {
        int n = adj[4*target_frontier[u] + v];
        if (n != -1) {
          if (target_level[n] == -1) {
            target_level[n] = l;
            target_parent[n] = target_frontier[u];
            target_next.push_back(n);
          }
        }
      }
    }
    target_frontier.clear();
    for (int u = 0; u < target_next.size(); u++) {
      target_frontier.push_back(target_next[u]);
    }
    l++;
  }

  // No path found.
  // Clean up...
  delete[] start_level;
  delete[] start_parent;
  delete[] target_level;
  delete[] target_parent;
  delete[] adj;

  return -1;
}
