#!/bin/env/python
"""
The maze looks like:

 _________|30 |
|  *  8  -  1 |
|  4  * 11  * |
|  +  4  - 18 |
|     -  9  * |
| 22 |--------|
"""

p = lambda x,y: x+y
m = lambda x,y: x-y
t = lambda x,y: x*y

maze = [
    [22, m, 9, t],
    [p, 4, m, 18],
    [4, t, 11, t],
    [t, 8, m, 1],
]

class player:
    def __init__(self):
        self.row: int = 0
        self.col: int = 0
        self.op = None
        self.value: int= maze[0][0]
        self.lastMove: tuple|None = None

    def continuations(self):
        cont = []
        if self.row > 0:
            cont.append(self.__clone_and_move((-1, 0)))
        if self.row < 3:
            cont.append(self.__clone_and_move((1, 0)))
        if self.col > 0:
            cont.append(self.__clone_and_move((0,-1)))
        if self.col < 3:
            cont.append(self.__clone_and_move((0,1)))
        return list(filter(lambda x: x is not None, cont))

    def __clone_and_move(self, move: tuple):
        p = player()
        p.lastMove = move
        p.row = self.row + move[0]
        p.col = self.col + move[1]
        if p.row == 0 and p.col == 0:
            return None
        w = maze[p.row][p.col]
        if self.op == None:
            p.op = w
            p.value = self.value
        else:
            p.op = None
            p.value = self.op(self.value, w)
        return p

def main():
    p = player()
    result = dfs(p, 12)
    if result is None:
        print("No result")
        return

    print(f"Found a result in {len(result)} moves:")
    for state in result[::-1]:
        match state.lastMove:
            case (0,1):
                print(f"east")
            case (0,-1):
                print(f"west")
            case (1,0):
                print(f"north")
            case (-1,0):
                print(f"south")
            case _:
                raise RuntimeError(f"Invalid move {state.lastMove}")

def manhattan(p: player, destination: tuple = (3,3)) -> int:
    return abs(destination[0]-p.row)+abs(destination[1]-p.col)

def dfs(p: player, depth: int) -> list|None:
    if p is None:
        return None

    if manhattan(p) > depth:
        # Won't have time to get there
        return None

    if p.row==3 and p.col==3:
        if p.value==30:
            return []
        return None

    conts = p.continuations()
    # Heuristic: move towards destination
    conts.sort(key=manhattan, reverse=True)
    for c in conts:
        path = dfs(c, depth-1)
        if path is None:
            continue
        else:
            path.append(c)
            return path

if __name__ == '__main__':
    main()
